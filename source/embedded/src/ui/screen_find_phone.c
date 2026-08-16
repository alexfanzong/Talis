/**
 * @file screen_find_phone.c
 * @brief S3 正在呼叫手机
 *
 * 这个页面永远不会停在这里：talis_ble 内部有 5 秒 ACK 超时，
 * 超时/失败/断连都会投 EVENT_PHONE_RING_FAIL，状态机据此跳到结果页。
 */

#include "tal_api.h"
#include "talis_screens.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_strings.h"

static lv_obj_t *sg_scr = NULL;

static void find_init(void)
{
    sg_scr = talis_ui_screen_create();

    talis_ui_header(sg_scr, TALIS_TXT_RING_TITLE, false, false);

    talis_ui_text(sg_scr, TALIS_TXT_RING_BODY, 70, LV_TEXT_ALIGN_CENTER, TALIS_FONT_BODY, TALIS_COLOR_TEXT_DIM);

    /* 转圈指示，让用户知道设备在等回应而不是死了 */
    lv_obj_t *spinner = lv_spinner_create(sg_scr, 1000, 60);
    lv_obj_set_size(spinner, 72, 72);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_arc_color(spinner, TALIS_COLOR_CARD, LV_PART_MAIN);
    lv_obj_set_style_arc_color(spinner, TALIS_COLOR_PRIMARY, LV_PART_INDICATOR);

    talis_ui_button(sg_scr, TALIS_TXT_CANCEL, -8, TALIS_COLOR_CARD, EVENT_BACK);
}

static void find_deinit(void)
{
    sg_scr = NULL;
}

Screen_t screen_find_phone = {
    .init       = find_init,
    .deinit     = find_deinit,
    .screen_obj = &sg_scr,
    .name       = "find_phone",
    .state_data = NULL,
};
