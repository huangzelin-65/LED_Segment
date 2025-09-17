#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_test.h"
#include "LogDebugInfo.h"

// 双缓冲区（防止处理期间数据被覆盖）
uint8_t ucTest_Rx_Buffer[TEST_RX_BUF_SIZE];

// 外部声明
extern osSemaphoreId_t xPrintSemHandle;
extern UART_HandleTypeDef huart1;
extern osMessageQueueId_t xTest_Rx_QueueHandle;

void vPrint_start_Transmit(uint8_t *rxData, uint16_t Size)
{
  // 获取TX发送锁
  //if (osSemaphoreAcquire(xPrintSemHandle, osWaitForever) == osOK) 
  if (osSemaphoreAcquire(xPrintSemHandle, pdMS_TO_TICKS(100)) == osOK) 
  {
    HAL_UART_Transmit_DMA(&huart1, rxData, Size);    // 启动DMA发送
  }
  // HAL_UART_Transmit(&huart1, rxData, Size, HAL_MAX_DELAY);
  // vPortFree(rxData);
}


// 启动Test串口的GPDMA接收
void vTest_Start_GPDMA_Receive(void) {
  // 启动DMA接收（空闲模式）
  HAL_UARTEx_ReceiveToIdle_DMA(&huart1, ucTest_Rx_Buffer, TEST_RX_BUF_SIZE);
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}


// Test串口接收处理，在stm32h5xx_it.c中调用
void vTest_RxEventCallback(uint16_t dataLength)
{
  //HAL_UART_DMAStop(&huart1);           // 停止当前DMA传输

  if (dataLength > 0) 
  {
    osMessageQueuePut(xTest_Rx_QueueHandle, ucTest_Rx_Buffer, 0, 0);
  }

        
  // 重启接收
  vTest_Start_GPDMA_Receive();
}
