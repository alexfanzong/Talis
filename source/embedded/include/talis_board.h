/**
 * @file talis_board.h
 * @brief Talis 项目本地板级定义（SPARKLEIOT_T5AI_DEV）
 *
 * 移植自 TuyaOpenSDK/boards/T5AI/SPARKLEIOT_T5AI_DEV/board_com_api.c
 * 移植日期：2026-08-15
 *
 * ============================================================================
 * 为什么把板级代码搬进项目
 * ============================================================================
 * SDK 的 boards/ 目录是所有人共用的，队友同步 SDK 会覆盖掉本地修改。
 * Talis 需要两处偏离 SDK 默认板级配置（竖屏、释放震动引脚），
 * 因此把板级注册代码收进项目自管，SDK 侧保持零修改。
 *
 * SDK 侧仍然保留 CONFIG_BOARD_CHOICE_SPARKLEIOT_T5AI_DEV=y，
 * 因为板级 Kconfig 负责 select ENABLE_DISPLAY / ENABLE_TP / ENABLE_BUTTON
 * 等开关，并提供 DISPLAY_NAME / BUTTON_NAME / AUDIO_CODEC_NAME 宏。
 * 我们只是不调用 SDK 的 board_register_hardware()，改调本文件的版本。
 *
 * ============================================================================
 * 与 SDK 原版的差异（共 2 处，均可用下方开关一键还原）
 * ============================================================================
 * 1. TALIS_BOARD_LCD_ROTATION：ROTATION_90 → ROTATION_0
 *    原因：Talis UI 为 240x320 竖屏。
 *    附带收益：rotation != 0 时 LVGL 每帧要做一次软件旋转
 *    （src/liblvgl/v8/port/lv_port_disp.c + tdl_display_draw_rotate.c，纯 CPU
 *    且额外占一块 rotate buffer），改成 0 之后这部分开销直接消失。
 *
 * 2. TALIS_BOARD_ENABLE_SDCARD：1 → 0（不调用 SD 卡 pinmux）
 *    原因：SDK 原版 board_register_hardware() 无条件调用 board_sdcard_prepare()，
 *    把 GPIO14~GPIO19 复用成 SDIO。其中 GPIO_19 是 Talis 的震动马达引脚，
 *    被 SDIO 抢走后 GPIO 输出失效，表现为「代码全对但马达不动」。
 *    Talis 不使用 SD 卡，直接不调用即可，无需任何 pinmux 抢回技巧。
 *
 * 另外 TALIS_BOARD_ENABLE_CAMERA 默认关闭：Talis 不用摄像头，
 * 不注册可以省下 GPIO_0/GPIO_1 和一部分内存。
 */

#ifndef __TALIS_BOARD_H__
#define __TALIS_BOARD_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
********************* 功能开关 *****************************
***********************************************************/

/** SD 卡：Talis 不使用。置 1 会占用 GPIO14~GPIO19（含震动脚 GPIO_19） */
#ifndef TALIS_BOARD_ENABLE_SDCARD
#define TALIS_BOARD_ENABLE_SDCARD    0
#endif

/** 摄像头：Talis 不使用。置 1 会占用 GPIO_0 / GPIO_1 */
#ifndef TALIS_BOARD_ENABLE_CAMERA
#define TALIS_BOARD_ENABLE_CAMERA    0
#endif

/***********************************************************
********************* 引脚定义 *****************************
***********************************************************/

/* ---- 音频 ---- */
#define TALIS_BOARD_SPEAKER_EN_PIN      TUYA_GPIO_NUM_7

/* ---- 实体按键（BACK）---- */
#define TALIS_BOARD_BUTTON_PIN          TUYA_GPIO_NUM_8
#define TALIS_BOARD_BUTTON_ACTIVE_LV    TUYA_GPIO_LEVEL_LOW

/* ---- LCD ST7789 (SPI0) ---- */
#define TALIS_BOARD_LCD_BL_TYPE         TUYA_DISP_BL_TP_GPIO
#define TALIS_BOARD_LCD_BL_PIN          TUYA_GPIO_NUM_9
#define TALIS_BOARD_LCD_BL_ACTIVE_LV    TUYA_GPIO_LEVEL_HIGH

#define TALIS_BOARD_LCD_WIDTH           240
#define TALIS_BOARD_LCD_HEIGHT          320
#define TALIS_BOARD_LCD_PIXELS_FMT      TUYA_PIXEL_FMT_RGB565

