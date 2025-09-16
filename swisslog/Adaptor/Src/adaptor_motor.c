#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_motor.h"
#include "LogDebugInfo.h"


// 双缓冲区（防止处理期间数据被覆盖）
uint8_t ucMotor_Rx_Buffer[2][MOTOR_RX_BUF_SIZE];
uint8_t ucMotor_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern UART_HandleTypeDef huart7;
extern osMessageQueueId_t xMotor_Rx_Queue;
extern osSemaphoreId_t xMotorTxSemHandle;
//extern DMA_HandleTypeDef hdma_usart7_rx;


void vSendToMotor(u8 * CmdDataArr,u8 len)
{
  // 获取RS485发送锁
  if (osSemaphoreAcquire(xMotorTxSemHandle, osWaitForever) == osOK) 
  {
    Motor_RS485_TX_MODE(); // 使能发送模式(在发送完成回调中切回RX模式)

    taskENTER_CRITICAL(); // 进入临界区
    HAL_UART_Transmit_DMA(&huart7, CmdDataArr, len);    // 启动DMA发送
    //HAL_UART_Transmit_IT(&huart7, CmdDataArr, len); // 启动中断发送
    taskEXIT_CRITICAL(); // 退出临界区
  }
}

// 启动Motor的GPDMA接收
void vMotor_Start_GPDMA_Receive(void) {
  // 启动DMA接收（中断模式）
  HAL_UARTEx_ReceiveToIdle_DMA(&huart7, ucMotor_Rx_Buffer[ucMotor_current_buf_idx], MOTOR_RX_BUF_SIZE);
  // 禁用半传输中断(否则会产生两次中断)
  __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
}

// Motor的GPDMA接收处理，在stm32h5xx_it.c中调用
void vMotor_RxEventCallback(uint16_t Size)
{
  //HAL_UART_DMAStop(&huart7);           // 停止当前DMA传输

  if (Size > 0) 
  {
    osMessageQueuePut(xMotor_Rx_Queue, ucMotor_Rx_Buffer[ucMotor_current_buf_idx], 0, 0);
  }
        
  // 切换缓冲区并重启接收
  ucMotor_current_buf_idx ^= 1;
  vMotor_Start_GPDMA_Receive();
}
