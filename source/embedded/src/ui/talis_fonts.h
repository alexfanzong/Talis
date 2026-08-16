/**
 * @file talis_fonts.h
 * @brief Talis 自建字体
 *
 * ============================================================================
 * 为什么不用 LVGL 内置的 Montserrat
 * ============================================================================
 * 内置 lv_font_montserrat_* 的字符范围是 `-r 0x20-0x7F,0xB0,0x2022`，
 * 只有 ASCII。而 Talis 的求助卡是法语：
 *
 *   "Mon téléphone a disparu. Je ne peux pas accéder aux informations
 *    de mon hébergement ni passer d'appel..."
 *
 * é è à ç ô û 这些字符不在内置字体里，直接显示会变成空白或方块 ——
 * 一张给法国路人看的求助卡如果重音全丢，可读性和专业度都会塌掉。
 *
 * ============================================================================
 * 这套字体怎么来的
 * ============================================================================
 * 用 LVGL 官方的 lv_font_conv (github.com/lvgl/lv_font_conv, MIT) 生成，
 * 字体源文件就是 SDK 自带的 Montserrat-Medium.ttf，所以字形风格与内置版一致。
 *
 * 字符范围：
 *   0x20-0x7F      ASCII
 *   0xA0-0xFF      Latin-1 补充 —— 法语重音、« » · ° 全覆盖
 *   0x2013-0x2014  – —
 *   0x2018-0x2019  ' '        （数据里用的是弯引号）
 *   0x201C-0x201D  " "
 *   0x2022, 0x20AC •  €
 *   + FontAwesome 图标（与内置字体同一份列表，LV_SYMBOL_* 照常可用）
 *
 * 重新生成命令见 tools/make_fonts.sh
 */

#ifndef __TALIS_FONTS_H__
#define __TALIS_FONTS_H__

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

LV_FONT_DECLARE(lv_font_talis_14);
LV_FONT_DECLARE(lv_font_talis_16);
LV_FONT_DECLARE(lv_font_talis_20);
LV_FONT_DECLARE(lv_font_talis_28);

/* 语义别名：页面里用这些，将来换字号只改这里 */
#define TALIS_FONT_BODY     (&lv_font_talis_14)
#define TALIS_FONT_BTN      (&lv_font_talis_16)
#define TALIS_FONT_TITLE    (&lv_font_talis_20)
#define TALIS_FONT_BIG      (&lv_font_talis_28)

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_FONTS_H__ */
