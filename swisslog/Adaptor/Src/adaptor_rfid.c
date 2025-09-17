#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_rfid.h"
#include "LogDebugInfo.h"

// 外部声明
extern UART_HandleTypeDef huart5;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;


void vRfid_Start_GPDMA_Receive(uint8_t* ucRfid_Rx_Buffer) {
  // 启动DMA接收
  HAL_UARTEx_ReceiveToIdle_DMA(&huart5, ucRfid_Rx_Buffer, RFID_RX_BUF_SIZE);
  __HAL_DMA_DISABLE_IT(huart5.hdmarx, DMA_IT_HT);
}


