/*
 * Ota.h
 *
 *  Created on: Nov 11, 2025
 *      Author: e3lijia25d
 */

#ifndef OTA_INC_OTA_H_
#define OTA_INC_OTA_H_

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "param.h"

// OTA状态枚举
typedef enum {
    OTA_STATE_IDLE = 0,        // 空闲
    OTA_STATE_DOWNLOADING,     // 下载中
    OTA_STATE_DOWNLOAD_OK,     // 下载完成
    OTA_STATE_DOWNLOAD_ERR,    // 下载失败
    OTA_STATE_VERIFYING,       // 校验中
    OTA_STATE_VERIFY_OK,       // 校验通过
    OTA_STATE_VERIFY_ERR,      // 校验失败
    OTA_STATE_UPGRADE_READY,   // 升级就绪
    OTA_STATE_UPGRADE_ERR      // 升级失败
} OTA_StateTypeDef;

// OTA进度结构体
typedef struct {
    //uint32_t total_size;     // OTA包总大小（字节）
    //uint32_t current_size;   // 已下载大小（字节）
    OTA_StateTypeDef state;  // 当前状态
    Partition_TypeDef target_part; // 目标分区
} OTA_ProgressTypeDef;

// 函数声明
void ota_init(void);
OTA_StateTypeDef ota_get_state(void);
void ota_get_progress(OTA_ProgressTypeDef* progress);
//BaseType_t ota_start_download(void);
BaseType_t ota_start_verify(void); 
BaseType_t ota_trigger_upgrade(void);
//Partition_TypeDef ota_get_current_partition(void);


#endif /* OTA_INC_OTA_H_ */
