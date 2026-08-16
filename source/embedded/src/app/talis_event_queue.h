/**
 * @file talis_event_queue.h
 * @brief 跨任务事件队列
 *
 * ============================================================================
 * 为什么需要它
 * ============================================================================
 * LVGL 跑在自己的任务里（lv_vendor_start 创建）。以下回调都发生在别的任务：
 *   - BLE 事件回调        → 蓝牙协议栈任务
 *   - 按键事件回调        → button 任务
 *   - 软件定时器回调      → timer 任务
 * 在这些回调里直接调 lv_* 会与 LVGL 任务竞争，表现为跑一段时间后随机死机。
 *
 * 约定：非 LVGL 任务只调 talis_event_post()，状态机在 LVGL 任务侧消费，
 * 这样所有 lv_* 调用天然在同一个任务里，不需要加锁。
 */

#ifndef __TALIS_EVENT_QUEUE_H__
#define __TALIS_EVENT_QUEUE_H__

#include "talis_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化事件队列
 */
OPERATE_RET talis_event_queue_init(void);

/**
 * @brief 投递事件（任意任务/中断下文均可调用，非阻塞）
 */
OPERATE_RET talis_event_post(TalisEvent_t evt);

/**
 * @brief 投递带参数的事件
 */
OPERATE_RET talis_event_post_param(TalisEvent_t evt, uint32_t param);

/**
 * @brief 取事件
 * @param[out] msg   取出的事件
 * @param[in]  timeout_ms  0 表示不等待
 * @return OPRT_OK 取到；其它表示无事件
 */
OPERATE_RET talis_event_fetch(TalisEventMsg_t *msg, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_EVENT_QUEUE_H__ */
