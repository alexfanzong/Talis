/**
 * @file talis_ble.h
 * @brief BLE Peripheral：广播、连接管理、Talis 帧收发
 */

#ifndef __TALIS_BLE_H__
#define __TALIS_BLE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 BLE 协议栈（Peripheral 角色）并注册事件回调
 *
 * 回调内部只投递事件到 talis_event_queue，不做任何 lv_* 调用。
 */
OPERATE_RET talis_ble_init(void);

OPERATE_RET talis_ble_start_advertising(void);
OPERATE_RET talis_ble_stop_advertising(void);

/**
 * @brief 发送“请手机响铃”指令，并启动 ACK 超时定时器
 *
 * 超时或失败都会投递 EVENT_PHONE_RING_FAIL，
 * 成功收到应答投递 EVENT_PHONE_RING_ACK。
 * 调用方不需要自己处理超时。
 *
 * @return OPRT_OK 已发出；其它表示当前发不出去（未连接/未订阅）
 */
OPERATE_RET talis_ble_send_ring_phone(void);

/** @brief 是否已建立 BLE 连接 */
bool talis_ble_is_connected(void);

/**
 * @brief 是否真正可以收发数据（已连接 且 App 已订阅 notify）
 *
 * UI 上的 "Find my phone" 按钮应该按这个判断是否置灰，
 * 而不是 talis_ble_is_connected()：连上了但没订阅时，
 * tal_ble_server_common_send() 是发不出去的。
 */
bool talis_ble_is_ready(void);

/**
 * @brief 取 BLE 调试摘要，用于在屏幕上显示
 *
 * 这块板子的日志走 UART1，而 USB 串口接的是 UART0（下载口），
 * 拿不到串口日志，所以把关键计数显示到屏幕上代替。
 * 格式：A:广播次数 C:连接次数 S:订阅次数 D:断连次数 W:收包次数
 *
 * @param buf  输出缓冲
 * @param len  缓冲大小
 */
void talis_ble_debug_str(char *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_BLE_H__ */
