/*
 * Task_CarRun.c
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "queue.h"
#include "Task_Sensor.h"
#include <stdio.h>
#include "sensors.h"
#include "motor_LD25B60G.h"
#include "LogDebugInfo.h"
#include "adaptor_beep.h"

extern QueueHandle_t xMotion_QueueHandle;
extern QueueHandle_t xSensor_QueueHandle;
extern _CarCheckFlag_obj CarCheckFlagobj;
extern _CarRunStatus_obj CarRunStatus_obj;
extern CarToPlcData CarToPlcData_obj;
extern osTimerId_t xResetButtonTimerHandle;
extern u8 Beep_enable;



void vCarInStation(void)
{
  DEBUGINFO_ALL("Car In Station\r\n");
}

void vCarOutStation(void)
{
  DEBUGINFO_ALL("Car Out Station\r\n");
}


void vCarSelfCheck(void)
{
  //vSensorStatusCheck();
  vToggleSwitchStatusCheck(); //已经包含了vSensorStatusCheck()，ToggleStop状态也不需要检测传感器
  vResetLedStatusCheck();
}


void vSensorTask(void *argument)
{

  u8 ucSensorRecv_msg;
  u8 ucMotion_msg;

  //蜂鸣器响0.5s
  vBeep_Control(ENABLE);
  //osDelay(500);
  vBeep_Control(DISABLE);

  //延时0.5s，让电机上电完成
  //vTaskDelay(500);

  //电机测试
  //vMotorTest();

  //开机状态自检
  vCarSelfCheck();
  
  while (1)
  {
    if (xQueueReceive(xSensor_QueueHandle, &ucSensorRecv_msg, portMAX_DELAY) == pdPASS) 
    {
      switch (ucSensorRecv_msg)
      {
        //传感器事件
        case SensorEvent:
          //向前运动时
          {
            if(CarToPlcData_obj.bDire == Forward)
            {
              //运动方向上其中一个传感器触发即可触发
              if (((CarCheckFlagobj.FrontCrashStatus == SensorTrigger) \
                || (CarCheckFlagobj.FrontProxStatus == SensorTrigger)))
              {
                GPIO_WRITE(LED1, GPIO_PIN_SET); // 使能LED1
                DEBUGINFO_ALL("enable LED1\r\n");

                //请求停止电机
                ucMotion_msg = CarStop;
              }
              //运动方向上其中全部传感器释放即可释放
              if ((CarCheckFlagobj.FrontCrashStatus == SensorRelease) \
                && (CarCheckFlagobj.FrontProxStatus == SensorRelease))
              {
                GPIO_WRITE(LED1, GPIO_PIN_RESET); // 关闭LED1
                DEBUGINFO_ALL("disable LED1\r\n");

                //请求启动电机
                ucMotion_msg = CarRunning;
              }
            }
            //向后运动时
            else if(CarToPlcData_obj.bDire == Backward)
            {
              //运动方向上其中一个传感器触发即可触发
              if (((CarCheckFlagobj.RearCrashStatus == SensorTrigger) \
                || (CarCheckFlagobj.RearProxStatus == SensorTrigger)))
              {
                GPIO_WRITE(LED1, GPIO_PIN_SET); // 使能LED1
                DEBUGINFO_ALL("enable LED1\r\n");

                //请求停止电机
                ucMotion_msg = CarStop;
              }
              //运动方向上其中全部传感器释放即可释放
              if ((CarCheckFlagobj.RearCrashStatus == SensorRelease) \
                && (CarCheckFlagobj.RearProxStatus == SensorRelease))
              {
                GPIO_WRITE(LED1, GPIO_PIN_RESET); // 关闭LED1
                DEBUGINFO_ALL("disable LED1\r\n");

                //请求启动电机
                ucMotion_msg = CarRunning;
              }
            }
          }
          if(xQueueSend(xMotion_QueueHandle, &ucMotion_msg, pdMS_TO_TICKS(100)) != pdPASS)
          {
            DEBUGINFO_ALL("vSensorask() send motion msg error1\r\n");
          }
          break;

        // 拨动开关向前
        case ToggleFront:
          // 设置为自动模式
          CarRunStatus_obj.AutoMode = Auto;
          // 通过sensor自检，判断是否可以运动，并跳转到运动控制任务
          vSensorStatusCheck(); 
          break;

        // 拨动开关向后
        case ToggleBack:
          // 设置为手动模式
          CarRunStatus_obj.AutoMode = Manual;
          // 通过sensor自检，判断是否可以运动，并跳转到运动控制任务
          vSensorStatusCheck(); 
          break;


        // 拨动开关停止
        case ToggleStop:
          CarRunStatus_obj.AutoMode = Manual;
          // 停止电机
          ucMotion_msg = CarStop;
          if(xQueueSend(xMotion_QueueHandle, &ucMotion_msg, pdMS_TO_TICKS(100)) != pdPASS)
          {
            DEBUGINFO_ALL("vSensorask() send motion msg error2\r\n");
          }
          break;

        // 复位按钮触发
        case ResetButtonTrigger:
          // 按钮按下
          if(GPIO_READ(RESET) == GPIO_PIN_RESET)
          {
            GPIO_WRITE(LED_RESET, GPIO_PIN_SET); // 使能LED_RESET
            DEBUGINFO_ALL("reset button trigger\r\n");

            // 请求停止电机
            ucMotion_msg = CarStop;
            if(xQueueSend(xMotion_QueueHandle, &ucMotion_msg, pdMS_TO_TICKS(100)) != pdPASS)
            {
              DEBUGINFO_ALL("vSensorask() send motion msg error3\r\n");
            }
            osTimerStart(xResetButtonTimerHandle, pdMS_TO_TICKS(ResetDuration));
          }
          // 按钮松开
          else
          {
            GPIO_WRITE(LED_RESET, GPIO_PIN_RESET); // 关闭LED_RESET
            DEBUGINFO_ALL("reset button release\r\n");
          }
          break;
        
        // 开始复位（复位按钮持续触发1秒）
        case Reset:
          DEBUGINFO_ALL("SystemReset\r\n");
          NVIC_SystemReset();
          break;

        default:
          break;
      }
    }
  }
}


