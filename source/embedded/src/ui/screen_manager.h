/**
 * @file screen_manager.h
 * @brief 栈式页面管理器
 *
 * 移植自 TuyaOpenSDK/apps/tuya_t5_pocket/tuya_t5_pocket_ai/src/display/ui/screen_manager.c
 * 适配改动：
 *   - 分辨率 384x168 横屏 → 240x320 竖屏（走 talis_app_config.h 的宏）
 *   - 栈深度 MAX_DEPTH 6 → 8（Talis 页面更多）
 *   - 切换动画由左右滑入改为上下滑入
 *   - 去掉 terminusTTF 字体的 LV_FONT_DECLARE（Talis 用 Montserrat）
 *   - 去掉 `#define printf PR_DEBUG`（会污染整个编译单元）
 *   - 去掉 PC 模拟器相关的条件编译
 *
 * 线程约定：本文件所有函数必须在 LVGL 任务里调用（即状态机里），
 * 不要在 BLE/按键/定时器回调里直接调用。详见 talis_event_queue.h
 */

#ifndef __SCREEN_MANAGER_H__
#define __SCREEN_MANAGER_H__

#include "lvgl.h"
#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCREEN_STACK_MAX_DEPTH   8

/**
 * @brief 页面描述结构
 *
 * 与 pocket_ai 原版保持一致，方便日后回抄改动。
 */
typedef struct {
    void (*init)(void);     /**< 建立页面 UI */
    void (*deinit)(void);   /**< 释放页面持有的资源（定时器等） */
    lv_obj_t **screen_obj;  /**< 指向页面根对象的指针 */
    char *name;             /**< 页面名，日志用 */
    void *state_data;       /**< 页面私有状态 */
} Screen_t;

/** @brief 取当前页面（栈顶），空栈返回 NULL */
Screen_t *screen_get_now_screen(void);

/** @brief 压入并切换到新页面 */
void screen_load(Screen_t *newScreen);

/** @brief 返回上一页 */
void screen_back(void);

/** @brief 返回栈底页面（首页） */
void screen_back_bottom(void);

/**
 * @brief 初始化页面栈并加载首页
 * @param home 首页（栈底，永远不会被 pop 掉）
 */
void screen_manager_init(Screen_t *home);

/** @brief 当前栈深度 */
uint8_t screen_stack_depth(void);

#ifdef __cplusplus
}
#endif

#endif /* __SCREEN_MANAGER_H__ */
