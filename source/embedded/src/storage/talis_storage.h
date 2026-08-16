/**
 * @file talis_storage.h
 * @brief 设备包存储
 *
 * v0.3 的模型：Android 生成完整 Action Pack，压缩出 ≤16KB 的 devicePack
 * 同步给 Talis。本轮 Demo 不做真实 BLE 同步（那是 Phase 2A），
 * 设备包直接编译进固件。
 */

#ifndef __TALIS_STORAGE_H__
#define __TALIS_STORAGE_H__

#include "talis_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化存储并选定当前生效的设备包
 *
 * 目前直接指向固件内置的 Paris 包。日后接 Phase 2A 的 BLE 同步时，
 * 这里改成「先读 Flash 里上一份校验通过的包，没有再回落到内置包」。
 */
OPERATE_RET talis_storage_init(void);

/**
 * @brief 取当前生效的设备包（只读，永不返回 NULL）
 */
const talis_device_pack_t *talis_storage_get_pack(void);

#ifdef __cplusplus
}
#endif

#endif /* __TALIS_STORAGE_H__ */
