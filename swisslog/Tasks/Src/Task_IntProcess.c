/*
 * Task_IntHandle.c
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */

#include "../Inc/Task_IntProcess.h"

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "queue.h"
#include <stdio.h>
#include "sensors.h"
#include "LogDebugInfo.h"

extern osMessageQueueId_t xInterrupt_QueueHandle;
//extern osMessageQueueId_t motion_QueueHandle;
extern QueueHandle_t xSensor_QueueHandle;
extern osTimerId_t xSensorDebounceTimerHandle;
extern osTimerId_t xToggleSwitchTimerHandle;

volatile uint8_t SensorDebounce_flag;
volatile uint8_t ToggleDebounce_flag;


void vIntProcessTask(void *argument)
{
  u8 IntProcessRecv_msg;
  u8 sensor_msg;

  SensorDebounce_flag = 0;
  ToggleDebounce_flag = 0;

  while(1)
  {
    //if (xQueueReceive(xInterrupt_QueueHandle, &IntProcessRecv_msg, portMAX_DELAY) == pdPASS) 
    if (osMessageQueueGet(xInterrupt_QueueHandle, &IntProcessRecv_msg, NULL, osWaitForever) == osOK) 
    {
      DEBUGINFO("IntProcessRecv_msg = %d \r\n",IntProcessRecv_msg);
      switch(IntProcessRecv_msg)
      {
        // 判断 前碰撞传感器 是否触发或释放
        case FrontCrashSensor:
        // 判断 后碰撞传感器 是否触发或释放
        case RearCrashSensor: 
        // 判断 前距离传感器 是否触发或释放
        case FrontProxSensor:
        // 判断 后距离传感器 是否触发或释放
        case RearProxSensor:
          //传感器中断防重入
          if (SensorDebounce_flag == 0) {
            //防抖20ms
            osTimerStart(xSensorDebounceTimerHandle, pdMS_TO_TICKS(DebounceTime));
            SensorDebounce_flag = 1;  // 置位标志
          }
          break;
        
        case ToggleSwitch:
          //拨动中断开关防重入
          if (ToggleDebounce_flag == 0) {
            //防抖20ms
            osTimerStart(xToggleSwitchTimerHandle, pdMS_TO_TICKS(DebounceTime));
            ToggleDebounce_flag = 1;  // 置位标志
          }
          break;
        
        case ResetButton:
          sensor_msg = ResetButtonTrigger;
          if(osMessageQueuePut(xSensor_QueueHandle, &sensor_msg, 0, pdMS_TO_TICKS(100)) != osOK)
          {
            DEBUGINFO("vIntProcessTask() send sensor msg error\r\n");
          }
          break;

        case SensorDebounce:
          SensorDebounce_flag = 0;
          vSensorStatusCheck();     //检测传感器状态
          break;

        case ToggleDebounce:
          ToggleDebounce_flag = 0;
          vToggleSwitchStatusCheck();   //检测开关状态
          break;
        default:
          break;
      }
    }
  }
}
