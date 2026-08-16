/**
 * @file talis_ui.c
 * @brief 页面公共构件实现
 */

#include "tal_api.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_event_queue.h"

/* 点击回调携带的上下文，随对象删除一起释放 */
typedef struct {
    TalisEvent_t evt;
    uint32_t     param;
} talis_click_ctx_t;

static void __click_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    talis_click_ctx_t *ctx = (talis_click_ctx_t *)lv_event_get_user_data(e);
    if (NULL == ctx) {
        return;
    }

    PR_DEBUG("click -> evt %d param %u", ctx->evt, (unsigned)ctx->param);
    talis_event_post_param(ctx->evt, ctx->param);
}

static void __delete_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_DELETE) {
        return;
    }

    talis_click_ctx_t *ctx = (talis_click_ctx_t *)lv_event_get_user_data(e);
    if (ctx) {
        lv_mem_free(ctx);
    }
}

/* 给对象挂上"点击后投递事件"的行为 */
static void __attach_click(lv_obj_t *obj, TalisEvent_t evt, uint32_t param)
{
    talis_click_ctx_t *ctx = (talis_click_ctx_t *)lv_mem_alloc(sizeof(talis_click_ctx_t));
    if (NULL == ctx) {
        PR_ERR("alloc click ctx failed");
        return;
    }

    ctx->evt   = evt;
    ctx->param = param;

    lv_obj_add_event_cb(obj, __click_cb, LV_EVENT_CLICKED, ctx);
    lv_obj_add_event_cb(obj, __delete_cb, LV_EVENT_DELETE, ctx);
}

lv_obj_t *talis_ui_screen_create(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);

    lv_obj_set_size(scr, TALIS_SCREEN_W, TALIS_SCREEN_H);
    lv_obj_set_style_bg_color(scr, TALIS_COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(scr, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr, 0, LV_PART_MAIN);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    return scr;
}

lv_obj_t *talis_ui_header(lv_obj_t *parent, const char *title, bool show_dot, bool dot_ok)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, title);
    lv_obj_set_style_text_color(label, TALIS_COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, TALIS_FONT_TITLE, LV_PART_MAIN);
    /* 标题固定单行。用 WRAP 的话长标题会换行，撞到下面固定位置的内容区 */
    lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(label, TALIS_SCREEN_W - 2 * TALIS_PAD - (show_dot ? 16 : 0));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, TALIS_PAD, 14);

    if (show_dot) {
        lv_obj_t *dot = lv_obj_create(parent);
        lv_obj_set_size(dot, 10, 10);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(dot, dot_ok ? TALIS_COLOR_OK : TALIS_COLOR_TEXT_DIM,
                                  LV_PART_MAIN);
        lv_obj_align(dot, LV_ALIGN_TOP_RIGHT, -TALIS_PAD, 20);
        lv_obj_clear_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    }

    return label;
}

lv_obj_t *talis_ui_text(lv_obj_t *parent, const char *text, lv_coord_t y_ofs,
                        lv_text_align_t align, const lv_font_t *font, lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);

    lv_label_set_text(label, text ? text : "");
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, font, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, align, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, TALIS_SCREEN_W - 2 * TALIS_PAD);

    if (align == LV_TEXT_ALIGN_CENTER) {
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y_ofs);
    } else {
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, TALIS_PAD, y_ofs);
    }

    return label;
}

lv_obj_t *talis_ui_scroll_area(lv_obj_t *parent, lv_coord_t y_top, lv_coord_t height)
{
    lv_obj_t *area = lv_obj_create(parent);

    lv_obj_set_size(area, TALIS_SCREEN_W, height);
    lv_obj_align(area, LV_ALIGN_TOP_MID, 0, y_top);
    lv_obj_set_style_bg_opa(area, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(area, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(area, 0, LV_PART_MAIN);

    /* 只允许竖向滚动，横向滑动会让文字左右晃 */
    lv_obj_set_scroll_dir(area, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(area, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_bg_color(area, TALIS_COLOR_TEXT_DIM, LV_PART_SCROLLBAR);

    return area;
}

lv_obj_t *talis_ui_button(lv_obj_t *parent, const char *text, lv_coord_t y_ofs,
                          lv_color_t color, TalisEvent_t evt)
{
    lv_obj_t *btn = lv_btn_create(parent);

    lv_obj_set_size(btn, TALIS_SCREEN_W - 2 * TALIS_PAD, 48);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, y_ofs);

    lv_obj_set_style_bg_color(btn, color, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, TALIS_COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, TALIS_FONT_BTN, LV_PART_MAIN);
    lv_obj_center(label);

    __attach_click(btn, evt, 0);

    return btn;
}

lv_obj_t *talis_ui_list_item(lv_obj_t *parent, const talis_node_t *node, uint8_t index)
{
    /* 整行可点，不画小箭头 —— 慌乱状态下点得准比好看重要 */
    lv_obj_t *item = lv_btn_create(parent);

    bool two_line = (node->hint && node->hint[0]);

    lv_obj_set_width(item, TALIS_SCREEN_W - 2 * TALIS_PAD);
    lv_obj_set_height(item, two_line ? 54 : 42);

    lv_obj_set_style_bg_color(item, TALIS_COLOR_CARD, LV_PART_MAIN);
    lv_obj_set_style_radius(item, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(item, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(item, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(item, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(item, 6, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(item);
    lv_label_set_text(title, node->title ? node->title : "");
    lv_obj_set_style_text_color(title, TALIS_COLOR_TEXT, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, TALIS_FONT_BTN, LV_PART_MAIN);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(title, TALIS_SCREEN_W - 2 * TALIS_PAD - 24);
    lv_obj_align(title, two_line ? LV_ALIGN_TOP_LEFT : LV_ALIGN_LEFT_MID, 0, two_line ? 0 : 0);

    if (two_line) {
        lv_obj_t *hint = lv_label_create(item);
        lv_label_set_text(hint, node->hint);
        lv_obj_set_style_text_color(hint, TALIS_COLOR_TEXT_DIM, LV_PART_MAIN);
        lv_obj_set_style_text_font(hint, TALIS_FONT_BODY, LV_PART_MAIN);
        lv_label_set_long_mode(hint, LV_LABEL_LONG_DOT);
        lv_obj_set_width(hint, TALIS_SCREEN_W - 2 * TALIS_PAD - 24);
        lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    }

    __attach_click(item, EVENT_OPEN_CHILD, (uint32_t)index);

    return item;
}

void talis_ui_button_set_enabled(lv_obj_t *btn, bool enabled)
{
    if (NULL == btn) {
        return;
    }

    if (enabled) {
        lv_obj_clear_state(btn, LV_STATE_DISABLED);
        lv_obj_set_style_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    } else {
        lv_obj_add_state(btn, LV_STATE_DISABLED);
        lv_obj_set_style_opa(btn, LV_OPA_40, LV_PART_MAIN);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    }
}
