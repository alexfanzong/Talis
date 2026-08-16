/**
 * @file talis_app_config.h
 * @brief Talis 应用级配置：屏幕尺寸、配色、超时、功能开关
 */

#ifndef __TALIS_APP_CONFIG_H__
#define __TALIS_APP_CONFIG_H__

/***********************************************************
********************* 屏幕 *********************************
***********************************************************/
/* 竖屏。所有页面尺寸都用这两个宏，不要在页面里写死数字，
 * 万一要改横屏只需改这里 + talis_board.h 的 ROTATION */
#define TALIS_SCREEN_W              240
#define TALIS_SCREEN_H              320

/* 通用留白 */
#define TALIS_PAD                   16

/***********************************************************
********************* 配色（深色底，高对比）****************
***********************************************************/
#define TALIS_COLOR_BG              lv_color_hex(0x101418)
#define TALIS_COLOR_CARD            lv_color_hex(0x1C232B)
#define TALIS_COLOR_TEXT            lv_color_hex(0xF2F5F7)
#define TALIS_COLOR_TEXT_DIM        lv_color_hex(0x93A1AD)
#define TALIS_COLOR_PRIMARY         lv_color_hex(0x2E7DF6)
#define TALIS_COLOR_DANGER          lv_color_hex(0xE2453C)
#define TALIS_COLOR_OK              lv_color_hex(0x2FB86B)
#define TALIS_COLOR_WARN            lv_color_hex(0xE8A33D)

/***********************************************************
********************* 超时与时序 ***************************
***********************************************************/
/* 断连多久后开始提醒（毫秒） */
#define TALIS_DISCONNECT_ALERT_MS   3000

/* Find my phone 等待手机 ACK 的超时（毫秒） */
#define TALIS_RING_ACK_TIMEOUT_MS   5000

/* 结果页停留多久自动回首页（毫秒） */
#define TALIS_RESULT_AUTO_BACK_MS   3000

/* 页面切换后忽略触摸的时间，防连击（毫秒） */
#define TALIS_TOUCH_GUARD_MS        300

/* 震动“三段短震”时序（毫秒） */
#define TALIS_HAPTIC_ON_MS          200
#define TALIS_HAPTIC_OFF_MS         150
#define TALIS_HAPTIC_PULSES         3

/***********************************************************
********************* LVGL 任务 ****************************
***********************************************************/
#define TALIS_LVGL_TASK_PRIO        5
#define TALIS_LVGL_TASK_STACK       (1024 * 8)

/***********************************************************
********************* 功能开关 *****************************
***********************************************************/
/* 音频：需要 talis_audio_data.c 里有真实 MP3 数据才有声音。
 * 置 0 可在没有音频素材时跑通全流程 */
#ifndef TALIS_ENABLE_AUDIO
#define TALIS_ENABLE_AUDIO          1
#endif

/* 行动包持久化到 Flash KV。置 0 则只用固件内置的常量行动包 */
#ifndef TALIS_ENABLE_KV_STORAGE
#define TALIS_ENABLE_KV_STORAGE     1
#endif

/* ---- 调试用：最小 UI 模式 ----
 * 置 1 时跳过页面管理器和状态机，只在屏幕上画一个 label，
 * 其余（板级、震动、存储、音频、BLE、LVGL v8、ROTATION_0）全部保持不变。
 *
 * 用途：这块板子上 SDK 的 lvgl_label 例子（LVGL v9 + ROTATION_90）能正常显示，
 * 但完整 Talis 不显示。用这个开关把「UI 层」和「其余所有层」分开：
 *   - 最小 UI 能显示 → v8 / ROTATION_0 / 板级 / 初始化顺序都没问题，
 *                      故障在 screen_manager 或状态机
 *   - 最小 UI 也不显示 → 故障在更底层（v8 移植、rotation、或初始化顺序）
 * 定位完成后必须改回 0。 */
#ifndef TALIS_MINIMAL_UI
#define TALIS_MINIMAL_UI            0
#endif

#endif /* __TALIS_APP_CONFIG_H__ */
