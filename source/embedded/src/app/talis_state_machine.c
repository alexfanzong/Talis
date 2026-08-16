/**
 * @file talis_state_machine.c
 * @brief Talis 状态机实现
 *
 * 导航模型：在节点树里进出，路径存在 sg_path 栈里。
 *   BOOT -> 根列表(模块) -> 子列表 -> ... -> 详情
 * 页面栈由 screen_manager 维护，两者深度一一对应。
 */

#include "tal_api.h"
#include "lvgl.h"

#include "talis_app_config.h"
#include "talis_strings.h"
#include "talis_types.h"
#include "talis_event_queue.h"
#include "talis_state_machine.h"
#include "talis_screens.h"
#include "screen_manager.h"
#include "talis_ble.h"
#include "talis_haptic.h"
#include "talis_audio.h"
#include "talis_storage.h"

#define SM_POLL_INTERVAL_MS   20
#define SM_MAX_DEPTH          6

static TalisState_t sg_state      = STATE_BOOT;
static lv_timer_t  *sg_poll_timer = NULL;
static TIMER_ID     sg_disc_timer = NULL;
static TIMER_ID     sg_auto_back  = NULL;

/* 当前在树里的位置。sg_depth==0 表示在根（模块列表） */
static const talis_node_t *sg_path[SM_MAX_DEPTH];
static uint8_t             sg_depth = 0;

/***********************************************************
********************* 名字表 *******************************
***********************************************************/
const char *talis_state_name(TalisState_t st)
{
    static const char *names[STATE_MAX] = {
        "BOOT", "BROWSE", "DISCONNECTED", "FIND_PHONE", "FIND_RESULT",
    };
    return (st < STATE_MAX) ? names[st] : "?";
}

const char *talis_event_name(TalisEvent_t ev)
{
    static const char *names[EVENT_MAX] = {
        "NONE", "BOOT_DONE", "BLE_CONN", "BLE_DISC", "BLE_READY",
        "TIMEOUT_3S", "DISMISS", "FIND_PHONE", "RING_ACK", "RING_FAIL",
        "BACK", "BACK_LONG", "FINISH", "PLAY_AUDIO", "OPEN_CHILD",
    };
    return (ev < EVENT_MAX) ? names[ev] : "?";
}

/***********************************************************
********************* 定时器回调 ***************************
***********************************************************/
/* 跑在 timer 任务，只能投事件，不能碰 lv_* */
static void __disc_timer_cb(TIMER_ID t, void *a)
{
    (void)t; (void)a;
    talis_event_post(EVENT_TIMEOUT_3S);
}

static void __auto_back_cb(TIMER_ID t, void *a)
{
    (void)t; (void)a;
    talis_event_post(EVENT_FINISH);
}

/***********************************************************
********************* 树导航 *******************************
***********************************************************/
static const talis_node_t *__cur_node(void)
{
    return (sg_depth > 0) ? sg_path[sg_depth - 1] : NULL;
}

/* 取当前层的子节点数组 */
static void __cur_children(const talis_node_t **items, uint8_t *count)
{
    const talis_device_pack_t *pack = talis_storage_get_pack();
    const talis_node_t *node = __cur_node();

    if (NULL == node) {
        *items = pack->modules;
        *count = pack->module_count;
    } else {
        *items = node->children;
        *count = node->child_count;
    }
}

/* 回到根：清空路径并把页面栈弹到底 */
static void __goto_root(void)
{
    sg_depth = 0;
    sg_state = STATE_BROWSE;
    screen_list_set(NULL);
    screen_back_bottom();
    PR_NOTICE("nav -> root");
}

/* 后退一层 */
static void __go_back(void)
{
    if (sg_depth == 0) {
        return;   /* 已经在根，不再退 */
    }

    sg_depth--;
    sg_state = STATE_BROWSE;

    /* 重新设置目标页面要展开的节点，再让页面栈弹一层 */
    screen_list_set(__cur_node());
    screen_back();

    PR_NOTICE("nav <- depth %d", sg_depth);
}

static void __enter_find_phone(void);

/* 进入第 idx 个子节点 */
static void __open_child(uint32_t idx)
{
    const talis_node_t *items = NULL;
    uint8_t count = 0;

    __cur_children(&items, &count);

    if (NULL == items || idx >= count) {
        PR_ERR("open child %u out of range (%d)", (unsigned)idx, count);
        return;
    }

    const talis_node_t *child = &items[idx];

    /* 动作节点：不入栈，直接执行 */
    if (child->action == ACTION_FIND_PHONE) {
        __enter_find_phone();
        return;
    }

    if (sg_depth >= SM_MAX_DEPTH) {
        PR_ERR("nav too deep");
        return;
    }

    sg_path[sg_depth++] = child;
    sg_state = STATE_BROWSE;

    if (child->child_count > 0) {
        screen_list_set(child);
        screen_load(&screen_list);
        PR_NOTICE("nav -> [%s] list, depth %d", child->title, sg_depth);
    } else {
        screen_detail_set(child);
        screen_load(&screen_detail);
        PR_NOTICE("nav -> [%s] detail, depth %d", child->title, sg_depth);
    }
}

/***********************************************************
********************* 找手机 *******************************
***********************************************************/
static void __enter_find_phone(void)
{
    /* 本机先震一下，给用户即时反馈：设备确实收到了这次操作。
     * 就算 BLE 发不出去，这一下震动也说明设备是活的。 */
    talis_haptic_trigger();

    if (!talis_ble_is_ready() || OPRT_OK != talis_ble_send_ring_phone()) {
        PR_WARN("cannot ring phone: BLE not ready");
        screen_find_result_set(false);
        sg_state = STATE_FIND_RESULT;
        screen_load(&screen_find_result);
        tal_sw_timer_start(sg_auto_back, TALIS_RESULT_AUTO_BACK_MS, TAL_TIMER_ONCE);
        return;
    }

    sg_state = STATE_FIND_PHONE;
    screen_load(&screen_find_phone);
}

