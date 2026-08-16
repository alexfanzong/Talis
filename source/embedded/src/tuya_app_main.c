/**
 * @file tuya_app_main.c
 * @brief Talis 应用入口
 *
 * 启动顺序不能乱：
 *   1. 日志
 *   2. talis_board_register_hardware()  —— 板级外设注册（本地板级，非 SDK 的）
 *   3. tal_sw_timer_init()              —— 必须早于任何 tal_sw_timer_create()
 *   4. talis_haptic_init()              —— 板级之后（引脚归属已定）+ 定时器之后
 *   5. 事件队列 / 存储 / 音频
 *   6. lv_vendor_init()                 —— LVGL 初始化，此时还没有 LVGL 任务
 *   7. 建 UI + 状态机                    —— 在 LVGL 任务启动前建，无需加锁
 *   8. lv_vendor_start()                —— LVGL 任务起来
 *   9. 输入 / BLE                        —— 回调只投事件，不碰 lv_*
 *
 * 第 3 步曾经排在第 4 步之后，导致 hard fault，屏幕全程无反应。详见下方注释。
 */

#include "tal_api.h"
#include "tkl_output.h"

#include "lvgl.h"
#include "lv_vendor.h"

#include "talis_board.h"
#include "talis_app_config.h"
#include "talis_event_queue.h"
#include "talis_state_machine.h"
#include "talis_haptic.h"
#include "talis_input.h"
#include "talis_ble.h"
#include "talis_audio.h"
#include "talis_storage.h"

static void user_main(void)
{
    OPERATE_RET rt = OPRT_OK;

    tal_log_init(TAL_LOG_LEVEL_DEBUG, 1024, (TAL_LOG_OUTPUT_CB)tkl_log_output);

    PR_NOTICE("========================================");
    PR_NOTICE(" Talis  v%s", PROJECT_VERSION);
    PR_NOTICE(" board : %s", PLATFORM_BOARD);
    PR_NOTICE(" chip  : %s", PLATFORM_CHIP);
    PR_NOTICE(" build : %s %s", __DATE__, __TIME__);
    PR_NOTICE("========================================");

    /* --- 1. 板级外设 --- */
    rt = talis_board_register_hardware();
    if (rt != OPRT_OK) {
        PR_ERR("board register failed: %d", rt);
    }

    /* --- 2. 软件定时器子系统 ---
     * 必须排在所有会调用 tal_sw_timer_create() 的模块之前
     * （haptic / ble / state_machine 都会）。
     *
     * tal_sw_timer_create() 内部直接 tal_mutex_lock(s_timer_mgr.mutex) 并往
     * s_timer_mgr.list_standby 插节点，而这两者都是在 tal_sw_timer_init() 里
     * 才创建/初始化的。顺序颠倒 = 锁 NULL 信号量 + 往零地址链表插节点 = hard fault，
     * 应用线程当场死掉，后面的 lv_vendor_init() 永远执行不到，
     * 症状是「屏幕从头到尾一点反应都没有」。 */
    TUYA_CALL_ERR_LOG(tal_sw_timer_init());

    /* --- 3. 震动马达（板级注册之后，定时器之后）--- */
    rt = talis_haptic_init();
    if (rt != OPRT_OK) {
        PR_ERR("haptic init failed: %d", rt);
    }

    /* --- 4. 其余基础设施 --- */
    TUYA_CALL_ERR_LOG(talis_event_queue_init());
    TUYA_CALL_ERR_LOG(talis_storage_init());
    TUYA_CALL_ERR_LOG(talis_audio_init());

    /* --- 4. LVGL 初始化（还没起任务）--- */
    lv_vendor_init(DISPLAY_NAME);

#if TALIS_MINIMAL_UI
    /* --- 调试模式：只画一个 label，跟 SDK lvgl_label 例子等价 ---
     * 用来判断故障是在 UI 层还是更底层，见 talis_app_config.h 的说明 */
    PR_WARN("*** TALIS_MINIMAL_UI=1: state machine SKIPPED, drawing test label ***");
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN);
    lv_obj_t *dbg_label = lv_label_create(lv_scr_act());
    lv_label_set_text(dbg_label, "TALIS MIN\nLVGL v8\nROT 0");
    lv_obj_set_style_text_color(dbg_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_align(dbg_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(dbg_label, LV_ALIGN_CENTER, 0, 0);
#else
    /* --- 5. 建首页 UI + 挂状态机定时器 ---
     * 此时 LVGL 任务还没启动，建 UI 不需要加锁。
     * 状态机本身注册成 lv_timer，之后跑在 LVGL 任务里（锁内），
     * 因此状态机里可以自由调用 lv_*，不需要也不允许再手动加锁。 */
    talis_state_machine_init();
#endif

    /* --- 6. 启动 LVGL 任务 --- */
    lv_vendor_start(TALIS_LVGL_TASK_PRIO, TALIS_LVGL_TASK_STACK);

    /* --- 7. 输入与 BLE：回调只投事件队列，不碰 lv_* --- */
    TUYA_CALL_ERR_LOG(talis_input_init());
    /* 只做初始化。广播不在这里开 —— tal_ble_bt_init 是异步的，
     * 协议栈就绪会回调 TAL_BLE_STACK_INIT，由那里延迟启动广播。
     * 在这里抢跑会导致手机扫得到但连不上（GATT 服务表还没建好）。 */
    TUYA_CALL_ERR_LOG(talis_ble_init());

    PR_NOTICE("Talis started, entering LVGL-driven event loop.");

    /* 初始化完成，通知状态机离开 BOOT 页 */
    talis_event_post(EVENT_BOOT_DONE);

    /* 本线程使命结束。此后一切由 LVGL 任务（状态机）和各回调驱动。 */
}

/**
 * @brief main
 */
#if OPERATING_SYSTEM == SYSTEM_LINUX
void main(int argc, char *argv[])
{
    user_main();
}
#else

/* Tuya thread handle */
static THREAD_HANDLE ty_app_thread = NULL;

static void tuya_app_thread(void *arg)
{
    user_main();

    tal_thread_delete(ty_app_thread);
    ty_app_thread = NULL;
}

void tuya_app_main(void)
{
    THREAD_CFG_T thrd_param = {0};
    thrd_param.stackDepth = 1024 * 8;
    thrd_param.priority = THREAD_PRIO_1;
    thrd_param.thrdname = "tuya_app_main";

    tal_thread_create_and_start(&ty_app_thread, NULL, NULL, tuya_app_thread, NULL, &thrd_param);
}
#endif
