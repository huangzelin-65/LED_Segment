#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_wifi.h"
#include "LogDebugInfo.h"


/* AT指令缓冲区与状态定义 */
uint8_t ucAt_Cmd_Buffer[WIFI_TX_BUF_SIZE];
uint8_t ucAt_Respond_Buffer[WIFI_RX_BUF_SIZE];
uint8_t ucWifi_Respond_Buffer[WIFI_RX_BUF_SIZE];
uint16_t resp_length = 0;


// 外部声明
extern UART_HandleTypeDef huart6;
extern osMessageQueueId_t xWifi_Rx_QueueHandle;
extern osSemaphoreId_t xWifiTxSemHandle;
extern osSemaphoreId_t xWifiRxSemHandle;


/**************************************************
 * 启动WiFi的GPDMA接收功能（空闲模式）。
 *
 *  @param ucWifi_Rx_Buffer WiFi接收缓冲区指针
 *  @note 如果启动DMA接收失败，将会重试一次。
 *************************************************/
void vWifi_Start_DMA_Receive(uint8_t *ucWifi_Rx_Buffer)
{
  // 启动DMA接收（空闲模式）
  if (HAL_UARTEx_ReceiveToIdle_DMA(&huart6, ucWifi_Rx_Buffer, WIFI_RX_BUF_SIZE) != HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    //若启动DMA接收失败，再启动一次
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, ucWifi_Rx_Buffer, WIFI_RX_BUF_SIZE);
  }
  __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}


/*********************************
 * 停止Wi-Fi GPDMA接收。
 *********************************/
void vWifi_Stop_GPDMA_Receive(void)
{
  HAL_UART_DMAStop(&huart6);
}


/*********************************************************************
 * 向 Wi-Fi 发送数据
 *
 *  @param ucCmdDataArr 要发送的数据数组
 *  @param len 数据数组的长度
 *
 *  @note 函数使用 osSemaphoreAcquire 获取 TX 发送锁，确保数据发送的原子性
 **********************************************************************/
void vSendToWifiTX(uint8_t *ucCmdDataArr, uint8_t len)
{
  if (ucCmdDataArr == NULL || len == 0)
  {
    DEBUGINFO("Invalid TX parameters");
    return;
  }

  // 获取TX发送锁
  if (osSemaphoreAcquire(xWifiTxSemHandle, osWaitForever) == osOK)
  {
    taskENTER_CRITICAL();                              // 进入临界区
    HAL_UART_Transmit_DMA(&huart6, ucCmdDataArr, len); // 启动DMA发送
    // HAL_UART_Transmit_IT(&huart2, CmdDataArr, len); // 启动中断发送
    taskEXIT_CRITICAL(); // 退出临界区

    DEBUGINFO("wifi send:%s", ucCmdDataArr);
  }
}

/*****************************************************************
 * 发送一个AT命令到WiFi模块，并等待指定的时间内收到预期的响应。
 *
 *   @param cmd        要发送的AT命令。
 *   @param expect     预期的响应字符串。
 *   @param timeout_ms 等待响应的超时时间（毫秒）。
 *
 *   @return 如果收到预期的响应，返回HAL_OK；否则返回HAL_ERROR。
 ******************************************************************/
HAL_StatusTypeDef at_send_command(const char *cmd, const char *expect, uint32_t timeout_ms)
{
  uint32_t start_tick = osKernelGetTickCount();
  uint32_t rsp_timeout = 100;  // 每次等待回复的超时时间（ms）
  HAL_StatusTypeDef ret = HAL_ERROR;
  //Wifi_Rx_Frame_t frame;
  
  //启动DMA接收
  vWifi_Start_DMA_Receive(ucAt_Respond_Buffer);
  
  // 格式化并发送AT指令
  snprintf((char *)ucAt_Cmd_Buffer, WIFI_TX_BUF_SIZE, "%s\r\n", cmd);
  vSendToWifiTX(ucAt_Cmd_Buffer, strlen((char *)ucAt_Cmd_Buffer));

  
  // 超时时间内循环等待响应
  while ((osKernelGetTickCount() - start_tick) < timeout_ms) 
  {
    
    // 等待响应, 检查信号量
    if (osSemaphoreAcquire(xWifiRxSemHandle, rsp_timeout) == osOK)
    {
      DEBUGINFO("wifi received:%s\r\n",ucAt_Respond_Buffer);
      // 检查收到回复是否包含预期字段
      ret = (strstr((char *)ucAt_Respond_Buffer, expect) != NULL) ? HAL_OK : HAL_ERROR;
      if(ret == HAL_OK)
      {
        break;
      }
      else
      {
        //启动DMA接收
        vWifi_Start_DMA_Receive(ucAt_Respond_Buffer);
      }
    }
  }
  
  //停止DMA获取
  vWifi_Stop_GPDMA_Receive();
  //vWifi_Start_DMA_Receive(ucAt_Respond_Buffer);
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
