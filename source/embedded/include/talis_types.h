/**
 * @file talis_types.h
 * @brief Talis 公共类型定义
 *
 * ============================================================================
 * 信息结构：节点树
 * ============================================================================
 * 整个内容是一棵树，每个节点要么有子节点（显示成列表页），
 * 要么没有（显示成详情页）。层级不限，不用为每一层写一个页面。
 *
 *   根
 *   ├ 手机问题
 *   │   ├ 寻找我的手机        ← 动作节点，点了直接让手机响铃+震动
 *   │   └ 手机已经丢了
 *   │       ├ 出示求助卡      ← 法/中双语 + 语音
 *   │       ├ 回到住处
 *   │       └ 找警局
 *   ├ 医疗求助
 *   ├ 领事协助
 *   └ ...
 *
 * 加模块、加层级只改 talis_pack_*.c，界面代码一行都不用动。
 */

#ifndef __TALIS_TYPES_H__
#define __TALIS_TYPES_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 节点被点击时执行的动作 */
typedef enum {
    ACTION_NONE = 0,     /* 普通节点：有子节点就进列表，没有就进详情 */
    ACTION_FIND_PHONE,   /* 让手机响铃：走 BLE，同时本机震动 */
} talis_action_t;

/**
 * @brief 内容节点
 *
 * 所有字段都可为 NULL / 0，为空的不显示。
 */
typedef struct talis_node {
    const char *title;
    const char *hint;       /* 列表里标题下的一行小字 */

    /* 详情内容 */
    const char *body_fr;    /* 法语正文，给当地人看 */
    const char *body_zh;    /* 中文正文，给用户看 */
    const char *place;      /* 机构 / 地点名，保留原文，要指给当地人看 */
    const char *address;
    const char *phone;
    const char *next_step;  /* 下一步做什么 */
    const char *keep;       /* 需要留存什么凭证 */

    bool has_audio;         /* 详情页是否出现语音播放按钮 */
    talis_action_t action;

    /* 子节点。child_count > 0 时本节点显示为列表页 */
    const struct talis_node *children;
    uint8_t child_count;
} talis_node_t;

/** @brief 整份设备包 */
typedef struct {
    const char        *city;
    const char        *dates;
    const talis_node_t *modules;
    uint8_t             module_count;
} talis_device_pack_t;

/***********************************************************
********************* 状态机 *******************************
***********************************************************/
typedef enum {
    STATE_BOOT = 0,
    STATE_BROWSE,         /* 在树里浏览：列表页或详情页 */
    STATE_DISCONNECTED,
    STATE_FIND_PHONE,
    STATE_FIND_RESULT,
    STATE_MAX
} TalisState_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_BOOT_DONE,
    EVENT_BLE_CONNECTED,
    EVENT_BLE_DISCONNECTED,
    EVENT_BLE_READY,
    EVENT_TIMEOUT_3S,
    EVENT_DISMISS,
    EVENT_FIND_PHONE,
    EVENT_PHONE_RING_ACK,
    EVENT_PHONE_RING_FAIL,
    EVENT_BACK,
    EVENT_BACK_LONG,
    EVENT_FINISH,
    EVENT_PLAY_AUDIO,
    EVENT_OPEN_CHILD,     /* param = 当前节点的第几个子节点 */
    EVENT_MAX
} TalisEvent_t;

typedef struct {
    TalisEvent_t evt;
    uint32_t     param;
} TalisEventMsg_t;

const char *talis_state_name(TalisState_t st);
const char *talis_event_name(TalisEvent_t ev);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_TYPES_H__ */
