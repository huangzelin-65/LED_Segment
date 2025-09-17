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


extern UART_HandleTypeDef huart1;
extern PlcToCarData PlcToCarData_obj;
extern osMessageQueueId_t xTest_Rx_QueueHandle;
extern osMessageQueueId_t xPrint_QueueHandle;


/*  Test任务入口函数  */
void vTestTask(void *argument)
{
  uint8_t ucTest_Receive_Buffer[TEST_RX_BUF_SIZE];
  
  //启动DMA接收
  vTest_Start_GPDMA_Receive();

  while (1)
  {

    // 等待RX接收完成十六进制信号
    if (osMessageQueueGet(xTest_Rx_QueueHandle, ucTest_Receive_Buffer, NULL, osWaitForever) == osOK)
    {

      DEBUGINFO("Test received:%s, len:%d\r\n",ucTest_Receive_Buffer,strlen((char *)ucTest_Receive_Buffer));

      // PlcToCarData_obj.wSeq = ucTest_Receive_Buffer[1]<<8 | ucTest_Receive_Buffer[0]; // 序号
      // PlcToCarData_obj.dwPlcNum = ucTest_Receive_Buffer[5]<<24 | ucTest_Receive_Buffer[4]<<16 | ucTest_Receive_Buffer[3]<<8 | ucTest_Receive_Buffer[2]; // PLC编号
      // PlcToCarData_obj.wHeatBeat = ucTest_Receive_Buffer[BASE_COUNT+1]<<8 | ucTest_Receive_Buffer[BASE_COUNT]; // 心跳信号
      // PlcToCarData_obj.wAlm = ucTest_Receive_Buffer[BASE_COUNT+3]<<8 | ucTest_Receive_Buffer[BASE_COUNT+2]; // 报警信号
      // PlcToCarData_obj.wCtrl = ucTest_Receive_Buffer[BASE_COUNT+5]<<8 | ucTest_Receive_Buffer[BASE_COUNT+4]; // 控制信号
      // PlcToCarData_obj.bDire = ucTest_Receive_Buffer[BASE_COUNT+30]; // 小车运行方向 1=正转 2=反转

      PlcToCarData_obj.wCtrl = ucTest_Receive_Buffer[1]<<8 | ucTest_Receive_Buffer[0]; // 控制信号
      PlcToCarData_obj.bDire = ucTest_Receive_Buffer[2]; // 小车运行方向 1=正转 2=反转

      DEBUGINFO("wCtrl : %X\r\n",PlcToCarData_obj.wCtrl);

      vParseCommandToCar();

      // 重启RX接收
      vTest_Start_GPDMA_Receive();
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
      vPrint_start_Transmit(rxData, strlen(rxData));
      // HAL_UART_Transmit(&huart1, rxData, strlen((char *)rxData), HAL_MAX_DELAY);
      // vPortFree(rxData);
		}
	}
}