/**
 * @file talis_ble_protocol.h
 * @brief Talis 应用层 BLE 帧协议
 *
 * ============================================================================
 * 为什么是应用层帧协议而不是自定义 GATT 服务
 * ============================================================================
 * TuyaOpen 的 tal_bluetooth API（src/tal_bluetooth/include/tal_bluetooth.h）
 * 没有提供注册自定义 GATT service / characteristic 的接口，只暴露一套固定的
 * Tuya 服务：
 *   Service UUID : 0xFD50   (TAL_BLE_CMD_SERVICE_UUID_V2)
 *   Write  Char  : 0x0001   (App -> 设备，经 TAL_BLE_EVT_WRITE_REQ 上报)
 *   Notify       : tal_ble_server_common_send()
 *
 * 因此 Talis 的指令封装在特征的 payload 里。
 *
 * ============================================================================
 * 帧格式
 * ============================================================================
 *   偏移  长度  字段     说明
 *   0     1     MAGIC    固定 0x54 ('T')
 *   1     1     CMD      指令码
 *   2     1     SEQ      序号，应答时原样回填
 *   3     1     LEN      payload 长度 (0 ~ 16)
 *   4     LEN   PAYLOAD
 */

#ifndef __TALIS_BLE_PROTOCOL_H__
#define __TALIS_BLE_PROTOCOL_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TALIS_FRAME_MAGIC     0x54
#define TALIS_FRAME_HDR_LEN   4
#define TALIS_PAYLOAD_MAX     16
#define TALIS_FRAME_MAX_LEN   (TALIS_FRAME_HDR_LEN + TALIS_PAYLOAD_MAX)

typedef enum {
    CMD_RING_PHONE = 0x01,  /* 设备 -> App：请求手机响铃 */
    CMD_HELLO      = 0x02,  /* 设备 -> App：连接握手 */
    CMD_RING_ACK   = 0x81,  /* App -> 设备：响铃应答，payload[0] 0=成功 1=失败 */
    CMD_HELLO_ACK  = 0x82,  /* App -> 设备：握手应答 */
} TalisBleCmd_t;

#define TALIS_RING_ACK_OK     0x00
#define TALIS_RING_ACK_FAIL   0x01

typedef struct {
    uint8_t cmd;
    uint8_t seq;
    uint8_t len;
    uint8_t payload[TALIS_PAYLOAD_MAX];
} TalisFrame_t;

/**
 * @brief 打包一帧
 *
 * @param[out] buf      输出缓冲，至少 TALIS_FRAME_MAX_LEN 字节
 * @param[in]  buf_size 缓冲大小
 * @param[in]  cmd      指令码
 * @param[in]  seq      序号
 * @param[in]  payload  可为 NULL
 * @param[in]  len      payload 长度
 * @return >0 实际帧长；<0 出错
 */
int talis_frame_pack(uint8_t *buf, uint16_t buf_size, uint8_t cmd, uint8_t seq,
                     const uint8_t *payload, uint8_t len);

/**
 * @brief 解析一帧
 *
 * @param[in]  data  原始数据
 * @param[in]  len   数据长度
 * @param[out] out   解析结果
 * @return OPRT_OK 成功；其它表示不是合法 Talis 帧
 */
OPERATE_RET talis_frame_parse(const uint8_t *data, uint16_t len, TalisFrame_t *out);

/**
 * @brief 指令码转可读名字（日志用）
 */
const char *talis_cmd_name(uint8_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_BLE_PROTOCOL_H__ */
