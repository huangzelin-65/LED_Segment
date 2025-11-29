#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_HMI.h"
#include "LogDebugInfo.h"


extern UART_HandleTypeDef huart10;
extern osSemaphoreId_t xHMITxSemHandle;

void vSendToHMI(uint8_t * CmdDataArr,uint8_t len)
{
  // DEBUGINFO("send len = %d ,data:",len);
  // vPrint_Array(CmdDataArr,len);

	// 获取HMI发送锁
  if (osSemaphoreAcquire(xHMITxSemHandle, osWaitForever) == osOK) 
  {

    taskENTER_CRITICAL(); // 进入临界区
    HAL_UART_Transmit_DMA(&huart10, CmdDataArr, len);    // 启动DMA发送
    taskEXIT_CRITICAL(); // 退出临界区
    //DEBUGINFO("send finish \r\n");
  }
}

void vHMI_Start_DMA_Receive(uint8_t* ucHMI_Rx_Buffer) {
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart10, ucHMI_Rx_Buffer, HMI_RX_BUF_SIZE)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart10, ucHMI_Rx_Buffer, HMI_RX_BUF_SIZE);
  }
  __HAL_DMA_DISABLE_IT(huart10.hdmarx, DMA_IT_HT);
}

uint32_t ulHMI_Get_DMA_Receive_Len(void)
{
	uint32_t ulLen = HMI_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart10.hdmarx);
  return ulLen;
}