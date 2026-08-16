/**
 * @file screen_find_result.c
 * @brief S4 呼叫结果页
 */

#include "tal_api.h"
#include "talis_screens.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_strings.h"

static lv_obj_t *sg_scr     = NULL;
static bool      sg_success = false;

void screen_find_result_set(bool success)
{
    sg_success = success;
}

static void result_init(void)
{
    sg_scr = talis_ui_screen_create();



    lv_obj_t *icon = lv_label_create(sg_scr);
    lv_label_set_text(icon, sg_success ? LV_SYMBOL_OK : LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(icon, sg_success ? TALIS_COLOR_OK : TALIS_COLOR_DANGER,
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, TALIS_FONT_BIG, LV_PART_MAIN);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *msg = lv_label_create(sg_scr);
    lv_label_set_text(msg, sg_success ? TALIS_TXT_RING_OK : TALIS_TXT_RING_FAIL);
    lv_obj_set_style_text_color(msg, TALIS_COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(msg, TALIS_FONT_TITLE, LV_PART_MAIN);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_label_set_long_mode(msg, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg, TALIS_SCREEN_W - 2 * TALIS_PAD);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 14);

    talis_ui_button(sg_scr, TALIS_TXT_BACK, -8, TALIS_COLOR_PRIMARY, EVENT_FINISH);
}

static void result_deinit(void)
{
    sg_scr = NULL;
}

Screen_t screen_find_result = {
    .init       = result_init,
    .deinit     = result_deinit,
    .screen_obj = &sg_scr,
    .name       = "find_result",
    .state_data = NULL,
};
