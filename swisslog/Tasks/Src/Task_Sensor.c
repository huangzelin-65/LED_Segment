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
extern CarStatus_t CarStatus;
extern CarToServerData_t CarToServerData;
extern osTimerId_t xResetButtonTimerHandle;
extern u8 Beep_enable;



void vCarInStation(void)
{
  DEBUGINFO("Car In Station\r\n");
}

void vCarOutStation(void)
{
  DEBUGINFO("Car Out Station\r\n");
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

  if (xSensor_QueueHandle == NULL) {
    // 队列创建失败，处理错误
    DEBUGINFO("xSensor_QueueHandle == NULL\r\n");
  }
  
  // //蜂鸣器响0.5s
  // vBeep_Control(ENABLE);
  // osDelay(pdMS_TO_TICKS(1000));
  // vBeep_Control(DISABLE);

  //电机测试
  //vMotorTest();

  //开机状态自检
  vCarSelfCheck();
  
  while (1)
  {
    //if (xQueueReceive(xSensor_QueueHandle, &ucSensorRecv_msg, portMAX_DELAY) == pdPASS) 
    if (osMessageQueueGet(xSensor_QueueHandle, &ucSensorRecv_msg, NULL, osWaitForever) == osOK) 
    {
      switch (ucSensorRecv_msg)
      {
        //*************************** 传感器事件 ***************************
        case SensorEvent:
          //向前运动时
          {
            if(CarStatus.xRealDirection == Forward)
            {
              DEBUGINFO("Forward\r\n");
              
              // 传感器异常
              if(((CarStatus.FrontCrashStatus == SensorError) \
                || (CarStatus.FrontProxStatus == SensorError)))
              {
                GPIO_WRITE(LED1, GPIO_PIN_SET); // 使能LED1
                DEBUGINFO("sensors Error\r\n");
                DEBUGINFO("enable LED1\r\n");
              
                // 电机停止原因
                CarStatus.xMotorStopReason = BySensorError;
                DEBUGINFO("MotorStopReason: BySensorError\r\n");
                // 请求停止电机
                ucMotion_msg = CarStop;
              }

              //运动方向上其中一个传感器触发即可触发
              if (((CarStatus.FrontCrashStatus == SensorTrigger) \
                || (CarStatus.FrontProxStatus == SensorTrigger)))
              {
                GPIO_WRITE(LED1, GPIO_PIN_SET); // 使能LED1
                DEBUGINFO("sensors Trigger\r\n");
                DEBUGINFO("enable LED1\r\n");

                // 电机停止原因
                CarStatus.xMotorStopReason = BySensorTrigger;
                DEBUGINFO("MotorStopReason: BySensorTrigger\r\n");
                // 请求停止电机
                ucMotion_msg = CarStop;
              }
              //运动方向上其中全部传感器释放即可释放
              if ((CarStatus.FrontCrashStatus == SensorRelease) \
                && (CarStatus.FrontProxStatus == SensorRelease))
              {
                GPIO_WRITE(LED1, GPIO_PIN_RESET); // 关闭LED1
                DEBUGINFO("sensors release\r\n");
                DEBUGINFO("disable LED1\r\n");

                //请求启动电机
                ucMotion_msg = CarRunning;
              }
            }
            //向后运动时
            else if(CarStatus.xRealDirection == Backward)
            {
              DEBUGINFO("Backward\r\n");

              // 传感器异常
              if(((CarStatus.RearCrashStatus == SensorError) \
                || (CarStatus.RearProxStatus == SensorError)))
              {
                GPIO_WRITE(LED1, GPIO_PIN_SET); // 使能LED1
                DEBUGINFO("sensors Error\r\n");
                DEBUGINFO("enable LED1\r\n");

                // 电机停止原因
                CarStatus.xMotorStopReason = BySensorError;
                DEBUGINFO("MotorStopReason: BySensorError\r\n");
                // 请求停止电机
                ucMotion_msg = CarStop;
              }

              //运动方向上其中一个传感器触发即可触发
              if (((CarStatus.RearCrashStatus == SensorTrigger) \
                || (CarStatus.RearProxStatus == SensorTrigger)))
              {
                GPIO_WRITE(LED1, GPIO_PIN_SET); // 使能LED1
                DEBUGINFO("sensors Trigger\r\n");
                DEBUGINFO("enable LED1\r\n");

                // 电机停止原因
                CarStatus.xMotorStopReason = BySensorTrigger;
                DEBUGINFO("MotorStopReason: BySensorTrigger\r\n");
                // 请求停止电机
                ucMotion_msg = CarStop;
              }
              //运动方向上其中全部传感器释放即可释放
              if ((CarStatus.RearCrashStatus == SensorRelease) \
                && (CarStatus.RearProxStatus == SensorRelease))
              {
                GPIO_WRITE(LED1, GPIO_PIN_RESET); // 关闭LED1
                DEBUGINFO("sensors release\r\n");
                DEBUGINFO("disable LED1\r\n");

                //请求启动电机
                ucMotion_msg = CarRunning;
              }
            }
          }
          if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
          {
            DEBUGINFO("SensorEvent send motion msg error\r\n");
          }
          break;


        //*************************** 拨动开关向前（运行方向不变） ***********************
        case ToggleFront:
          // 设置为自动模式
          CarStatus.xAutoMode = Auto;

          // 预设值向前
          if(CarStatus.xSetDirection == Forward)
          {
            CarStatus.xRealDirection = Forward; //实际运行方向记录为向前
          }
          // 预设值向后
          else if(CarStatus.xSetDirection == Backward)
          {
            CarStatus.xRealDirection = Backward; //实际运行方向记录为向后
          }
          
          // 通过sensor自检，判断是否可以运动，并跳转到运动控制任务
          vSensorStatusCheck(); 
          break;


        //**************************** 拨动开关向后（运行方向相反）*********************
        case ToggleBack:
/******************** TK1.0的手动模式为反方向运动 *********************

          // 设置为手动模式
          CarStatus.xAutoMode = Manual;

          // 预设值向前
          if(CarStatus.xSetDirection == Forward)
          {
            CarStatus.xRealDirection = Backward; //实际运行方向记录为向后
          }

          // 预设值向后
          else if(CarStatus.xSetDirection == Backward)
          {
            CarStatus.xRealDirection = Forward; //实际运行方向记录为向前
          }

          // 通过sensor自检，判断是否可以运动，并跳转到运动控制任务
          vSensorStatusCheck(); 

***************** TK2.0以后的手动模式改为维修拨杆点动模式 ******************/

          break;


        //********************************* 拨动开关停止 ******************************
        case ToggleStop:
          CarStatus.xAutoMode = Manual;
          // 电机停止原因
          CarStatus.xMotorStopReason = ByToggleStop;
          DEBUGINFO("MotorStopReason: ByToggleStop\r\n");
          // 停止电机
          ucMotion_msg = CarStop;
          if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
          {
            DEBUGINFO("ToggleStop send motion msg error\r\n");
          }
          break;

        //*********************************** 维修控杆向前 ********************************
        case ServiceFront:
          CarStatus.xRealDirection = Forward; //实际运行方向记录为向前

          // 通过sensor自检，判断是否可以运动，并跳转到运动控制任务
          vSensorStatusCheck(); 
          break;
        

        //************************************ 维修控杆向后 ********************************
        case ServiceBack:
          CarStatus.xRealDirection = Backward; //实际运行方向记录为向后
          
          // 通过sensor自检，判断是否可以运动，并跳转到运动控制任务
          vSensorStatusCheck(); 
          break;


        //************************************ 维修控杆停止 ********************************
        case ServiceStop:
          // 停止电机
          CarStatus.xMotorStopReason = ByServiceMode;
          DEBUGINFO("MotorStopReason: ByServiceMode\r\n");

          ucMotion_msg = CarStop;
          if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
          {
            DEBUGINFO("ServiceStop send motion msg error\r\n");
          }
          break;


        //******************************* 复位按钮触发 ******************************
        case ResetButtonTrigger:
          // 按钮按下
          if(GPIO_READ(RESET) == GPIO_PIN_RESET)
          {
            GPIO_WRITE(LED_RESET, GPIO_PIN_SET); // 使能LED_RESET
            DEBUGINFO("reset button trigger\r\n");

            // 电机停止原因
            CarStatus.xMotorStopReason = ByReset;
            DEBUGINFO("MotorStopReason: ByReset\r\n");

            // 请求停止电机
            ucMotion_msg = CarStop;
            if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
            {
              DEBUGINFO("send motion msg error3\r\n");
            }
            osTimerStart(xResetButtonTimerHandle, pdMS_TO_TICKS(ResetDuration));// 回调vResetButtonCallback
          }
          // 按钮松开
          else
          {
            GPIO_WRITE(LED_RESET, GPIO_PIN_RESET); // 关闭LED_RESET
            DEBUGINFO("reset button release\r\n");
          }
          break;
        

        //*********************** 开始复位（复位按钮持续触发1秒） **********************
        case Reset:
          DEBUGINFO("SystemReset\r\n");
          NVIC_SystemReset();
          break;

        default:
          break;
      }
    }
  }
}