/* 【差异 1】竖屏。改回 TUYA_DISPLAY_ROTATION_90 即恢复 SDK 原始行为 */
#define TALIS_BOARD_LCD_ROTATION        TUYA_DISPLAY_ROTATION_0

#define TALIS_BOARD_LCD_SPI_PORT        TUYA_SPI_NUM_0
#define TALIS_BOARD_LCD_SPI_CLK         48000000
#define TALIS_BOARD_LCD_SPI_CS_PIN      TUYA_GPIO_NUM_45
#define TALIS_BOARD_LCD_SPI_DC_PIN      TUYA_GPIO_NUM_47
#define TALIS_BOARD_LCD_SPI_RST_PIN     TUYA_GPIO_NUM_6
#define TALIS_BOARD_LCD_SPI_MOSI_PIN    TUYA_GPIO_NUM_46
#define TALIS_BOARD_LCD_SPI_CLK_PIN     TUYA_GPIO_NUM_44

#define TALIS_BOARD_LCD_POWER_PIN       TUYA_GPIO_NUM_MAX
#define TALIS_BOARD_LCD_POWER_ACTIVE_LV TUYA_GPIO_LEVEL_HIGH

/* ---- 触摸 CST816X (I2C1) ---- */
#define TALIS_BOARD_TP_I2C_PORT         TUYA_I2C_NUM_1
#define TALIS_BOARD_TP_I2C_SCL_PIN      TUYA_GPIO_NUM_20
#define TALIS_BOARD_TP_I2C_SDA_PIN      TUYA_GPIO_NUM_21
#define TALIS_BOARD_TP_RST_PIN          TUYA_GPIO_NUM_23
/* 中断脚未接线，触摸走轮询 */
#define TALIS_BOARD_TP_INTR_PIN         TUYA_GPIO_NUM_MAX

/* ---- 震动马达（Talis 新增，SDK 板级没有）---- */
/* 注意：与 SDIO_D3 是同一根引脚。TALIS_BOARD_ENABLE_SDCARD 必须为 0 */
#define TALIS_BOARD_HAPTIC_PIN          TUYA_GPIO_NUM_19

/* ---- 摄像头（默认不启用）---- */
#define TALIS_BOARD_CAMERA_I2C_PORT     TUYA_I2C_NUM_0
#define TALIS_BOARD_CAMERA_I2C_SCL      TUYA_GPIO_NUM_0
#define TALIS_BOARD_CAMERA_I2C_SDA      TUYA_GPIO_NUM_1
#define TALIS_BOARD_CAMERA_RST_PIN      TUYA_GPIO_NUM_MAX
#define TALIS_BOARD_CAMERA_RST_ACTIVE_LV TUYA_GPIO_LEVEL_LOW
#define TALIS_BOARD_CAMERA_POWER_PIN    TUYA_GPIO_NUM_MAX
#define TALIS_BOARD_CAMERA_PWR_ACTIVE_LV TUYA_GPIO_LEVEL_LOW
#define TALIS_BOARD_CAMERA_CLK          24000000

/* ---- SDIO（默认不启用，引脚留档备查）---- */
#define TALIS_BOARD_SDIO_CLK_PIN        TUYA_GPIO_NUM_14
#define TALIS_BOARD_SDIO_CMD_PIN        TUYA_GPIO_NUM_15
#define TALIS_BOARD_SDIO_D0_PIN         TUYA_GPIO_NUM_16
#define TALIS_BOARD_SDIO_D1_PIN         TUYA_GPIO_NUM_17
#define TALIS_BOARD_SDIO_D2_PIN         TUYA_GPIO_NUM_18
#define TALIS_BOARD_SDIO_D3_PIN         TUYA_GPIO_NUM_19  /* == HAPTIC_PIN */

/***********************************************************
********************* 对外接口 *****************************
***********************************************************/

/**
 * @brief 注册板载全部外设（音频、按键、显示、触摸）
 *
 * 替代 SDK 的 board_register_hardware()。差异见文件头注释。
 * 必须在使用任何外设之前调用一次。
 *
 * @return OPRT_OK 成功；其它值为错误码
 */
OPERATE_RET talis_board_register_hardware(void);

/**
 * @brief 配置 SD 卡 SDIO pinmux（MODE1, GPIO14~GPIO19）
 *
 * @warning 会占用 GPIO_19（震动马达引脚）。Talis 默认不调用。
 *          仅在 TALIS_BOARD_ENABLE_SDCARD=1 时编译。
 */
#if TALIS_BOARD_ENABLE_SDCARD
OPERATE_RET talis_board_sdcard_prepare(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_BOARD_H__ */
