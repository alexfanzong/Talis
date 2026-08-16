/**
 * @file talis_haptic.c
 * @brief 震动马达实现
 *
 * 设计要点：用软件定时器推进状态，而不是 tal_system_sleep() 阻塞。
 * 震动常常由 BLE 断连回调触发，若在那里阻塞 200ms x 5 会拖垮协议栈任务。
 */

#include "tal_api.h"
#include "tkl_gpio.h"
#include "tkl_pinmux.h"

#include "talis_board.h"
#include "talis_app_config.h"
#include "talis_haptic.h"

typedef enum {
    HAPTIC_IDLE = 0,
    HAPTIC_ON,
    HAPTIC_OFF,
} haptic_phase_t;

static TIMER_ID       sg_timer     = NULL;
static haptic_phase_t sg_phase     = HAPTIC_IDLE;
static uint8_t        sg_pulse_cnt = 0;
static bool           sg_inited    = false;

static void __haptic_output(bool on)
{
    tkl_gpio_write(TALIS_BOARD_HAPTIC_PIN, on ? TUYA_GPIO_LEVEL_HIGH : TUYA_GPIO_LEVEL_LOW);
}

static void __haptic_timer_cb(TIMER_ID timer_id, void *arg)
{
    (void)timer_id;
    (void)arg;

    switch (sg_phase) {
    case HAPTIC_ON:
        /* 一段震动结束 */
        __haptic_output(false);
        sg_pulse_cnt++;

        if (sg_pulse_cnt >= TALIS_HAPTIC_PULSES) {
            sg_phase = HAPTIC_IDLE;
            PR_DEBUG("haptic done (%d pulses)", sg_pulse_cnt);
            return;
        }

        sg_phase = HAPTIC_OFF;
        tal_sw_timer_start(sg_timer, TALIS_HAPTIC_OFF_MS, TAL_TIMER_ONCE);
        break;

    case HAPTIC_OFF:
        /* 间隔结束，开始下一段 */
        __haptic_output(true);
        sg_phase = HAPTIC_ON;
        tal_sw_timer_start(sg_timer, TALIS_HAPTIC_ON_MS, TAL_TIMER_ONCE);
        break;

    case HAPTIC_IDLE:
    default:
        __haptic_output(false);
        break;
    }
}

OPERATE_RET talis_haptic_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    if (sg_inited) {
        return OPRT_OK;
    }

#if TALIS_BOARD_ENABLE_SDCARD
    /* 编译期就能发现的冲突，直接喊出来 */
#warning "TALIS_BOARD_ENABLE_SDCARD=1 会把 GPIO_19 复用成 SDIO_D3，震动马达将失效"
    PR_ERR("HAPTIC PIN %d conflicts with SDIO_D3 (SD card enabled). Motor will NOT work.",
           TALIS_BOARD_HAPTIC_PIN);
#endif

    /* 显式把引脚设回普通 GPIO 功能。
     * 当前 SD 卡关闭时这一步其实是冗余的，但留着可以防御两种情况：
     *   1) 日后有人打开 SD 卡或别的外设抢了这根脚
     *   2) 平台上电默认复用状态不确定
     * 代价只有一次寄存器写。 */
    rt = tkl_io_pinmux_config(TALIS_BOARD_HAPTIC_PIN, TUYA_GPIO);
    if (rt != OPRT_OK) {
        PR_WARN("haptic pinmux to GPIO returned %d (continue anyway)", rt);
    }

    TUYA_GPIO_BASE_CFG_T cfg = {
        .mode   = TUYA_GPIO_PUSH_PULL,
        .direct = TUYA_GPIO_OUTPUT,
        .level  = TUYA_GPIO_LEVEL_LOW,
    };
    TUYA_CALL_ERR_RETURN(tkl_gpio_init(TALIS_BOARD_HAPTIC_PIN, &cfg));
    __haptic_output(false);

    TUYA_CALL_ERR_RETURN(tal_sw_timer_create(__haptic_timer_cb, NULL, &sg_timer));

    sg_phase  = HAPTIC_IDLE;
    sg_inited = true;

    PR_NOTICE("haptic ready on GPIO_%d (%d pulses, %dms on / %dms off)",
              TALIS_BOARD_HAPTIC_PIN, TALIS_HAPTIC_PULSES,
              TALIS_HAPTIC_ON_MS, TALIS_HAPTIC_OFF_MS);

    return OPRT_OK;
}

void talis_haptic_trigger(void)
{
    if (!sg_inited) {
        PR_WARN("haptic not init");
        return;
    }

    /* 重复触发时重新开始，不叠加 */
    tal_sw_timer_stop(sg_timer);

    sg_pulse_cnt = 0;
    sg_phase     = HAPTIC_ON;
    __haptic_output(true);
    tal_sw_timer_start(sg_timer, TALIS_HAPTIC_ON_MS, TAL_TIMER_ONCE);
}

void talis_haptic_stop(void)
{
    if (!sg_inited) {
        return;
    }

    tal_sw_timer_stop(sg_timer);
    sg_phase = HAPTIC_IDLE;
    __haptic_output(false);
}

bool talis_haptic_is_running(void)
{
    return (sg_phase != HAPTIC_IDLE);
}
