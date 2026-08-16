/**
 * @file screen_list.c
 * @brief 通用列表页
 *
 * 显示当前节点的子节点。树的每一层都用这一个页面，
 * 首屏（模块列表）和「手机已经丢了」下面的三个选项走的是同一份代码。
 */

#include "tal_api.h"
#include "talis_screens.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_strings.h"
#include "talis_ble.h"
#include "talis_storage.h"

static lv_obj_t          *sg_scr  = NULL;
static lv_obj_t          *sg_dot  = NULL;
static const talis_node_t *sg_node = NULL;   /* NULL 表示根，显示模块列表 */
static const char        *sg_title = NULL;
static const char        *sg_sub   = NULL;

void screen_list_set(const talis_node_t *node)
{
    const talis_device_pack_t *pack = talis_storage_get_pack();

    sg_node = node;

    if (NULL == node) {
        /* 根：标题用城市和日期，让用户确认设备里装的是这趟行程 */
        sg_title = pack->city;
        sg_sub   = pack->dates;
    } else {
        sg_title = node->title;
        sg_sub   = node->hint;
    }
}

void screen_list_refresh(void)
{
    if (NULL == sg_scr || NULL == sg_dot) {
        return;
    }

    lv_obj_set_style_bg_color(sg_dot,
                              talis_ble_is_ready() ? TALIS_COLOR_OK : TALIS_COLOR_TEXT_DIM,
                              LV_PART_MAIN);
}

static void list_init(void)
{
    const talis_device_pack_t *pack = talis_storage_get_pack();

    const talis_node_t *items = sg_node ? sg_node->children : pack->modules;
    uint8_t count = sg_node ? sg_node->child_count : pack->module_count;

    sg_scr = talis_ui_screen_create();

    /* 顶部标题区 */
    lv_obj_t *title = lv_label_create(sg_scr);
    lv_label_set_text(title, sg_title ? sg_title : "");
    lv_obj_set_style_text_color(title, TALIS_COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, TALIS_FONT_TITLE, LV_PART_MAIN);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title, TALIS_SCREEN_W - 2 * TALIS_PAD - 14);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, TALIS_PAD, 12);

    if (sg_sub && sg_sub[0]) {
        lv_obj_t *sub = lv_label_create(sg_scr);
        lv_label_set_text(sub, sg_sub);
        lv_obj_set_style_text_color(sub, TALIS_COLOR_TEXT_DIM, LV_PART_MAIN);
        lv_obj_set_style_text_font(sub, TALIS_FONT_BODY, LV_PART_MAIN);
        lv_label_set_long_mode(sub, LV_LABEL_LONG_DOT);
        lv_obj_set_width(sub, TALIS_SCREEN_W - 2 * TALIS_PAD);
        lv_obj_align(sub, LV_ALIGN_TOP_LEFT, TALIS_PAD, 38);
    }

    /* 只有根页面显示 BLE 状态点 —— 深入之后它没有意义 */
    sg_dot = NULL;
    if (NULL == sg_node) {
        sg_dot = lv_obj_create(sg_scr);
        lv_obj_set_size(sg_dot, 9, 9);
        lv_obj_set_style_radius(sg_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_border_width(sg_dot, 0, LV_PART_MAIN);
        lv_obj_align(sg_dot, LV_ALIGN_TOP_RIGHT, -TALIS_PAD, 18);
        lv_obj_clear_flag(sg_dot, LV_OBJ_FLAG_SCROLLABLE);
        screen_list_refresh();
    }

    /* 列表区。项数不定，可滚动 */
    bool is_root = (NULL == sg_node);
    lv_coord_t top = 62;
    lv_coord_t h   = TALIS_SCREEN_H - top - (is_root ? 6 : 62);

    lv_obj_t *area = talis_ui_scroll_area(sg_scr, top, h);
    lv_obj_set_flex_flow(area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(area, 8, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(area, TALIS_PAD, LV_PART_MAIN);

    for (uint8_t i = 0; i < count; i++) {
        talis_ui_list_item(area, &items[i], i);
    }

    /* 根页面不需要返回按钮，长按实体键随时回家 */
    if (!is_root) {
        talis_ui_button(sg_scr, TALIS_TXT_BACK, -8, TALIS_COLOR_CARD, EVENT_BACK);
    }
}

static void list_deinit(void)
{
    sg_scr = NULL;
    sg_dot = NULL;
}

Screen_t screen_list = {
    .init       = list_init,
    .deinit     = list_deinit,
    .screen_obj = &sg_scr,
    .name       = "list",
    .state_data = NULL,
};
