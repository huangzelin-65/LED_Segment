/*
 * Ota.c
 *
 *  Created on: Nov 11, 2025
 *      Author: e3lijia25d
 */

#include "Ota.h"
#include "Flash.h"
#include "string.h"
#include "stm32h5xx_hal.h"
#include "Calculate.h"

// 配置参数（适配960KB App分区，增大缓冲区和栈大小）
#define OTA_TASK_STACK_SIZE  8192    // 任务栈大小（960KB分区需更大栈）
#define OTA_TASK_PRIORITY    5       // 任务优先级（高于应用任务）
// #define OTA_BUF_SIZE         8192    // 下载缓冲区大小（提升大文件下载效率）
// #define OTA_SAVE_INTERVAL    128*1024 // 每下载128KB保存一次续传状态（适配大分区）

// 全局变量
static TaskHandle_t ota_task_handle = NULL;
static QueueHandle_t ota_cmd_queue = NULL;
static OTA_ProgressTypeDef ota_progress = {0};
//static Partition_TypeDef ota_current_part = PARTITION_MAX;

//保存升级的APP flash位置
static Partition_TypeDef ota_target_part = PARTITION_MAX;

//保存运行APP的相关参数，及更新的参数区位置
//static Param_And_Area s_struTargetRunningParam= {0};
//static uint32_t s_OtaCurrentUpgCounter =0;
//保存升级Firmware信息
//static OTA_Upg_FW_TypeDef s_OtaUpgFwParam= {0};

//static OTA_Resume_TypeDef s_OtaResumeParam= {0};

//extern CRC_HandleTypeDef hcrc; // App侧独立CRC句柄

// 命令枚举
typedef enum {
    OTA_CMD_NONE = 0,
    OTA_CMD_START_DOWNLOAD,
    OTA_CMD_START_VERIFY, // 启动校验命令
    OTA_CMD_CANCEL
} OTA_CmdTypeDef;

// 函数声明
static void ota_task(void* arg);
static BaseType_t ota_cmd_send(OTA_CmdTypeDef cmd, TickType_t timeout);





// OTA初始化（创建任务和队列，初始化CRC）
void ota_init(void) {

    // 创建命令队列
    ota_cmd_queue = xQueueCreate(1, sizeof(OTA_CmdTypeDef));
    if (ota_cmd_queue == NULL) {
        ota_progress.state = OTA_STATE_UPGRADE_ERR;
        return;
    }

    // 获取当前分区和目标分区
    /*ota_current_part = ota_get_current_partition();
    ota_target_part = ota_get_target_partition();
    if (ota_current_part == PARTITION_MAX || ota_target_part == PARTITION_MAX) {
        ota_progress.state = OTA_STATE_UPGRADE_ERR;
        return;
    }*/

    // 初始化进度信息
    ota_progress.target_part = ota_target_part;

    // 创建OTA任务
    /*xTaskCreate(ota_task, "OTA_Task", OTA_TASK_STACK_SIZE, NULL, OTA_TASK_PRIORITY, &ota_task_handle);
    if (ota_task_handle == NULL) {
        ota_progress.state = OTA_STATE_UPGRADE_ERR;
    }*/
}

// 获取当前OTA状态
OTA_StateTypeDef ota_get_state(void) {
    return ota_progress.state;
}

// 获取OTA进度
void ota_get_progress(OTA_ProgressTypeDef* progress) {
    if (progress != NULL) {
        memcpy(progress, &ota_progress, sizeof(OTA_ProgressTypeDef));
    }
}

// // 启动OTA下载
// BaseType_t ota_start_download(void) {
//     if (ota_progress.state == OTA_STATE_DOWNLOADING) {
//         return pdFALSE; // 正在下载中
//     }
//     return ota_cmd_send(OTA_CMD_START_DOWNLOAD, pdMS_TO_TICKS(100));
// }

