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
#include "OtaFlash.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"

// 配置参数（适配960KB App分区，增大缓冲区和栈大小）
#define OTA_TASK_STACK_SIZE  8192    // 任务栈大小（960KB分区需更大栈）
#define OTA_TASK_PRIORITY    5       // 任务优先级（高于应用任务）
// #define OTA_BUF_SIZE         8192    // 下载缓冲区大小（提升大文件下载效率）
// #define OTA_SAVE_INTERVAL    128*1024 // 每下载128KB保存一次续传状态（适配大分区）

// 全局变量
//static TaskHandle_t ota_task_handle = NULL;
//static QueueHandle_t ota_cmd_queue = NULL;
//static OTA_ProgressTypeDef ota_progress = {0};
//static Partition_TypeDef ota_current_part = PARTITION_MAX;

//保存升级的APP flash位置
//static Partition_TypeDef ota_target_part = PARTITION_MAX;

//保存运行APP的相关参数，及更新的参数区位置
//static Param_And_Area s_struTargetRunningParam= {0};
//static uint32_t s_OtaCurrentUpgCounter =0;
//保存升级Firmware信息
//static OTA_Upg_FW_TypeDef s_OtaUpgFwParam= {0};

//static OTA_Resume_TypeDef s_OtaResumeParam= {0};

//extern CRC_HandleTypeDef hcrc; // App侧独立CRC句柄

// 命令枚举
/*typedef enum {
    OTA_CMD_NONE = 0,
    OTA_CMD_START_DOWNLOAD,
    OTA_CMD_START_VERIFY, // 启动校验命令
    OTA_CMD_CANCEL
} OTA_CmdTypeDef;*/

// 函数声明
void ota_task(void* arg);
//static BaseType_t ota_cmd_send(OTA_CmdTypeDef cmd, TickType_t timeout);

extern osMessageQueueId_t xMqttManagerQueueHandle;
STRU_Of_Get_UpgPkg_Info_TypeDef ReadyBack;
STRU_Of_Upg_Result_TypeDef upgRlt;

// OTA任务核心逻辑
void ota_task(void* arg) {
    //OTA_CmdTypeDef cmd;

    of_init_target_param();

    //启动DMA接收
    //vTest_Start_DMA_Receive(ucTest_Rx_Buffer);

    char *manage_data = NULL;

    while (1) {
        // 等待命令
    	if(xQueueReceive(xMqttManagerQueueHandle, &manage_data, portMAX_DELAY) == pdTRUE) {
            continue;
        }

        //vTest_Start_DMA_Receive(ucTest_Rx_Buffer);
		uint8_t head = manage_data[0];
		//uint8_t *rlt;
		switch(head)
		{
		case CMD_HEAD_UPG_INFO:
		  memset(&ReadyBack, 0, sizeof(STRU_Of_Get_UpgPkg_Info_TypeDef));
		  of_get_first_packet_number((U_Of_UpgFistPkg_TypeDef*)&manage_data[1], &ReadyBack);
		  //接下来需要把ReadyBack数据发回给服务器
		  //printf("Start Packet:%4ld\r\n", ReadyBack.pkg_No);

		  /*rlt = (uint8_t *)&ReadyBack;
		  for(int i=0; i<sizeof(STRU_Of_Get_UpgPkg_Info_TypeDef); i++)
		  {
			  printf("0x%02x ", *rlt++);
		  }
		  printf("\n");*/
		  break;
		case CMD_HEAD_FRAME:
		  memset(&ReadyBack, 0, sizeof(STRU_Of_Get_UpgPkg_Info_TypeDef));
		  of_get_next_packet_number((U_Of_UpgPkg_TypeDef*)&manage_data[1], &ReadyBack);

		  if(of_get_upgrade_result(&upgRlt) == pdTRUE)
		  {
			  //把升级结果发送给服务器
			  //printf("End Upgrade\r\n");

			  /*rlt = (uint8_t *)&upgRlt;
			  for(int i=0; i<sizeof(STRU_Of_Upg_Result_TypeDef); i++)
			  {
				  printf("0x%02x ", *rlt++);
			  }
			  printf("\n");*/
			  //启动延迟，3S后系统重启
			  //HAL_NVIC_SystemReset();
		  }
		  else
		  {
			  //接下来需要把ReadyBack数据发回给服务器
			  //printf("Start Packet:%4ld\r\n", ReadyBack.pkg_No);

			  /*rlt = (uint8_t *)&ReadyBack;
			  for(int i=0; i<sizeof(STRU_Of_Get_UpgPkg_Info_TypeDef); i++)
			  {
				  printf("0x%02x ", *rlt++);
			  }
			  printf("\n");*/
		  }
		  break;
		default:
		  break;
		}
    }
}
/*
// 发送OTA命令
static BaseType_t ota_cmd_send(OTA_CmdTypeDef cmd, TickType_t timeout) {
    if (ota_cmd_queue == NULL) {
        return pdFALSE;
    }
    return xQueueSend(ota_cmd_queue, &cmd, timeout);
}
*/
