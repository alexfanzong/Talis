/**
 * @file talis_strings.h
 * @brief 界面文案集中定义
 *
 * ============================================================================
 * 语言策略
 * ============================================================================
 * 目标用户是中国女性独行旅行者，所以：
 *
 *   界面（给用户自己看）        —— 中文
 *   求助卡正文（给当地人看）    —— 法语在前
 *   求助卡译文（给用户核对）    —— 中文
 *   地址 / 机构名 / 电话        —— 保留原文，因为要指给当地人看
 *
 * 不放英文：路人读法语，用户读中文，英文对两边都不解决问题。
 *
 * ============================================================================
 * 字体依赖
 * ============================================================================
 * 本文件里出现的每一个汉字都必须在 src/ui/fonts/lv_font_talis_*.c 里有字形。
 * 字库是按实际用到的字符做的子集，不是完整字库。
 *
 * ⚠️ 改了这里的中文之后，必须重新跑 tools/make_fonts.sh，
 *    否则新增的字会显示成空白。脚本会自动扫描源码里的汉字，不用手工列。
 */

#ifndef __TALIS_STRINGS_H__
#define __TALIS_STRINGS_H__

/* ---- 品牌 ---- */
#define TALIS_TXT_BRAND_CN  "行符"

/* ---- 旧的六入口标题已并入 talis_pack_*.c 的节点树 ---- */

/* ---- 按钮 ---- */
#define TALIS_TXT_BACK      "返回"
#define TALIS_TXT_PLAY_FR   "播放法语"
#define TALIS_TXT_STOP      "停止"
#define TALIS_TXT_DISMISS   "知道了"
#define TALIS_TXT_CANCEL    "取消"
#define TALIS_TXT_FIND_PHONE "寻找我的手机"

/* ---- 字段标签 ---- */
#define TALIS_TXT_NEXT      "下一步"
#define TALIS_TXT_KEEP      "需保留"
#define TALIS_TXT_SOURCE    "数据来源"
#define TALIS_TXT_CHINESE   "中文"

/* ---- 各页面文案 ---- */
#define TALIS_TXT_BOOT_HINT     "离线可用 · 无需联网"

#define TALIS_TXT_DISC_TITLE    "手机不在身边"
#define TALIS_TXT_DISC_BODY     "手机已离开范围。\n先检查随身的包和口袋。"

#define TALIS_TXT_RING_TITLE    "正在呼叫手机"
#define TALIS_TXT_RING_BODY     "留意附近的响铃声。"
#define TALIS_TXT_RING_OK       "已发出信号"
#define TALIS_TXT_RING_FAIL     "手机未连接"

#endif /* __TALIS_STRINGS_H__ */
