/**
 * @file talis_state_machine.h
 * @brief Talis 状态机
 *
 * 运行位置：注册成 lv_timer，跑在 LVGL 任务里。
 *
 * 这一点很关键 —— lv_vendor 的任务循环是这样的：
 *     while (running) {
 *         lv_vendor_disp_lock();
 *         lv_task_handler();      <-- lv_timer 回调在这里面执行
 *         lv_vendor_disp_unlock();
 *         sleep();
 *     }
 * 所以 lv_timer 回调天然处于持锁状态，状态机里可以自由调用 lv_*，
 * 而且**绝不能**再调 lv_vendor_disp_lock()，那会重复上锁导致死锁。
 */

#ifndef __TALIS_STATE_MACHINE_H__
#define __TALIS_STATE_MACHINE_H__

#include "talis_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化状态机：建首页 UI + 注册事件轮询定时器
 *
 * @warning 必须在 lv_vendor_init() 之后、lv_vendor_start() 之前调用。
 */
OPERATE_RET talis_state_machine_init(void);

/** @brief 当前状态 */
TalisState_t talis_state_get(void);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_STATE_MACHINE_H__ */
