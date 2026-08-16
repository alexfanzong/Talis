/**
 * @file talis_ui.h
 * @brief 页面公共构件：统一配色、标题、正文、按钮、列表项
 */

#ifndef __TALIS_UI_H__
#define __TALIS_UI_H__

#include "lvgl.h"
#include "talis_types.h"
#include "talis_fonts.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 建一个 Talis 风格的空白页面（深色底，240x320，不可滚动） */
lv_obj_t *talis_ui_screen_create(void);

/** 顶部标题栏（标题 + 可选右上角状态点） */
lv_obj_t *talis_ui_header(lv_obj_t *parent, const char *title, bool show_dot, bool dot_ok);

/**
 * @brief 正文文字（自动换行）
 * @param align  对齐方式，求助卡用 LEFT，提示用 CENTER
 * @param y_ofs  相对顶部的偏移
 */
lv_obj_t *talis_ui_text(lv_obj_t *parent, const char *text, lv_coord_t y_ofs,
                        lv_text_align_t align, const lv_font_t *font, lv_color_t color);

/**
 * @brief 可滚动的内容区
 *
 * 求助卡和详情页内容长度不定，240x320 放不下时必须能滑动，
 * 否则地址电话被截断，那正是最需要看到的信息。
 */
lv_obj_t *talis_ui_scroll_area(lv_obj_t *parent, lv_coord_t y_top, lv_coord_t height);

/**
 * @brief 底部大按钮
 * @param y_ofs 相对底部的偏移（负值向上）
 */
lv_obj_t *talis_ui_button(lv_obj_t *parent, const char *text, lv_coord_t y_ofs,
                          lv_color_t color, TalisEvent_t evt);

/**
 * @brief 列表项（整行可点，标题 + 可选副标题）
 *
 * 点击后投递 EVENT_OPEN_CHILD，param 为 index。
 *
 * @param parent flex 容器
 * @param node   要显示的节点
 * @param index  它是父节点的第几个子节点
 */
lv_obj_t *talis_ui_list_item(lv_obj_t *parent, const talis_node_t *node, uint8_t index);

/** 设置按钮是否可用（置灰） */
void talis_ui_button_set_enabled(lv_obj_t *btn, bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_UI_H__ */