// 启动OTA校验（替代原下载函数，触发校验目标分区）
BaseType_t ota_start_verify(void) {
    if (ota_progress.state == OTA_STATE_VERIFYING) {
        return pdFALSE; // 正在校验中
    }
    return ota_cmd_send(OTA_CMD_START_VERIFY, pdMS_TO_TICKS(100));
}

// 触发升级（切换分区并重启）
BaseType_t ota_trigger_upgrade(void) {
#if 0
    if (ota_progress.state != OTA_STATE_UPGRADE_READY) {
        return pdFALSE; // 未就绪
    }

    // 更新激活分区到参数区
    Param_Config_TypeDef param = *PARAM_PTR;
    param.active_part = ota_target_part;
    // 同步更新目标分区的CRC（避免Bootloader校验失败）
    uint32_t target_crc = HAL_CRC_Calculate(&hcrc, (uint32_t*)ota_target_part, 
                                           (ota_target_part == PARTITION_APP1) ? APP1_SIZE/4 : APP2_SIZE/4);
    if (ota_target_part == PARTITION_APP1) {
        param.app1_crc = target_crc;
    } else {
        param.app2_crc = target_crc;
    }
    param.self_crc = crc32_calc((uint32_t)&param, sizeof(Param_Config_TypeDef) - 4);
#endif
    /*if (!param_save(&param)) {
        ota_progress.state = OTA_STATE_UPGRADE_ERR;
        return pdFALSE;
    }*/

    // 重启系统
    HAL_NVIC_SystemReset();
    return pdTRUE;
}

