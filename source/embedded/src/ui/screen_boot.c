/**
 * @file screen_boot.c
 * @brief 开机页：品牌标识
 */

#include "tal_api.h"
#include "talis_screens.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_strings.h"

static lv_obj_t *sg_scr = NULL;

static void boot_init(void)
{
    sg_scr = talis_ui_screen_create();

    /* 英文名在上，中文名在下。开机一眼就知道这是什么设备 */
    lv_obj_t *brand = lv_label_create(sg_scr);
    lv_label_set_text(brand, "TALIS");
    lv_obj_set_style_text_color(brand, TALIS_COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(brand, TALIS_FONT_BIG, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(brand, 6, LV_PART_MAIN);
    lv_obj_align(brand, LV_ALIGN_CENTER, 0, -26);

    lv_obj_t *cn = lv_label_create(sg_scr);
    lv_label_set_text(cn, TALIS_TXT_BRAND_CN);
    lv_obj_set_style_text_color(cn, TALIS_COLOR_PRIMARY, LV_PART_MAIN);
    lv_obj_set_style_text_font(cn, TALIS_FONT_TITLE, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(cn, 8, LV_PART_MAIN);
    lv_obj_align(cn, LV_ALIGN_CENTER, 0, 8);

    /* 一条细分隔线，让品牌区看起来是刻意排版的 */
    lv_obj_t *line = lv_obj_create(sg_scr);
    lv_obj_set_size(line, 48, 2);
    lv_obj_set_style_bg_color(line, TALIS_COLOR_CARD, LV_PART_MAIN);
    lv_obj_set_style_border_width(line, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(line, 0, LV_PART_MAIN);
    lv_obj_align(line, LV_ALIGN_CENTER, 0, 40);

    lv_obj_t *tag = lv_label_create(sg_scr);
    lv_label_set_text(tag, TALIS_TXT_BOOT_HINT);
    lv_obj_set_style_text_color(tag, TALIS_COLOR_TEXT_DIM, LV_PART_MAIN);
    lv_obj_set_style_text_font(tag, TALIS_FONT_BODY, LV_PART_MAIN);
    lv_obj_align(tag, LV_ALIGN_BOTTOM_MID, 0, -28);
}

static void boot_deinit(void)
{
    sg_scr = NULL;
}

Screen_t screen_boot = {
    .init       = boot_init,
    .deinit     = boot_deinit,
    .screen_obj = &sg_scr,
    .name       = "boot",
    .state_data = NULL,
};
