#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_rfid.h"
#include "LogDebugInfo.h"


// 双缓冲区（防止处理期间数据被覆盖）
uint8_t ucRfid_Rx_Buffer[2][RFID_RX_BUF_SIZE];
uint8_t ucRfid_current_buf_idx = 0;  // 当前使用的缓冲区索引

// 外部声明
extern UART_HandleTypeDef huart5;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;
extern osSemaphoreId_t xWifiTxSemHandle;


// 启动Rfid的GPDMA接收
void vRfid_Start_GPDMA_Receive(void) {
  // 启动DMA接收（中断模式）
  if (HAL_UARTEx_ReceiveToIdle_DMA(&huart5, ucRfid_Rx_Buffer[ucRfid_current_buf_idx], RFID_RX_BUF_SIZE) != HAL_OK) {
    Error_Handler();
  }
}

// Rfid的GPDMA接收处理，在stm32h5xx_it.c中调用
void vRfid_RxEventCallback(uint16_t Size)
{
  HAL_UART_DMAStop(&huart5);           // 停止当前DMA传输

  if (Size > 0) 
  {
    Rfid_Rx_Frame_t xRfid_Rx_frame;
    memcpy(xRfid_Rx_frame.data, ucRfid_Rx_Buffer[ucRfid_current_buf_idx], Size);
    xRfid_Rx_frame.len = Size;
    osMessageQueuePut(xRfid_Rx_QueueHandle, &xRfid_Rx_frame, 0, 0);
  }
        
  // 切换缓冲区并重启接收
  ucRfid_current_buf_idx ^= 1;
  vRfid_Start_GPDMA_Receive();
}


