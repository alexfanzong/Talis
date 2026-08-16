/**
 * @file talis_audio.h
 * @brief 预录求助语音播放（MP3，C 数组编译进固件）
 */

#ifndef __TALIS_AUDIO_H__
#define __TALIS_AUDIO_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化音频播放服务
 *
 * 扬声器使能脚 GPIO_7 由 talis_board.c 的 audio codec 注册处理，这里不管。
 */
OPERATE_RET talis_audio_init(void);

/**
 * @brief 播放一段内存里的 MP3
 *
 * @param data MP3 数据（16kHz mono）
 * @param len  数据长度
 */
OPERATE_RET talis_audio_play(const uint8_t *data, uint32_t len);

/**
 * @brief 播放内置的求助语音
 *
 * 数据来自 talis_audio_data.c。若那里还是占位空数组，
 * 本函数返回 OPRT_NOT_FOUND 并打印提示，不会崩。
 */
OPERATE_RET talis_audio_play_help(void);

OPERATE_RET talis_audio_stop(void);

bool talis_audio_is_playing(void);

/** @brief 内置求助语音是否有真实数据 */
bool talis_audio_has_help_clip(void);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_AUDIO_H__ */
