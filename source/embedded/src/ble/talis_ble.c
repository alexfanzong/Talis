/**
 * @file talis_ble.c
 * @brief BLE Peripheral 实现
 *
 * 线程约定：本文件所有回调运行在蓝牙协议栈任务里，
 * 只允许调用 talis_event_post() / talis_haptic_*，禁止调用任何 lv_* 。
 */

#include <string.h>

#include "tal_api.h"
#include "tal_bluetooth.h"

#include "talis_app_config.h"
#include "talis_event_queue.h"
#include "talis_ble.h"
#include "talis_ble_protocol.h"

/***********************************************************
********************* 广播数据 *****************************
***********************************************************/
/* 广播包结构（沿用 SDK ble_peripher 示例，保证被 Tuya 协议栈接受）：
 *   02 01 06                    Flags: LE General Discoverable + BR/EDR not supported
 *   03 02 FD FD                 不完整 16-bit 服务 UUID 列表
 *   17 16 50 FD ...             服务数据，对应服务 0xFD50
 *
 * 注意：0xFDFD 是广播里声明的 UUID，与 GATT 服务 UUID 0xFD50 不是一回事。
 * Android 侧连上以后要找的是 0xFD50。
 */
static uint8_t sg_adv_data[31] = {
    0x02, 0x01, 0x06,
    0x03, 0x02, 0xFD, 0xFD,
    0x17, 0x16, 0x50, 0xFD, 0x41, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* 扫描响应：末尾放设备名 "Talis"，方便队友用 nRF Connect 认出来 */
static uint8_t sg_scan_rsp_data[31] = {
    0x17,                                     /* len */
    0xFF, 0xD0, 0x07,                         /* 厂商自定义数据，Tuya ID */
    0x00,                                     /* Encry Mode */
    0x00, 0x00,                               /* communication way */
    0x00,                                     /* FLAG */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x06,                                     /* 下一个 AD 结构长度: 1(type) + 5(name) */
    0x09, 'T', 'a', 'l', 'i', 's',            /* Complete Local Name = "Talis" */
};

/***********************************************************
********************* 内部状态 *****************************
***********************************************************/
/* 协议栈上报 STACK_INIT 后，再等这么久才开广播。
 * SDK 示例用的是 1000ms 的 sleep，这里用定时器达到同样效果但不阻塞。 */
#define BLE_ADV_START_DELAY_MS   1000

static TAL_BLE_PEER_INFO_T sg_peer;
static bool     sg_connected      = false;
static bool     sg_notify_enabled = false;
static uint8_t  sg_seq            = 0;
static uint8_t  sg_ring_seq       = 0;      /* 正在等待应答的 RING_PHONE 序号 */
static bool     sg_ring_pending   = false;
static TIMER_ID sg_ack_timer      = NULL;
static TIMER_ID sg_adv_timer      = NULL;   /* 延迟开广播 */

/* 调试计数器：这块板子的日志走 UART1，USB 串口接的是 UART0，
 * 拿不到串口日志，只能把事件计数显示到屏幕上 */
static uint16_t sg_cnt_adv    = 0;
static uint16_t sg_cnt_conn   = 0;
static uint16_t sg_cnt_sub    = 0;
static uint16_t sg_cnt_disc   = 0;
static uint16_t sg_cnt_write  = 0;
static int16_t  sg_last_err   = 0;

/***********************************************************
********************* 内部函数 *****************************
***********************************************************/
static OPERATE_RET __ble_send_frame(uint8_t cmd, uint8_t seq, const uint8_t *payload, uint8_t len)
{
    uint8_t buf[TALIS_FRAME_MAX_LEN] = {0};

    if (!sg_connected) {
        PR_WARN("send %s failed: not connected", talis_cmd_name(cmd));
        return OPRT_COM_ERROR;
    }

    if (!sg_notify_enabled) {
        PR_WARN("send %s failed: peer has not subscribed notify", talis_cmd_name(cmd));
        return OPRT_COM_ERROR;
    }

    int frame_len = talis_frame_pack(buf, sizeof(buf), cmd, seq, payload, len);
    if (frame_len <= 0) {
        return OPRT_COM_ERROR;
    }

    TAL_BLE_DATA_T pkt = {
        .len    = (uint16_t)frame_len,
        .p_data = buf,
    };

    OPERATE_RET rt = tal_ble_server_common_send(&pkt);
    PR_DEBUG("BLE tx %s seq=%d len=%d rt=%d", talis_cmd_name(cmd), seq, frame_len, rt);

    return rt;
}

/* 延迟开广播，见 TAL_BLE_STACK_INIT 处的说明 */
static void __adv_start_cb(TIMER_ID timer_id, void *arg)
{
    (void)timer_id;
    (void)arg;
    talis_ble_start_advertising();
}

/* ACK 超时：不能让 UI 卡在“发送中”，超时一律按失败处理 */
static void __ack_timeout_cb(TIMER_ID timer_id, void *arg)
{
    (void)timer_id;
    (void)arg;

    if (!sg_ring_pending) {
        return;
    }

    sg_ring_pending = false;
    PR_WARN("RING_PHONE ack timeout (seq=%d)", sg_ring_seq);
    talis_event_post(EVENT_PHONE_RING_FAIL);
}

static void __handle_rx_frame(const uint8_t *data, uint16_t len)
{
    TalisFrame_t frame;

    if (OPRT_OK != talis_frame_parse(data, len, &frame)) {
        return;
    }

    PR_DEBUG("BLE rx %s seq=%d len=%d", talis_cmd_name(frame.cmd), frame.seq, frame.len);

    switch (frame.cmd) {
    case CMD_RING_ACK: {
        if (!sg_ring_pending) {
            PR_DEBUG("unsolicited RING_ACK, ignore");
            break;
        }

        /* 序号对不上说明是上一次请求的迟到应答，丢弃 */
        if (frame.seq != sg_ring_seq) {
            PR_WARN("RING_ACK seq mismatch: got %d, expect %d", frame.seq, sg_ring_seq);
            break;
        }

        sg_ring_pending = false;
        tal_sw_timer_stop(sg_ack_timer);

        bool ok = (frame.len >= 1 && frame.payload[0] == TALIS_RING_ACK_OK);
        PR_NOTICE("phone ring %s", ok ? "OK" : "FAILED");
        talis_event_post(ok ? EVENT_PHONE_RING_ACK : EVENT_PHONE_RING_FAIL);
        break;
    }

    case CMD_HELLO_ACK:
        PR_NOTICE("handshake done with phone");
        break;

    default:
        PR_DEBUG("unhandled cmd 0x%02X", frame.cmd);
        break;
    }
}

static void __ble_event_cb(TAL_BLE_EVT_PARAMS_T *p_event)
{
    if (NULL == p_event) {
        return;
    }

    switch (p_event->type) {
    case TAL_BLE_STACK_INIT:
        /* init == 0 表示协议栈起来了 */
        if (p_event->ble_event.init == 0) {
            PR_NOTICE("BLE stack ready, will start advertising in %dms", BLE_ADV_START_DELAY_MS);
            /* 不在回调里 tal_system_sleep —— 那会阻塞蓝牙协议栈任务。
             * 改用一次性定时器延迟启动广播。
             * 延迟是必要的：SDK 的 ble_peripher 示例在这里也 sleep 了 1 秒，
             * 说明协议栈上报 STACK_INIT 之后还需要一点时间把 GATT 服务表建好。
             * 抢跑开广播会导致手机能扫到、但连上去没有服务，连接随即断开。 */
            tal_sw_timer_start(sg_adv_timer, BLE_ADV_START_DELAY_MS, TAL_TIMER_ONCE);
        } else {
            PR_ERR("BLE stack init failed: %d", p_event->ble_event.init);
        }
        break;

    case TAL_BLE_EVT_PERIPHERAL_CONNECT: {
        /* result != 0 是连接失败的回调，不能当成连上了 */
        if (p_event->ble_event.connect.result != 0) {
            PR_WARN("BLE connect failed: %d", p_event->ble_event.connect.result);
            break;
        }

        sg_cnt_conn++;
        sg_connected      = true;
        sg_notify_enabled = false;   /* 新连接必须重新订阅 */
        memcpy(&sg_peer, &p_event->ble_event.connect.peer, sizeof(TAL_BLE_PEER_INFO_T));
        PR_NOTICE("BLE connected");
        talis_event_post(EVENT_BLE_CONNECTED);
        break;
    }

    case TAL_BLE_EVT_DISCONNECT: {
        sg_cnt_disc++;
        sg_connected      = false;
        sg_notify_enabled = false;
        memset(&sg_peer, 0, sizeof(TAL_BLE_PEER_INFO_T));

        /* 断连时如果还在等响铃应答，立刻判失败，别让 UI 干等 */
        if (sg_ring_pending) {
            sg_ring_pending = false;
            tal_sw_timer_stop(sg_ack_timer);
            talis_event_post(EVENT_PHONE_RING_FAIL);
        }

        PR_NOTICE("BLE disconnected, restart advertising");
        /* 不重启广播的话手机就再也连不上了 */
        tal_ble_advertising_start(TUYAOS_BLE_DEFAULT_ADV_PARAM);

        talis_event_post(EVENT_BLE_DISCONNECTED);
        break;
    }

    case TAL_BLE_EVT_SUBSCRIBE: {
        bool notify_now = (p_event->ble_event.subscribe.cur_notify != 0);
        PR_NOTICE("BLE subscribe char=0x%04X notify=%d",
                  p_event->ble_event.subscribe.char_handle, notify_now);

        if (notify_now) { sg_cnt_sub++; }
        sg_notify_enabled = notify_now;

        if (notify_now) {
            /* 订阅成功才算真正可通信，此时握手 */
            __ble_send_frame(CMD_HELLO, sg_seq++, NULL, 0);
            talis_event_post(EVENT_BLE_READY);
        }
        break;
    }

    case TAL_BLE_EVT_WRITE_REQ:
        sg_cnt_write++;
        __handle_rx_frame(p_event->ble_event.write_report.report.p_data,
                          p_event->ble_event.write_report.report.len);
        break;

    case TAL_BLE_EVT_MTU_REQUEST:
        PR_DEBUG("BLE MTU request: %d", p_event->ble_event.exchange_mtu.mtu);
        break;

    case TAL_BLE_EVT_CONN_PARAM_UPDATE:
        PR_DEBUG("BLE conn param updated");
        break;

    default:
        break;
    }
}

/***********************************************************
********************* 对外接口 *****************************
***********************************************************/
OPERATE_RET talis_ble_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    memset(&sg_peer, 0, sizeof(TAL_BLE_PEER_INFO_T));
    sg_connected      = false;
    sg_notify_enabled = false;
    sg_ring_pending   = false;

    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__ack_timeout_cb, NULL, &sg_ack_timer));
    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__adv_start_cb, NULL, &sg_adv_timer));

    /* 注意 SDK 里的枚举拼写就是 PERIPERAL（少一个 H），不是笔误 */
    TUYA_CALL_ERR_RETURN(tal_ble_bt_init(TAL_BLE_ROLE_PERIPERAL, __ble_event_cb));

    /* 这里不要直接开广播 —— tal_ble_bt_init 是异步的，协议栈还没就绪。
     * 广播由 TAL_BLE_STACK_INIT 事件延迟触发。 */
    PR_NOTICE("BLE peripheral init done (service 0xFD50), waiting for stack ready");

    return OPRT_OK;
}

