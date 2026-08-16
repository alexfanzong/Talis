/**
 * @file talis_input.h
 * @brief 输入统一入口：实体按键 + 触摸
 *
 * 触摸不在这里处理 —— CST816X 由板级注册后接进 LVGL indev，
 * 页面内的按钮点击走 LVGL 自己的事件回调（见 talis_ui.c）。
 * 注意板级 BOARD_TP_INTR_PIN 未接线，触摸是轮询的，不存在触摸中断。
 *
 * 本模块只负责实体 BACK 键，并把它翻译成事件队列里的事件，
 * 保证触摸失效时用户仍能退出任何页面。
 */

#ifndef __TALIS_INPUT_H__
#define __TALIS_INPUT_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化输入
 *
 * @warning 必须在 talis_board_register_hardware() 之后调用，
 *          因为按键硬件是在那里注册的。
 */
OPERATE_RET talis_input_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_INPUT_H__ */
