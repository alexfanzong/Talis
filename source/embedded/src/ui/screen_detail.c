/**
 * @file screen_detail.c
 * @brief 通用详情页
 *
 * 显示一个叶子节点的全部内容。哪些字段有值就显示哪些，没有的不留空位。
 *
 * 布局用 LVGL flex 自动堆叠，不手算 y 坐标。
 * 早先版本按 strlen/32 估行数再累加 y —— strlen 数的是字节
 * （法语重音 2 字节、汉字 3 字节），且长词会提前换行，估少了就会重叠，
 * 那就是之前看到的"字挤在一起"。flex 由 LVGL 按实际高度排版，从根上没这问题。
 */

#include "tal_api.h"
#include "talis_screens.h"
#include "talis_ui.h"
#include "talis_app_config.h"
#include "talis_strings.h"
#include "talis_audio.h"

static lv_obj_t          *sg_scr      = NULL;
static lv_obj_t          *sg_btn_play = NULL;
static const talis_node_t *sg_node    = NULL;

void screen_detail_set(const talis_node_t *node)
{
    sg_node = node;
}

void screen_detail_refresh_audio(void)
{
    if (NULL == sg_btn_play) {
        return;
    }

    lv_obj_t *label = lv_obj_get_child(sg_btn_play, 0);
    if (label) {
        lv_label_set_text(label, talis_audio_is_playing() ? TALIS_TXT_STOP
                                                          : TALIS_TXT_PLAY_FR);
    }
}

/* 往 flex 容器里加一段文字，位置交给 flex */
static void __text(lv_obj_t *parent, const char *text,
                   const lv_font_t *font, lv_color_t color)
{
    if (NULL == text || text[0] == '\0') {
        return;
    }

    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, text);
    lv_obj_set_style_text_color(l, color, LV_PART_MAIN);
    lv_obj_set_style_text_font(l, font, LV_PART_MAIN);
    lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
    /* 必须用像素宽度，不能用 lv_pct(100)：
     * LVGL v8 里 flex 子元素的自动高度会在百分比宽度生效前就算好，
     * 换行文字会被算矮，导致下一段压上来（"字挤在一起"）。 */
    lv_obj_set_width(l, TALIS_SCREEN_W - 2 * TALIS_PAD);
}

/* 小标签 + 内容 */
static void __field(lv_obj_t *parent, const char *label, const char *value,
                    lv_color_t color)
{
    if (NULL == value || value[0] == '\0') {
        return;
    }

    __text(parent, label, TALIS_FONT_BODY, TALIS_COLOR_TEXT_DIM);
    __text(parent, value, TALIS_FONT_BODY, color);
}

static void detail_init(void)
{
    sg_scr      = talis_ui_screen_create();
    sg_btn_play = NULL;

    if (NULL == sg_node) {
        return;
    }

    talis_ui_header(sg_scr, sg_node->title, false, false);

    bool has_audio = sg_node->has_audio && talis_audio_has_help_clip();
    lv_coord_t scroll_h = TALIS_SCREEN_H - 52 - (has_audio ? 112 : 62);

    lv_obj_t *area = talis_ui_scroll_area(sg_scr, 48, scroll_h);
    lv_obj_set_flex_flow(area, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(area, 6, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(area, TALIS_PAD, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(area, 4, LV_PART_MAIN);

    /* 地点信息在前：慌乱时最先要看的是去哪、打给谁 */
    __text(area, sg_node->place, TALIS_FONT_BTN, TALIS_COLOR_TEXT);
    __text(area, sg_node->address, TALIS_FONT_BODY, TALIS_COLOR_TEXT_DIM);
    __text(area, sg_node->phone, TALIS_FONT_BODY, TALIS_COLOR_PRIMARY);

    /* 法语在中文前 —— 这段是要举给当地人看的 */
    if (sg_node->body_fr) {
        __text(area, "FRANÇAIS", TALIS_FONT_BODY, TALIS_COLOR_PRIMARY);
        __text(area, sg_node->body_fr, TALIS_FONT_BODY, TALIS_COLOR_TEXT);
    }

    if (sg_node->body_zh) {
        if (sg_node->body_fr) {
            __text(area, TALIS_TXT_CHINESE, TALIS_FONT_BODY, TALIS_COLOR_PRIMARY);
        }
        __text(area, sg_node->body_zh, TALIS_FONT_BODY, TALIS_COLOR_TEXT_DIM);
    }

    __field(area, TALIS_TXT_NEXT, sg_node->next_step, TALIS_COLOR_TEXT);
    __field(area, TALIS_TXT_KEEP, sg_node->keep, TALIS_COLOR_WARN);

    talis_ui_button(sg_scr, TALIS_TXT_BACK, -8, TALIS_COLOR_CARD, EVENT_BACK);

    if (has_audio) {
        sg_btn_play = talis_ui_button(sg_scr,
                                      talis_audio_is_playing() ? TALIS_TXT_STOP
                                                               : TALIS_TXT_PLAY_FR,
                                      -62, TALIS_COLOR_PRIMARY, EVENT_PLAY_AUDIO);
    }
}

static void detail_deinit(void)
{
    /* 离开页面就停音频，避免声音跟到别的页面 */
    if (talis_audio_is_playing()) {
        talis_audio_stop();
    }

    sg_scr      = NULL;
    sg_btn_play = NULL;
}

Screen_t screen_detail = {
    .init       = detail_init,
    .deinit     = detail_deinit,
    .screen_obj = &sg_scr,
    .name       = "detail",
    .state_data = NULL,
};