OPERATE_RET talis_ble_start_advertising(void)
{
    OPERATE_RET rt = OPRT_OK;

    TAL_BLE_DATA_T adv = {
        .p_data = sg_adv_data,
        .len    = sizeof(sg_adv_data),
    };
    TAL_BLE_DATA_T rsp = {
        .p_data = sg_scan_rsp_data,
        .len    = sizeof(sg_scan_rsp_data),
    };

    TUYA_CALL_ERR_LOG(tal_ble_advertising_data_set(&adv, &rsp));
    rt = tal_ble_advertising_start(TUYAOS_BLE_DEFAULT_ADV_PARAM);
    if (rt != OPRT_OK) {
        sg_last_err = (int16_t)rt;
        PR_ERR("advertising_start failed: %d", rt);
        return rt;
    }

    sg_cnt_adv++;
    PR_NOTICE("BLE advertising as \"Talis\"");

    return OPRT_OK;
}

OPERATE_RET talis_ble_stop_advertising(void)
{
    return tal_ble_advertising_stop();
}

OPERATE_RET talis_ble_send_ring_phone(void)
{
    if (!talis_ble_is_ready()) {
        PR_WARN("cannot ring phone: BLE not ready");
        return OPRT_COM_ERROR;
    }

    sg_ring_seq = sg_seq++;

    OPERATE_RET rt = __ble_send_frame(CMD_RING_PHONE, sg_ring_seq, NULL, 0);
    if (rt != OPRT_OK) {
        return rt;
    }

    sg_ring_pending = true;
    tal_sw_timer_start(sg_ack_timer, TALIS_RING_ACK_TIMEOUT_MS, TAL_TIMER_ONCE);

    PR_NOTICE("RING_PHONE sent (seq=%d), waiting ack up to %dms",
              sg_ring_seq, TALIS_RING_ACK_TIMEOUT_MS);

    return OPRT_OK;
}

bool talis_ble_is_connected(void)
{
    return sg_connected;
}

bool talis_ble_is_ready(void)
{
    return (sg_connected && sg_notify_enabled);
}

void talis_ble_debug_str(char *buf, uint32_t len)
{
    if (NULL == buf || 0 == len) {
        return;
    }

    snprintf(buf, len, "A%d C%d S%d D%d W%d E%d",
             sg_cnt_adv, sg_cnt_conn, sg_cnt_sub, sg_cnt_disc, sg_cnt_write, sg_last_err);
}
