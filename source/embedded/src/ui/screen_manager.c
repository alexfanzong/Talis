/**
 * @file screen_manager.c
 * @brief 栈式页面管理器实现
 */

#include "tal_api.h"
#include "screen_manager.h"

/* 0 = 不做转场动画。
 * 240x320 全屏刷新走 SPI 约 26ms，200ms 动画只够画 8 帧，
 * 既顿挫又会在动画期间吞掉触摸点击。即时切换反而干脆。 */
#define SCREEN_ANIM_MS   0

typedef struct {
    Screen_t *screens[SCREEN_STACK_MAX_DEPTH];
    int8_t    top;   /* 栈内元素个数 */
} ScreenStack_t;

static ScreenStack_t sg_stack = {.top = 0};
static Screen_t     *sg_home  = NULL;

/***********************************************************
********************* 栈操作 *******************************
***********************************************************/
static bool __stack_is_empty(void)
{
    return (sg_stack.top <= 0);
}

static bool __stack_push(Screen_t *screen)
{
    if (sg_stack.top >= SCREEN_STACK_MAX_DEPTH) {
        PR_ERR("screen stack full (%d), refuse to push [%s]",
               SCREEN_STACK_MAX_DEPTH, screen ? screen->name : "null");
        return false;
    }
    sg_stack.screens[sg_stack.top++] = screen;
    return true;
}

static Screen_t *__stack_pop(void)
{
    if (__stack_is_empty()) {
        return NULL;
    }
    return sg_stack.screens[--sg_stack.top];
}

static Screen_t *__stack_top(void)
{
    if (__stack_is_empty()) {
        return NULL;
    }
    return sg_stack.screens[sg_stack.top - 1];
}

/**
 * @brief 建立页面并做转场
 *
 * @param screen  目标页面
 * @param anim    转场动画
 *
 * 注意 lv_scr_load_anim 最后一个参数 auto_del=true：旧页面对象会被 LVGL 删除。
 * 因此调用方必须先调用旧页面的 deinit() 释放它自己持有的资源（定时器、
 * 订阅等），否则那些资源会指向已释放的 lv_obj。
 */
static void __screen_show(Screen_t *screen, lv_scr_load_anim_t anim)
{
    if (NULL == screen || NULL == screen->init) {
        PR_ERR("invalid screen");
        return;
    }

    screen->init();

    if (NULL == screen->screen_obj || NULL == *(screen->screen_obj)) {
        PR_ERR("screen [%s] init did not create an object", screen->name);
        return;
    }

    PR_DEBUG("screen -> [%s], depth=%d", screen->name, sg_stack.top);
    lv_scr_load_anim(*(screen->screen_obj), anim, SCREEN_ANIM_MS, 0, true);
}

/***********************************************************
********************* 对外接口 *****************************
***********************************************************/
Screen_t *screen_get_now_screen(void)
{
    return __stack_top();
}

uint8_t screen_stack_depth(void)
{
    return (uint8_t)(sg_stack.top < 0 ? 0 : sg_stack.top);
}

void screen_load(Screen_t *newScreen)
{
    if (NULL == newScreen) {
        return;
    }

    if (sg_stack.top >= SCREEN_STACK_MAX_DEPTH) {
        PR_ERR("screen stack full, cannot load [%s]", newScreen->name);
        return;
    }

    /* 注意：这里**不能**因为「栈顶已经是同一个 Screen_t」就跳过。
     *
     * screen_list / screen_detail 是通用页面，树的每一层都复用同一个对象，
     * 靠 screen_list_set() 切换要展开的节点。曾经这里有一道
     * 「栈顶相同就 return」的保护，结果列表→列表的跳转全被吃掉：
     * 数据层已经进入下一层，屏幕却还停在上一层，用户点第二行看着是
     * "医疗求助"，实际被当成新一层的第二项，表现为"选完手机丢了直接跳到回到住处"。
     *
     * 复用同一个对象是安全的：下面会先 deinit 旧的（把静态指针置空），
     * 再 init 出新的 lv_obj，旧对象由 lv_scr_load_anim(auto_del=true) 删除。
     * 防连点由栈深上限兜底。 */
    Screen_t *cur = __stack_top();
    if (cur && cur->deinit) {
        cur->deinit();
    }

    if (!__stack_push(newScreen)) {
        return;
    }

    /* 竖屏：新页面从下方推入 */
    __screen_show(newScreen, LV_SCR_LOAD_ANIM_OVER_TOP);
}

void screen_back(void)
{
    if (__stack_is_empty()) {
        return;
    }

    /* 已经在首页，不再返回 */
    if (sg_stack.top <= 1) {
        PR_DEBUG("already at home screen, ignore back");
        return;
    }

    Screen_t *cur = __stack_top();
    if (cur && cur->deinit) {
        cur->deinit();
    }
    __stack_pop();

    Screen_t *prev = __stack_top();
    if (NULL == prev) {
        prev = sg_home;
        __stack_push(prev);
    }

    __screen_show(prev, LV_SCR_LOAD_ANIM_OVER_BOTTOM);
}

void screen_back_bottom(void)
{
    if (__stack_is_empty()) {
        return;
    }

    Screen_t *cur = __stack_top();
    if (cur && cur->deinit) {
        cur->deinit();
    }

    /* 弹到只剩栈底。中间页面此时并未 init，不需要 deinit */
    while (sg_stack.top > 1) {
        __stack_pop();
    }

    __screen_show(__stack_top(), LV_SCR_LOAD_ANIM_OVER_BOTTOM);
}

void screen_manager_init(Screen_t *home)
{
    sg_stack.top = 0;
    sg_home      = home;

    if (NULL == home) {
        PR_ERR("home screen is NULL");
        return;
    }

    __stack_push(home);
    home->init();

    if (home->screen_obj && *(home->screen_obj)) {
        lv_scr_load(*(home->screen_obj));
        PR_NOTICE("screen manager started at [%s]", home->name);
    } else {
        PR_ERR("home screen [%s] init failed", home->name);
    }
}
