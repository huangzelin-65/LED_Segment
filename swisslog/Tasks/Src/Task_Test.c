#include "Task_Test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os2.h"
#include <string.h>
#include "motor_LD25B60G.h"
#include "LogDebugInfo.h"
#include "common.h"
#include "adaptor_test.h"

#define BASE_COUNT 6

uint8_t ucTest_Rx_Buffer[TEST_RX_BUF_SIZE];
uint8_t ucTest_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern UART_HandleTypeDef huart1;
extern PlcToCarData PlcToCarData_obj;
extern osMessageQueueId_t xTest_Rx_QueueHandle;
extern osMessageQueueId_t xPrint_QueueHandle;
extern osSemaphoreId_t xTestRxSemHandle;


/*  Test任务入口函数  */
void vTestTask(void *argument)
{
  
  //启动DMA接收
  vTest_Start_DMA_Receive(ucTest_Rx_Buffer);

  while (1)
  {

    // 等待RX接收完成十六进制信号
    if (osSemaphoreAcquire(xTestRxSemHandle, osWaitForever) == osOK)
    {
      // DEBUGINFO("Test received:%s, len:%d\r\n",ucTest_Rx_Buffer,strlen((char *)ucTest_Rx_Buffer));

      // for (u8 i=0;i<strlen((char *)ucTest_Rx_Buffer);i++)
      // {
      //   DEBUGINFO("%X ",ucTest_Rx_Buffer[i]);
      // }

      // PlcToCarData_obj.wSeq = ucTest_Receive_Buffer[1]<<8 | ucTest_Receive_Buffer[0]; // 序号
      // PlcToCarData_obj.dwPlcNum = ucTest_Receive_Buffer[5]<<24 | ucTest_Receive_Buffer[4]<<16 | ucTest_Receive_Buffer[3]<<8 | ucTest_Receive_Buffer[2]; // PLC编号
      // PlcToCarData_obj.wHeatBeat = ucTest_Receive_Buffer[BASE_COUNT+1]<<8 | ucTest_Receive_Buffer[BASE_COUNT]; // 心跳信号
      // PlcToCarData_obj.wAlm = ucTest_Receive_Buffer[BASE_COUNT+3]<<8 | ucTest_Receive_Buffer[BASE_COUNT+2]; // 报警信号
      // PlcToCarData_obj.wCtrl = ucTest_Receive_Buffer[BASE_COUNT+5]<<8 | ucTest_Receive_Buffer[BASE_COUNT+4]; // 控制信号
      // PlcToCarData_obj.bDire = ucTest_Receive_Buffer[BASE_COUNT+30]; // 小车运行方向 1=正转 2=反转

      PlcToCarData_obj.wCtrl = ucTest_Rx_Buffer[1]<<8 | ucTest_Rx_Buffer[0]; // 控制信号
      PlcToCarData_obj.bDire = ucTest_Rx_Buffer[2]; // 小车运行方向 1=正转 2=反转

      DEBUGINFO("wCtrl : %X, bDire : %X\r\n",PlcToCarData_obj.wCtrl,PlcToCarData_obj.bDire);

      vParseCommandToCar();

      // 重启RX接收
      vTest_Start_DMA_Receive(ucTest_Rx_Buffer);
    }
  }
}


void vPrintTask(void *argument)
{
  /* Infinite loop */
	uint8_t *rxData = NULL;
	while (1)
	{
		if(xQueueReceive(xPrint_QueueHandle, &rxData, portMAX_DELAY) == pdPASS)
		{
      vPrint_start_Transmit(rxData, strlen((char *)rxData));
      // HAL_UART_Transmit(&huart1, rxData, strlen((char *)rxData), HAL_MAX_DELAY);
      // vPortFree(rxData);
		}
	}
}