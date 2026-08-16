/**
 * @file talis_storage.c
 * @brief 设备包存储实现
 */

#include "tal_api.h"
#include "talis_app_config.h"
#include "talis_storage.h"

/* 定义在 talis_pack_paris.c */
extern const talis_device_pack_t g_pack_paris;

static const talis_device_pack_t *sg_pack = NULL;

OPERATE_RET talis_storage_init(void)
{
    if (sg_pack) {
        return OPRT_OK;
    }

    /* 本轮 Demo：直接用固件内置的 Paris 包。
     *
     * 接 Phase 2A（真实 BLE 同步）时，这里改成：
     *   1. 从 Flash 读上一份校验通过的 devicePack
     *   2. 校验 schemaVersion 与 checksum
     *   3. 通过则用它，否则回落到内置包
     * 按 v0.3 §10「同步失败不得显示成功、校验错误时保留旧包」的要求，
     * 回落必须是静默且可用的，绝不能让设备变成一块砖。 */
    sg_pack = &g_pack_paris;

    PR_NOTICE("device pack: %s, %d modules", sg_pack->city, sg_pack->module_count);

    return OPRT_OK;
}

const talis_device_pack_t *talis_storage_get_pack(void)
{
    /* 任何时候都要给出一份能用的数据，UI 不做 NULL 判断 */
    return sg_pack ? sg_pack : &g_pack_paris;
}
