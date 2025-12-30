#include "Task_Test.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cmsis_os2.h"
#include <string.h>
#include "motor_LD25B60G.h"
#include "LogDebugInfo.h"
#include "Common.h"
#include "adaptor_test.h"
#include "Task_MotionCtrl.h"
#include "Task_BoxCtrl.h"

#define BASE_COUNT 6

uint8_t ucTest_Rx_Buffer[TEST_RX_BUF_SIZE];
uint8_t ucTest_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern UART_HandleTypeDef huart1;
extern ServerToCarData_t ServerToCarData;
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
        uint32_t ucReciveLen = 0;

        // 等待RX接收完成十六进制信号
        if (osSemaphoreAcquire(xTestRxSemHandle, osWaitForever) == osOK)
        {
            ucReciveLen = ulTest_Get_DMA_Receive_Len();
            DEBUGINFO("Test received len:%d",ucReciveLen);
            vPrint_Array(ucTest_Rx_Buffer, ucReciveLen);

            ServerToCarData.xAutoMode = ucTest_Rx_Buffer[0]; // 模式选择
            ServerToCarData.xDirection = ucTest_Rx_Buffer[1]; // 方向
            ServerToCarData.xSetSpeed = ucTest_Rx_Buffer[2]; // 速度
            ServerToCarData.xScreenLockStatus = ucTest_Rx_Buffer[3]; // 屏幕锁定状态
            // ServerToCarData.xMotorEnable = ucTest_Rx_Buffer[3]; // 电机使能运行
            // ServerToCarData.xStationStatus = ucTest_Rx_Buffer[4]; // 到站状态
            // DEBUGINFO("xAutoMode : %X, xDirection : %X, xSetSpeed : %X, xMotorEnable : %X, xStationStatus : %X",
            //   ServerToCarData.xAutoMode, ServerToCarData.xDirection, ServerToCarData.xSetSpeed, ServerToCarData.xMotorEnable, ServerToCarData.xStationStatus);

            DEBUGINFO("xAutoMode : %X, xDirection : %X, xSetSpeed : %X, xScreenLockStatus : %X",
              ServerToCarData.xAutoMode, ServerToCarData.xDirection, ServerToCarData.xSetSpeed, ServerToCarData.xScreenLockStatus);

            // vParseCommandToCar();
            vRemoteModeSet(ServerToCarData.xAutoMode);
            vRemoteMotionCmd(ServerToCarData.xDirection, ServerToCarData.xSetSpeed);
            vSet_Screen_LockStatus(ServerToCarData.xScreenLockStatus);

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
      #ifdef LOG_USE_MALLOC
      vPrint_start_Transmit(rxData, strlen((char *)rxData));     
      #else
      // vPrint_start_Transmit(rxData, strlen((char *)rxData));
      HAL_UART_Transmit(&huart1, rxData, strlen((char *)rxData), 100);
      // vPortFree(rxData);
      #endif
		}
	}
}