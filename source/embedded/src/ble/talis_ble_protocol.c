/**
 * @file talis_ble_protocol.c
 * @brief Talis BLE 帧协议编解码
 */

#include <string.h>
#include "tal_api.h"
#include "talis_ble_protocol.h"

int talis_frame_pack(uint8_t *buf, uint16_t buf_size, uint8_t cmd, uint8_t seq,
                     const uint8_t *payload, uint8_t len)
{
    if (NULL == buf) {
        return -1;
    }

    if (len > TALIS_PAYLOAD_MAX) {
        PR_ERR("payload too long: %d > %d", len, TALIS_PAYLOAD_MAX);
        return -1;
    }

    if (buf_size < (uint16_t)(TALIS_FRAME_HDR_LEN + len)) {
        PR_ERR("frame buffer too small: %d < %d", buf_size, TALIS_FRAME_HDR_LEN + len);
        return -1;
    }

    buf[0] = TALIS_FRAME_MAGIC;
    buf[1] = cmd;
    buf[2] = seq;
    buf[3] = len;

    if (len > 0 && payload) {
        memcpy(&buf[TALIS_FRAME_HDR_LEN], payload, len);
    }

    return TALIS_FRAME_HDR_LEN + len;
}

OPERATE_RET talis_frame_parse(const uint8_t *data, uint16_t len, TalisFrame_t *out)
{
    if (NULL == data || NULL == out) {
        return OPRT_INVALID_PARM;
    }

    if (len < TALIS_FRAME_HDR_LEN) {
        PR_DEBUG("frame too short: %d", len);
        return OPRT_INVALID_PARM;
    }

    if (data[0] != TALIS_FRAME_MAGIC) {
        PR_DEBUG("bad magic: 0x%02X", data[0]);
        return OPRT_INVALID_PARM;
    }

    uint8_t payload_len = data[3];

    if (payload_len > TALIS_PAYLOAD_MAX) {
        PR_ERR("payload len out of range: %d", payload_len);
        return OPRT_INVALID_PARM;
    }

    /* 声明长度必须与实际收到的一致，短了说明帧被截断 */
    if (len < (uint16_t)(TALIS_FRAME_HDR_LEN + payload_len)) {
        PR_ERR("truncated frame: got %d, need %d", len, TALIS_FRAME_HDR_LEN + payload_len);
        return OPRT_INVALID_PARM;
    }

    memset(out, 0, sizeof(TalisFrame_t));
    out->cmd = data[1];
    out->seq = data[2];
    out->len = payload_len;

    if (payload_len > 0) {
        memcpy(out->payload, &data[TALIS_FRAME_HDR_LEN], payload_len);
    }

    return OPRT_OK;
}

const char *talis_cmd_name(uint8_t cmd)
{
    switch (cmd) {
    case CMD_RING_PHONE: return "RING_PHONE";
    case CMD_HELLO:      return "HELLO";
    case CMD_RING_ACK:   return "RING_ACK";
    case CMD_HELLO_ACK:  return "HELLO_ACK";
    default:             return "UNKNOWN";
    }
}