// OTA任务核心逻辑
static void ota_task(void* arg) {
    OTA_CmdTypeDef cmd;
    // OTA_Resume_TypeDef resume_info = {0};
    // int32_t sock_fd = -1;
    // uint8_t ota_buf[OTA_BUF_SIZE] = {0};
    // uint32_t server_crc32 = 0;
    // Net_ErrTypeDef net_err;

    while (1) {
        // 等待命令
        if (xQueueReceive(ota_cmd_queue, &cmd, portMAX_DELAY) != pdPASS) {
            continue;
        }

        switch (cmd) {
            /*
            case OTA_CMD_START_DOWNLOAD:
                // 1. 初始化状态
                ota_progress.state = OTA_STATE_DOWNLOADING;
                ota_progress.current_size = 0;

                // 2. 初始化断点续传
                if (!ota_resume_init(&resume_info)) {
                    ota_progress.state = OTA_STATE_DOWNLOAD_ERR;
                    break;
                }
                ota_progress.total_size = resume_info.total_size;
                ota_progress.current_size = resume_info.current_offset;

                // 3. 连接OTA服务器
                net_err = net_connect_server(&sock_fd);
                if (net_err != NET_OK) {
                    ota_progress.state = OTA_STATE_DOWNLOAD_ERR;
                    break;
                }

                // 4. 发送断点续传请求
                net_err = net_send_http_range_req(sock_fd, resume_info.current_offset);
                if (net_err != NET_OK) {
                    net_close_socket(sock_fd);
                    ota_progress.state = OTA_STATE_DOWNLOAD_ERR;
                    break;
                }

                // 5. 循环下载数据（适配960KB大分区）
                while (resume_info.current_offset < resume_info.total_size) {
                    uint32_t recv_len = 0;
                    net_err = net_recv_http_data(sock_fd, ota_buf, OTA_BUF_SIZE, &recv_len);
                    if (net_err != NET_OK || recv_len == 0) {
                        // 网络中断，保存续传状态
                        Param_Config_TypeDef param = *PARAM_PTR;
                        param.resume_info = resume_info;
                        param_save(&param);

                        net_close_socket(sock_fd);
                        ota_progress.state = OTA_STATE_DOWNLOAD_ERR;
                        goto download_exit;
                    }

                    // 6. 写入Flash（目标地址=起始地址+偏移量，支持跨Bank）
                    uint32_t target_addr = (ota_target_part == PARTITION_APP1) ? APP1_START : APP2_START;
                    if (!flash_write_data(target_addr + resume_info.current_offset, ota_buf, recv_len)) {
                        net_close_socket(sock_fd);
                        ota_progress.state = OTA_STATE_DOWNLOAD_ERR;
                        goto download_exit;
                    }

                    // 7. 更新续传状态
                    resume_info.current_offset += recv_len;
                    resume_info.crc32 = HAL_CRC_Calculate(&hcrc, (uint32_t*)target_addr, resume_info.current_offset/4);
                    ota_progress.current_size = resume_info.current_offset;

                    // 8. 定期保存续传状态（128KB一次，减少Flash写入次数）
                    if (resume_info.current_offset % OTA_SAVE_INTERVAL == 0) {
                        Param_Config_TypeDef param = *PARAM_PTR;
                        param.resume_info = resume_info;
                        param_save(&param);
                    }
                }

                // 9. 下载完成，关闭socket
                download_exit:
                net_close_socket(sock_fd);
                if (ota_progress.state != OTA_STATE_DOWNLOADING) {
                    break;
                }
                ota_progress.state = OTA_STATE_DOWNLOAD_OK;

                // 10. 获取服务器CRC32并校验
                net_err = net_get_ota_crc32(&server_crc32);
                if (net_err != NET_OK) {
                    ota_progress.state = OTA_STATE_VERIFY_ERR;
                    break;
                }
                if (!ota_verify_package(server_crc32)) {
                    ota_progress.state = OTA_STATE_VERIFY_ERR;
                    break;
                }
                ota_progress.state = OTA_STATE_VERIFY_OK;

                // 11. 标记升级就绪（清除续传状态）
                Param_Config_TypeDef param = *PARAM_PTR;
                param.resume_info.enable = 0; // 清除续传标记
                param_save(&param);
                ota_progress.state = OTA_STATE_UPGRADE_READY;

                break;*/

            case OTA_CMD_START_VERIFY:
                ota_progress.state = OTA_STATE_VERIFYING;

                // 直接校验目标分区（已通过工具烧录OTA包）
                /*if (ota_verify_target_part()) {
                    ota_progress.state = OTA_STATE_VERIFY_OK;
                    // 自动标记升级就绪
                    ota_progress.state = OTA_STATE_UPGRADE_READY;
                } else {
                    ota_progress.state = OTA_STATE_VERIFY_ERR;
                }*/
                break;

            case OTA_CMD_CANCEL:
                ota_progress.state = OTA_STATE_IDLE;
                break;

            default:
                break;
        }

        // 延迟1秒，避免频繁触发
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 发送OTA命令
static BaseType_t ota_cmd_send(OTA_CmdTypeDef cmd, TickType_t timeout) {
    if (ota_cmd_queue == NULL) {
        return pdFALSE;
    }
    return xQueueSend(ota_cmd_queue, &cmd, timeout);
}

/*****************************************************************************/
//分别获取到可以更新的APP分区(正在运行的反向)，可以更新的param分区

#if 0
// 获取当前App所在分区（通过向量表地址判断）
Partition_TypeDef ota_get_current_partition(void) {
	// SCB->VTOR 存储当前向量表起始地址（STM32H5 V1.5.0 标准用法）
	uint32_t vector_addr = SCB->VTOR;
    if (vector_addr == APP1_START) {
        return PARTITION_APP1;
    } else if (vector_addr == APP2_START) {
        return PARTITION_APP2;
    }
    return PARTITION_MAX;
}

// 获取OTA目标分区（当前分区的另一分区）
static Partition_TypeDef ota_get_target_partition(void) {
    if (ota_current_part == PARTITION_APP1) {
        return PARTITION_APP2;  // App1运行时，目标分区=App2（烧录到0x080F8000）
    } else if (ota_current_part == PARTITION_APP2) {
        return PARTITION_APP1;  // App2运行时，目标分区=App1（烧录到0x08008000）
    }
    return PARTITION_MAX;
}

#endif

//// 错误处理（OTA模块内部）
//static void Error_Handler(void) {
//    __disable_irq();
//    while (1) {
//        // 可添加LED报警逻辑
//    }
//}
