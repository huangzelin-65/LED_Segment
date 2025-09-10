#include "Task_Test.h"
#include "FreeRTOS.h"
#include "task.h"
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
  Test_Rx_Frame_t frame;
  
  //启动DMA接收
  vTest_Start_GPDMA_Receive();

  while (1)
  {

    // 等待RX接收完成十六进制信号
    if (osMessageQueueGet(xTest_Rx_QueueHandle, &frame, NULL, osWaitForever) == osOK)
    {

      DEBUGINFO_ALL("Test received:%s, len:%d\r\n",frame.data,frame.len);

      PlcToCarData_obj.wSeq = frame.data[1]<<8 | frame.data[0]; // 序号
      PlcToCarData_obj.dwPlcNum = frame.data[5]<<24 | frame.data[4]<<16 | frame.data[3]<<8 | frame.data[2]; // PLC编号
      PlcToCarData_obj.wHeatBeat = frame.data[BASE_COUNT+1]<<8 | frame.data[BASE_COUNT]; // 心跳信号
      PlcToCarData_obj.wAlm = frame.data[BASE_COUNT+3]<<8 | frame.data[BASE_COUNT+2]; // 报警信号
      PlcToCarData_obj.wCtrl = frame.data[BASE_COUNT+5]<<8 | frame.data[BASE_COUNT+4]; // 控制信号
      PlcToCarData_obj.bDire = frame.data[BASE_COUNT+30]; // 小车运行方向 1=正转 2=反转

      DEBUGINFO_ALL("wCtrl : %X\r\n",PlcToCarData_obj.wCtrl);

      vParseCommandToCar();

      // 重启RX接收
      vTest_Start_GPDMA_Receive();
    }
  }
}


/*  print任务入口函数  */
void vPrintTask(void *argument)
{
  char *rxData = NULL;
  while (1)
  {
    osStatus_t stat = osMessageQueueGet(xPrint_QueueHandle, &rxData, NULL,portMAX_DELAY);
    if(stat == osOK)
    {
      //HAL_UART_Transmit(&huart1, (uint8_t *)rxData, strlen(rxData), HAL_MAX_DELAY);
      vPrint_start_Transmit((uint8_t *)rxData, (uint16_t)strlen(rxData));
      vPortFree(rxData);
    }
  }
}
