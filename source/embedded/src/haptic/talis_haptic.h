/**
 * @file talis_haptic.h
 * @brief 震动马达（GPIO_19 直驱）
 */

#ifndef __TALIS_HAPTIC_H__
#define __TALIS_HAPTIC_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化震动马达
 *
 * @warning 必须在 talis_board_register_hardware() 之后调用。
 *
 * GPIO_19 在 SDK 原版板级里会被 board_sdcard_prepare() 复用成 SDIO_D3。
 * 本项目的 talis_board.c 默认不启用 SD 卡（TALIS_BOARD_ENABLE_SDCARD=0），
 * 因此这根脚是干净的 GPIO，不需要 pinmux 抢回。
 * 若日后打开 SD 卡，这里会检测到冲突并打印告警。
 */
OPERATE_RET talis_haptic_init(void);

/**
 * @brief 触发一次“三段短震”（非阻塞，立即返回）
 *
 * 可在任意任务/回调里调用，内部用软件定时器推进，不会阻塞调用方。
 * 重复调用会重新开始计数，不会叠加。
 */
void talis_haptic_trigger(void);

/**
 * @brief 立即停止震动
 */
void talis_haptic_stop(void);

/**
 * @brief 是否正在震动
 */
bool talis_haptic_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_HAPTIC_H__ */
