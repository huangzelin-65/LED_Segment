#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_wifi.h"
#include "LogDebugInfo.h"


// 双缓冲区（防止处理期间数据被覆盖）
uint8_t ucWifi_Rx_Buffer[2][WIFI_RX_BUF_SIZE];
uint8_t ucWifi_current_buf_idx = 0;  // 当前使用的缓冲区索引

/* AT指令缓冲区与状态定义 */
uint8_t at_cmd_buf[WIFI_TX_BUF_SIZE];
uint8_t at_resp_buf[WIFI_RX_BUF_SIZE];
uint16_t resp_length = 0;

Wifi_Rx_Frame_t xWifi_Rx_Frame;

// 外部声明
extern UART_HandleTypeDef huart6;
extern osMessageQueueId_t xWifi_Rx_QueueHandle;
extern osSemaphoreId_t xWifiTxSemHandle;


// 启动WIFI的GPDMA接收
void vWifi_Start_GPDMA_Receive(void) {
  // 启动DMA接收（空闲模式）
  if (HAL_UARTEx_ReceiveToIdle_DMA(&huart6, ucWifi_Rx_Buffer[ucWifi_current_buf_idx], WIFI_RX_BUF_SIZE) != HAL_OK) {
    Error_Handler();
  }
}


// 停止WIFI的GPDMA接收
void vWifi_Stop_GPDMA_Receive(void) {
  HAL_UART_DMAStop(&huart6);
}


// WIFI的GPDMA接收处理，在stm32h5xx_it.c中调用
void vWifi_RxEventCallback(uint16_t Size)
{
  HAL_UART_DMAStop(&huart6);           // 停止当前DMA传输

  if (Size > 0) 
  {
    Wifi_Rx_Frame_t xWifi_Rx_Frame;
    memcpy(xWifi_Rx_Frame.data, ucWifi_Rx_Buffer[ucWifi_current_buf_idx], Size);
    xWifi_Rx_Frame.len = Size;
    osMessageQueuePut(xWifi_Rx_QueueHandle, &xWifi_Rx_Frame, 0, 0);
  }

  // 切换缓冲区并重启接收
  ucWifi_current_buf_idx ^= 1;
  vWifi_Start_GPDMA_Receive();
}

// 发送数据到WIFI
void vSendToWifiTX(uint8_t *ucCmdDataArr, uint8_t len)
{
  if (ucCmdDataArr == NULL || len == 0)
  {
    DEBUGINFO_ALL("Invalid TX parameters");
    return;
  }

  // 获取TX发送锁
  if (osSemaphoreAcquire(xWifiTxSemHandle, osWaitForever) == osOK)
  {
    taskENTER_CRITICAL();                              // 进入临界区
    HAL_UART_Transmit_DMA(&huart6, ucCmdDataArr, len); // 启动DMA发送
    // HAL_UART_Transmit_IT(&huart2, CmdDataArr, len); // 启动中断发送
    taskEXIT_CRITICAL(); // 退出临界区

    DEBUGINFO_ALL("wifi send:%s,len:%d\r\n", ucCmdDataArr, len);
  }
}

/* 发送AT指令并等待响应 */
HAL_StatusTypeDef at_send_command(const char *cmd, const char *expect, uint32_t timeout_ms)
{
  uint32_t start_tick = osKernelGetTickCount();
  uint32_t rsp_timeout = 100;  // 每次等待回复的超时时间（ms）
  HAL_StatusTypeDef ret = HAL_ERROR;
  Wifi_Rx_Frame_t frame;
  
  //启动DMA接收
  vWifi_Start_GPDMA_Receive();
  
  // 格式化并发送AT指令
  snprintf((char *)at_cmd_buf, WIFI_TX_BUF_SIZE, "%s\r\n", cmd);
  vSendToWifiTX(at_cmd_buf, strlen((char *)at_cmd_buf));

  
  // 超时时间内循环等待响应
  while ((osKernelGetTickCount() - start_tick) < timeout_ms) 
  {
    
    // 等待响应, 检查信号量
    if (osMessageQueueGet(xWifi_Rx_QueueHandle, &frame, NULL, rsp_timeout) == osOK) 
    {
      DEBUGINFO_ALL("wifi received:%s\r\n",at_resp_buf);
      // 检查是否收到预期回复
      ret = (strstr((char *)frame.data, expect) != NULL) ? HAL_OK : HAL_ERROR;
      if(ret == HAL_OK)
      {
        break;
      }
      else
      {
        //启动DMA接收
        vWifi_Start_GPDMA_Receive();
      }
    }
  }
  
  //停止DMA获取
  vWifi_Stop_GPDMA_Receive();
  return ret;
}


/* 连接到指定AP */
HAL_StatusTypeDef wb502a_connect_ap(void) 
{
  char conn_cmd[WIFI_TX_BUF_SIZE];
  snprintf(conn_cmd, WIFI_TX_BUF_SIZE, "AT+RAP=%s,%s",WIFI_SSID, WIFI_PASSWORD);
  // 连接WiFi可能需要较长时间，设置15秒超时
  return at_send_command(conn_cmd, "+EVENT:WIFI_CONNECT", 30000);
}

/* 确认静态IP地址 */
HAL_StatusTypeDef wb502a_check_ip(const char *ip_addr) 
{
  char expect_str[WIFI_TX_BUF_SIZE];
  snprintf(expect_str, WIFI_TX_BUF_SIZE, "\r\n+LIP=%s",ip_addr);
  return at_send_command("AT+LIP", expect_str, 2000);
}