static void __show_ring_result(bool success)
{
    tal_sw_timer_stop(sg_auto_back);
    screen_find_result_set(success);
    sg_state = STATE_FIND_RESULT;
    screen_load(&screen_find_result);
    tal_sw_timer_start(sg_auto_back, TALIS_RESULT_AUTO_BACK_MS, TAL_TIMER_ONCE);
}

/* 找手机的结果页 / 呼叫页退出时，回到进入前那一层 */
static void __leave_find_phone(void)
{
    tal_sw_timer_stop(sg_auto_back);
    sg_state = STATE_BROWSE;
    screen_list_set(__cur_node());
    screen_back();
}

/***********************************************************
********************* 事件处理 *****************************
***********************************************************/
static void __handle_event(TalisEvent_t ev, uint32_t param)
{
    PR_DEBUG("[SM] %s + %s", talis_state_name(sg_state), talis_event_name(ev));

    /* ---- 全局 ---- */
    switch (ev) {
    case EVENT_BACK_LONG:
        tal_sw_timer_stop(sg_disc_timer);
        tal_sw_timer_stop(sg_auto_back);
        __goto_root();
        return;

    case EVENT_BLE_DISCONNECTED:
        screen_list_refresh();
        /* 用户正在看离线内容时不要打断 —— 手机丢了正是要看那些页的时候 */
        if (sg_depth > 0) {
            return;
        }
        tal_sw_timer_start(sg_disc_timer, TALIS_DISCONNECT_ALERT_MS, TAL_TIMER_ONCE);
        return;

    case EVENT_TIMEOUT_3S:
        /* 必须全局处理：事件到达时通常还在根页，
         * 放进分状态分支会被漏掉，提醒页永远出不来 */
        if (sg_depth > 0 || sg_state == STATE_DISCONNECTED) {
            return;
        }
        talis_haptic_trigger();
        sg_state = STATE_DISCONNECTED;
        screen_load(&screen_disconnected);
        return;

    case EVENT_BLE_CONNECTED:
    case EVENT_BLE_READY:
        tal_sw_timer_stop(sg_disc_timer);
        if (sg_state == STATE_DISCONNECTED) {
            __goto_root();
        } else {
            screen_list_refresh();
        }
        return;

    default:
        break;
    }

    /* ---- 分状态 ---- */
    switch (sg_state) {
    case STATE_BOOT:
        if (ev == EVENT_BOOT_DONE) {
            __goto_root();
        }
        break;

    case STATE_BROWSE:
        if (ev == EVENT_OPEN_CHILD) {
            __open_child(param);
        } else if (ev == EVENT_BACK) {
            __go_back();
        } else if (ev == EVENT_PLAY_AUDIO) {
            if (talis_audio_is_playing()) {
                talis_audio_stop();
            } else {
                talis_audio_play_help();
            }
            screen_detail_refresh_audio();
        }
        break;

    case STATE_DISCONNECTED:
        if (ev == EVENT_DISMISS || ev == EVENT_BACK) {
            __goto_root();
        } else if (ev == EVENT_FIND_PHONE) {
            __enter_find_phone();
        }
        break;

    case STATE_FIND_PHONE:
        if (ev == EVENT_PHONE_RING_ACK) {
            __show_ring_result(true);
        } else if (ev == EVENT_PHONE_RING_FAIL) {
            __show_ring_result(false);
        } else if (ev == EVENT_BACK) {
            __leave_find_phone();
        }
        break;

    case STATE_FIND_RESULT:
        if (ev == EVENT_FINISH || ev == EVENT_BACK) {
            __leave_find_phone();
        }
        break;

    default:
        break;
    }
}

/***********************************************************
********************* 事件轮询 *****************************
***********************************************************/
/* 跑在 LVGL 任务里（lv_task_handler 内部，锁已持有），
 * 所以可以直接调 lv_*，且绝不能再调 lv_vendor_disp_lock() */
static void __poll_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    TalisEventMsg_t msg;

    for (int i = 0; i < 8; i++) {
        /* timeout 必须为 0：在 LVGL 任务里阻塞会冻住整个界面 */
        if (OPRT_OK != talis_event_fetch(&msg, 0)) {
            break;
        }
        __handle_event(msg.evt, msg.param);
    }
}

/***********************************************************
********************* 对外接口 *****************************
***********************************************************/
OPERATE_RET talis_state_machine_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__disc_timer_cb, NULL, &sg_disc_timer));
    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__auto_back_cb, NULL, &sg_auto_back));

    sg_depth = 0;

    /* 根列表是栈底，screen_back_bottom() 才有正确的归宿 */
    screen_list_set(NULL);
    screen_manager_init(&screen_list);

    /* 开机页盖在上面，收到 EVENT_BOOT_DONE 后退回根列表 */
    screen_load(&screen_boot);
    sg_state = STATE_BOOT;

    sg_poll_timer = lv_timer_create(__poll_timer_cb, SM_POLL_INTERVAL_MS, NULL);
    if (NULL == sg_poll_timer) {
        PR_ERR("create state machine poll timer failed");
        return OPRT_COM_ERROR;
    }

    PR_NOTICE("state machine ready");

    return OPRT_OK;
}

TalisState_t talis_state_get(void)
{
    return sg_state;
}
