/**
 * @file talis_audio.c
 * @brief 音频播放实现
 *
 * 参考 SDK examples/multimedia/audio_player/tts —— 内存 MP3 播放。
 * 不能参考 audio_player/music 示例，那个用 URL 播放，需要联网，
 * 而 Talis 明确不启用 WiFi。
 */

#include "tal_api.h"

#include "talis_app_config.h"
#include "talis_audio.h"

#if TALIS_ENABLE_AUDIO

#include "svc_ai_player.h"
#include "tdl_audio_manage.h"

/* 内置求助语音，定义在 talis_audio_data.c */
extern const uint8_t  talis_help_clip_mp3[];
extern const uint32_t talis_help_clip_mp3_len;

static TDL_AUDIO_HANDLE_T sg_audio_hdl = NULL;
static AI_PLAYER_HANDLE   sg_player    = NULL;
static bool               sg_inited    = false;
static bool               sg_playing   = false;

static void __audio_frame_cb(TDL_AUDIO_FRAME_FORMAT_E type, TDL_AUDIO_STATUS_E status,
                             uint8_t *data, uint32_t len)
{
    (void)type;
    (void)status;
    (void)data;
    (void)len;
    /* Talis 不做录音，麦克风数据直接丢弃 */
}

OPERATE_RET talis_audio_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    if (sg_inited) {
        return OPRT_OK;
    }

    AI_PLAYER_CFG_T cfg = {
        .sample   = 16000,
        .datebits = 16,
        .channel  = 1,
    };
    TUYA_CALL_ERR_RETURN(tuya_ai_player_service_init(&cfg));
    TUYA_CALL_ERR_RETURN(tuya_ai_player_create(AI_PLAYER_MODE_FOREGROUND, &sg_player));

    /* 打开 codec，否则喂进去的数据出不了喇叭 */
    TUYA_CALL_ERR_RETURN(tdl_audio_find(AUDIO_CODEC_NAME, &sg_audio_hdl));
    TUYA_CALL_ERR_RETURN(tdl_audio_open(sg_audio_hdl, __audio_frame_cb));

    sg_inited = true;

    if (talis_audio_has_help_clip()) {
        PR_NOTICE("audio ready, help clip %u bytes", (unsigned)talis_help_clip_mp3_len);
    } else {
        PR_WARN("audio ready, but help clip is EMPTY "
                "(put a 16kHz mono MP3 into talis_audio_data.c)");
    }

    return OPRT_OK;
}

OPERATE_RET talis_audio_play(const uint8_t *data, uint32_t len)
{
    OPERATE_RET rt = OPRT_OK;

    if (!sg_inited) {
        PR_WARN("audio not init");
        return OPRT_RESOURCE_NOT_READY;
    }

    if (NULL == data || 0 == len) {
        return OPRT_INVALID_PARM;
    }

    /* 先停掉上一段，避免叠着放 */
    if (sg_playing) {
        tuya_ai_player_stop(sg_player);
    }

    TUYA_CALL_ERR_RETURN(tuya_ai_player_start(sg_player, AI_PLAYER_SRC_MEM, NULL, AI_AUDIO_CODEC_MP3));
    TUYA_CALL_ERR_RETURN(tuya_ai_player_feed(sg_player, (uint8_t *)data, len));
    /* 喂 NULL 表示数据结束 */
    TUYA_CALL_ERR_RETURN(tuya_ai_player_feed(sg_player, NULL, 0));

    sg_playing = true;
    PR_NOTICE("audio playing, %u bytes", (unsigned)len);

    return OPRT_OK;
}

OPERATE_RET talis_audio_play_help(void)
{
    if (!talis_audio_has_help_clip()) {
        PR_WARN("no help clip embedded, skip playback");
        return OPRT_NOT_FOUND;
    }

    return talis_audio_play(talis_help_clip_mp3, talis_help_clip_mp3_len);
}

OPERATE_RET talis_audio_stop(void)
{
    if (!sg_inited) {
        return OPRT_RESOURCE_NOT_READY;
    }

    sg_playing = false;

    return tuya_ai_player_stop(sg_player);
}

bool talis_audio_is_playing(void)
{
    return sg_playing;
}

bool talis_audio_has_help_clip(void)
{
    return (talis_help_clip_mp3_len > 0);
}

#else /* TALIS_ENABLE_AUDIO == 0 */

OPERATE_RET talis_audio_init(void)
{
    PR_NOTICE("audio disabled at compile time (TALIS_ENABLE_AUDIO=0)");
    return OPRT_OK;
}

OPERATE_RET talis_audio_play(const uint8_t *data, uint32_t len)
{
    (void)data;
    (void)len;
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET talis_audio_play_help(void)
{
    return OPRT_NOT_SUPPORTED;
}

OPERATE_RET talis_audio_stop(void)
{
    return OPRT_OK;
}

bool talis_audio_is_playing(void)
{
    return false;
}

bool talis_audio_has_help_clip(void)
{
    return false;
}

#endif /* TALIS_ENABLE_AUDIO */
