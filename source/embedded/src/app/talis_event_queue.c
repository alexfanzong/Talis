/**
 * @file talis_event_queue.c
 * @brief 跨任务事件队列实现
 */

#include "tal_api.h"
#include "talis_event_queue.h"

#define TALIS_EVENT_QUEUE_DEPTH   16

static QUEUE_HANDLE sg_evt_queue = NULL;

OPERATE_RET talis_event_queue_init(void)
{
    OPERATE_RET rt = OPRT_OK;

    if (sg_evt_queue) {
        return OPRT_OK;
    }

    TUYA_CALL_ERR_RETURN(
        tal_queue_create_init(&sg_evt_queue, sizeof(TalisEventMsg_t), TALIS_EVENT_QUEUE_DEPTH));

    return OPRT_OK;
}

OPERATE_RET talis_event_post_param(TalisEvent_t evt, uint32_t param)
{
    TalisEventMsg_t msg = {
        .evt   = evt,
        .param = param,
    };

    if (NULL == sg_evt_queue) {
        PR_ERR("event queue not init, drop evt %d", evt);
        return OPRT_RESOURCE_NOT_READY;
    }

    /* timeout=0：队列满时直接丢弃，绝不阻塞调用方。
     * 调用方可能是 BLE 协议栈回调，阻塞它比丢一个事件严重得多。 */
    return tal_queue_post(sg_evt_queue, &msg, 0);
}

OPERATE_RET talis_event_post(TalisEvent_t evt)
{
    return talis_event_post_param(evt, 0);
}

OPERATE_RET talis_event_fetch(TalisEventMsg_t *msg, uint32_t timeout_ms)
{
    if (NULL == sg_evt_queue || NULL == msg) {
        return OPRT_INVALID_PARM;
    }

    return tal_queue_fetch(sg_evt_queue, msg, timeout_ms);
}
