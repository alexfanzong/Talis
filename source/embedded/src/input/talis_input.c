/**
 * @file talis_input.c
 * @brief 实体按键输入实现
 *
 * 线程约定：按键回调运行在 button 任务里，只投事件，不碰 lv_*。
 */

#include "tal_api.h"
#include "tdl_button_manage.h"

#include "talis_app_config.h"
#include "talis_event_queue.h"
#include "talis_input.h"

/* 长按判定阈值：黑客松现场手指容易误判，1.2 秒比较跟手 */
#define TALIS_BTN_LONG_PRESS_MS    1200
#define TALIS_BTN_DEBOUNCE_MS      20

static TDL_BUTTON_HANDLE sg_btn_back = NULL;

static void __button_cb(char *name, TDL_BUTTON_TOUCH_EVENT_E event, void *argc)
{
    (void)name;
    (void)argc;

    switch (event) {
    case TDL_BUTTON_PRESS_SINGLE_CLICK:
        PR_DEBUG("BACK short press");
        talis_event_post(EVENT_BACK);
        break;

    case TDL_BUTTON_LONG_PRESS_START:
        PR_DEBUG("BACK long press -> home");
        talis_event_post(EVENT_BACK_LONG);
        break;

    default:
        break;
    }
}

OPERATE_RET talis_input_init(void)
{
    OPERATE_RET rt = OPRT_OK;

#if defined(BUTTON_NAME)
    TDL_BUTTON_CFG_T button_cfg = {
        .long_start_valid_time     = TALIS_BTN_LONG_PRESS_MS,
        .long_keep_timer           = 1000,
        .button_debounce_time      = TALIS_BTN_DEBOUNCE_MS,
        .button_repeat_valid_count = 0,   /* 不用双击 */
        .button_repeat_valid_time  = 0,
    };

    TUYA_CALL_ERR_RETURN(tdl_button_create(BUTTON_NAME, &button_cfg, &sg_btn_back));

    /* 短按返回上一页，长按直接回首页。
     * 这是触摸失效时的保底通道，务必两个都注册。 */
    tdl_button_event_register(sg_btn_back, TDL_BUTTON_PRESS_SINGLE_CLICK, __button_cb);
    tdl_button_event_register(sg_btn_back, TDL_BUTTON_LONG_PRESS_START, __button_cb);

    PR_NOTICE("input ready: BACK button \"%s\" (long press %dms -> home)",
              BUTTON_NAME, TALIS_BTN_LONG_PRESS_MS);
#else
    PR_WARN("BUTTON_NAME not defined, physical button disabled");
#endif

    return rt;
}
