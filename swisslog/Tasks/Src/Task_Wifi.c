/*
 * Task_Wifi.c
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Wifi.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
#include "common.h"


uint8_t ucWifiDataLen = 0;

// 双缓冲区（防止处理期间数据被覆盖）
uint8_t ucWifi_Receive_Buffer[2][WIFI_RX_BUF_SIZE];
uint8_t ucWifi_current_buf_idx = 0;  // 当前使用的缓冲区索引

/* 外部资源声明 */
extern osSemaphoreId_t WifiRxSemHandle;
extern osSemaphoreId_t xWifiReadySemHandle;
extern PlcToCarData PlcToCarData_obj;
extern osMessageQueueId_t xWifi_Rx_QueueHandle;
extern osSemaphoreId_t xWifiRxSemHandle;


/* 初始化WB502A模块 */
static HAL_StatusTypeDef wb502a_init(void) 
{
  char wifi_cmd[WIFI_TX_BUF_SIZE];

  DEBUGINFO("wifi init\r\n");

  //等待模块启动
  osDelay(1000);

  //退出透传模式步骤1
  at_send_command("+++", "a", 500);
  //退出透传模式步骤2
  at_send_command("a", "+ok", 500);

  // 退出透传后需要时间保存参数到flash
  osDelay(1000);

  // //查看透传模式
  // if (at_send_command("AT+TPMODE", "OK", 10000) != HAL_OK) 
  // {
  //   return HAL_ERROR;
  // }

  //复位模块
  if (at_send_command("AT+RESET", "OK", 10000) != HAL_OK) 
  {
    return HAL_ERROR;
  }

  // 模块复位后需要一定时间启动
  osDelay(2000);



  // 检查模块是否响应
  if (at_send_command("AT", "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  //设置模式为STA模式
  if (at_send_command("AT+ROLE=1", "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  // 设置静态IP
  snprintf(wifi_cmd, WIFI_TX_BUF_SIZE, "AT+SIP=%s", WIFI_STATIC_IP);
  if (at_send_command(wifi_cmd, "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  // 设置网关
  snprintf(wifi_cmd, WIFI_TX_BUF_SIZE, "AT+GW=%s", WIFI_GATEWAY);
  if (at_send_command(wifi_cmd, "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  // 设置掩码
  snprintf(wifi_cmd, WIFI_TX_BUF_SIZE, "AT+MASK=%s", WIFI_NETMASK);
  if (at_send_command(wifi_cmd, "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  // 关闭DHCP
  if (at_send_command("AT+DHCP=0", "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  // 连接到指定AP
  if (wb502a_connect_ap() != HAL_OK) 
  {
    return HAL_ERROR;
  }


  // 确认静态IP地址
  if (wb502a_check_ip(WIFI_STATIC_IP) != HAL_OK) 
  {
    return HAL_ERROR;
  }

  
  //设置客户端socket
  snprintf(wifi_cmd, WIFI_TX_BUF_SIZE, "AT+SOCKET=1,%s,%s", WIFI_SERVER_IP, WIFI_SERVER_PORT);
  if (at_send_command(wifi_cmd, "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }

  //进入透传模式
  if (at_send_command("AT+TPMODE=1", "OK", 2000) != HAL_OK) 
  {
    return HAL_ERROR;
  }


  return HAL_OK;
}



/* WiFi管理任务入口函数 */
void vWifiManagerTask(void *argument)
{
  DEBUGINFO("vWifiManagerTask\r\n");
  
  // 初始化WiFi模块
  while (wb502a_init() != HAL_OK) 
  {
     // 初始化失败重试
       osDelay(1000);
       DEBUGINFO("wifi retry!\r\n");
  }
  
  
  osSemaphoreRelease(xWifiReadySemHandle);  // 释放信号量启动WifiReceive任务

  // 连接成功后进入状态监测
  while (1)
  {
	  osDelay(2000);
  }
}
    

/* wifi接收任务入口函数 */
void vWifiReceiveTask(void *argument)
{
  uint32_t ulReceiveLen = 0;
  
  if (osSemaphoreAcquire(xWifiReadySemHandle, osWaitForever) == osOK)
  {
    //启动DMA接收
    vWifi_Start_DMA_Receive(ucWifi_Receive_Buffer[ucWifi_current_buf_idx]);

    while(1) {
      // 等待DMA接收完成信号
      //if (osMessageQueueGet(xWifi_Rx_QueueHandle,ucWifi_Receive_Buffer, NULL, osWaitForever) == osOK)
      if (osSemaphoreAcquire(xWifiRxSemHandle, osWaitForever) == osOK)
      {

        ulReceiveLen = ulWifi_Get_DMA_Receive_Len();
        DEBUGINFO("wifi received:%s, len:%d\r\n",ucWifi_Receive_Buffer[ucWifi_current_buf_idx], ulReceiveLen);

        vSendToWifiTX(ucWifi_Receive_Buffer[ucWifi_current_buf_idx], ulReceiveLen);

        // PlcToCarData_obj.wSeq = ucWifi_Receive_Buffer[1]<<8 | ucWifi_Receive_Buffer[0]; // 序号
        // PlcToCarData_obj.dwPlcNum = ucWifi_Receive_Buffer[5]<<24 | ucWifi_Receive_Buffer[4]<<16 | ucWifi_Receive_Buffer[3]<<8 | ucWifi_Receive_Buffer[2]; // PLC编号
        // PlcToCarData_obj.wHeatBeat = ucWifi_Receive_Buffer[CMD_BASE_COUNT+1]<<8 | ucWifi_Receive_Buffer[CMD_BASE_COUNT]; // 心跳信号
        // PlcToCarData_obj.wAlm = ucWifi_Receive_Buffer[CMD_BASE_COUNT+3]<<8 | ucWifi_Receive_Buffer[CMD_BASE_COUNT+2]; // 报警信号
        // PlcToCarData_obj.wCtrl = ucWifi_Receive_Buffer[CMD_BASE_COUNT+5]<<8 | ucWifi_Receive_Buffer[CMD_BASE_COUNT+4]; // 控制信号
        // PlcToCarData_obj.bDire = ucWifi_Receive_Buffer[CMD_BASE_COUNT+30]; // 小车运行方向 1=正转 2=反转

        PlcToCarData_obj.wCtrl = ucWifi_Receive_Buffer[ucWifi_current_buf_idx][1]<<8 \
                                | ucWifi_Receive_Buffer[ucWifi_current_buf_idx][0]; // 控制信号
        PlcToCarData_obj.bDire = ucWifi_Receive_Buffer[ucWifi_current_buf_idx][2]; // 小车运行方向 1=正转 2=反转
        DEBUGINFO("wCtrl : %X\r\n",PlcToCarData_obj.wCtrl);

        vParseCommandToCar();

        // 重启DMA接收(DMA循环模式下，重启后从缓冲区起始地址覆盖写入)
        ucWifi_current_buf_idx ^= 1;
        vWifi_Start_DMA_Receive(ucWifi_Receive_Buffer[ucWifi_current_buf_idx]);
      }

    }
  }
}


