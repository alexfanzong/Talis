/**
 * @file screen_disconnected.c
 * @brief S2 断连提醒页
 *
 * 注意：此页面不提供「Find my phone」—— 断连状态下指令物理上发不出去，
 * 给个点了没反应的按钮比不给更糟。
 * 但「I'm lost」必须可用，那正是最需要它的场景。
 */

#include "tal_api.h"
#include "talis_screens.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_strings.h"

static lv_obj_t *sg_scr = NULL;

static void disc_init(void)
{
    sg_scr = talis_ui_screen_create();



    lv_obj_t *title = talis_ui_header(sg_scr, LV_SYMBOL_WARNING " " TALIS_TXT_DISC_TITLE, false, false);
    lv_obj_set_style_text_color(title, TALIS_COLOR_WARN, LV_PART_MAIN);

    talis_ui_text(sg_scr,
                  TALIS_TXT_DISC_BODY,
                  110, LV_TEXT_ALIGN_CENTER, TALIS_FONT_BODY, TALIS_COLOR_TEXT_DIM);

    talis_ui_button(sg_scr, TALIS_TXT_DISMISS, -8, TALIS_COLOR_CARD, EVENT_DISMISS);
    talis_ui_button(sg_scr, TALIS_TXT_FIND_PHONE, -62, TALIS_COLOR_PRIMARY, EVENT_FIND_PHONE);
}

static void disc_deinit(void)
{
    sg_scr = NULL;
}

Screen_t screen_disconnected = {
    .init       = disc_init,
    .deinit     = disc_deinit,
    .screen_obj = &sg_scr,
    .name       = "disconnected",
    .state_data = NULL,
};
