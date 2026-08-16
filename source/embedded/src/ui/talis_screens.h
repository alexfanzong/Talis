/**
 * @file talis_screens.h
 * @brief 所有页面的前向声明
 *
 * 页面结构对齐 MVP v0.3：首页六入口 + 一个通用详情页。
 * find_phone / find_result / disconnected 属于 v0.3 的 Phase 2B / 2C，
 * 保留但不是主线。
 */

#ifndef __TALIS_SCREENS_H__
#define __TALIS_SCREENS_H__

#include "screen_manager.h"
#include "talis_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern Screen_t screen_boot;          /* 开机品牌页 */
extern Screen_t screen_list;          /* 通用列表页（树的任意一层） */
extern Screen_t screen_detail;        /* 通用详情页（叶子节点） */
extern Screen_t screen_disconnected;  /* 断连提醒（Phase 2C） */
extern Screen_t screen_find_phone;    /* 正在呼叫手机（Phase 2B） */
extern Screen_t screen_find_result;   /* 呼叫结果（Phase 2B） */

/** 设置列表页要展开哪个节点。NULL 表示根（模块列表） */
void screen_list_set(const talis_node_t *node);

/** 按 BLE 状态刷新根列表页的状态点。不在根页时调用是安全的 */
void screen_list_refresh(void);

/** 设置详情页要显示哪个节点 */
void screen_detail_set(const talis_node_t *node);

/** 刷新详情页里播放按钮的文字（播放/停止联动） */
void screen_detail_refresh_audio(void);

/** 设置呼叫结果页的成败 */
void screen_find_result_set(bool success);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_SCREENS_H__ */
