#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_rfid.h"
#include "LogDebugInfo.h"

// *************************小车底盘RFID****************************
// 外部声明
extern UART_HandleTypeDef huart5;


void vCarRfid_Start_DMA_Receive(uint8_t* ucCarRfid_Rx_Buffer) {
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart5, ucCarRfid_Rx_Buffer, CAR_RFID_RX_BUF_SIZE)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, ucCarRfid_Rx_Buffer, CAR_RFID_RX_BUF_SIZE);
  }
  __HAL_DMA_DISABLE_IT(huart5.hdmarx, DMA_IT_HT);
}

uint32_t ulCarRfid_Get_DMA_Receive_Len(void)
{
  uint32_t ulLen = CAR_RFID_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart5.hdmarx);
  return ulLen;
}

// *************************小车车厢RFID****************************
// 外部声明
extern UART_HandleTypeDef huart8;

void vSendToBoxRfid(uint8_t* ucBoxRfidSendBuffer, uint32_t ucLen)
{
  taskENTER_CRITICAL(); // 进入临界区
  HAL_UART_Transmit_IT(&huart8, ucBoxRfidSendBuffer, ucLen); // 启动中断发送
  taskEXIT_CRITICAL(); // 退出临界区
  DEBUGINFO("ucBoxRfidSendBuffer:\r\n");
  for(uint8_t i=0;i<ucLen;i++)
  {
    safe_printf("%X ",ucBoxRfidSendBuffer[i]);
  }
  safe_printf("\r\n");

}

void vBoxRfid_Start_DMA_Receive(uint8_t* ucBoxRfid_Rx_Buffer) {
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart8, ucBoxRfid_Rx_Buffer, BOX_RFID_RX_BUF_SIZE)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart8, ucBoxRfid_Rx_Buffer, BOX_RFID_RX_BUF_SIZE);
  }
  __HAL_DMA_DISABLE_IT(huart8.hdmarx, DMA_IT_HT);
}

uint32_t ulBoxRfid_Get_DMA_Receive_Len(void)
{
  uint32_t ulLen = BOX_RFID_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart8.hdmarx);
  return ulLen;
}
