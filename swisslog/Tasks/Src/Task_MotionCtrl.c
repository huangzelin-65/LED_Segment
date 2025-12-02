/*
 * Task_MotorCtrl.c
 *
 *  Created on: Aug 27, 2025
 *      Author: e3lijia25d
 */

#include "Task_MotionCtrl.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "queue.h"
#include <string.h>
#include "LogDebugInfo.h"
#include "adaptor_rfid.h"
#include "adaptor_motor.h"
#include "adaptor_wifi.h"
#include "motor_LD25B60G.h"
#include "Robot.h"
// DMA缓冲区
#define MOTOR_BUF_SIZE 16
uint8_t MotorDmaBuffer[2][MOTOR_BUF_SIZE]={0};
uint8_t ucMotor_Task_Rx_Buffer[MOTOR_RX_BUF_SIZE];

uint8_t* motor_msg;
uint8_t MotionRecv_msg;
uint8_t MotorDataLen = 0;

extern osMessageQueueId_t xMotion_QueueHandle;
extern osMessageQueueId_t xMotor_Rx_QueueHandle;
extern osSemaphoreId_t xMotorRxSemHandle;
extern CarToServerData_t CarToServerData;
extern CarStatus_t CarStatus;


void vCarRunStatusInit()
{
  //初始化为正向正常速度运行
  CarStatus.xSetSpeed = NormalSpeed;
  CarStatus.xRealDirection = Forward;
  CarStatus.xMotorEnable = MotorDisable;
}

void vMotorInit()
{
  // 电机使能
  vMotorEnable();

  osDelay(pdMS_TO_TICKS(500));

  // 设置电机加速度
  vMotorSetting(MOTOR_SETTING_ACCEL, MOTOR_SETTING_ACCEL_8000);

  osDelay(pdMS_TO_TICKS(500));

  // 设置电机减速度
  vMotorSetting(MOTOR_SETTING_DECEL, MOTOR_SETTING_DECEL_8000);
}

void vMotionCtrlTask(void *argument)
{
  
  // 小车运行状态初始化
  vCarRunStatusInit();

  // 电机参数初始化
  vMotorInit();

  while (1)
    {
      if ( osMessageQueueGet(xMotion_QueueHandle, &MotionRecv_msg, NULL, osWaitForever) == osOK ) 
      {
        DEBUGINFO("osMessageQueueGet:%d\r\n",MotionRecv_msg);
        switch( MotionRecv_msg )
        {
          case CarStop:
            if(CarStatus.xIsCarRunning != CarStop)
            {
              CarStatus.xIsCarRunning = CarStop; //小车状态记录为停止
              Robot_Event();
            }
            CarStatus.xMotorEnable = MotorDisable; //电机使能状态清除
            // CarStatus.xIsCarRunning = CarStop; //小车状态记录为停止

            // 若是由于rfid停止标签导致的停止，则不禁用电机
            if(CarStatus.xMotorStopReason == ByStopTag)
            {
              vMotorOps(NoDirection, ZeroSpeed);  // 设置0速度
            } else {
              vMotorDisable();  // 禁用电机
            }
            
            GPIO_WRITE(LED4, GPIO_PIN_RESET); // 关闭LED4
            DEBUGINFO("disable LED4 \r\n");
            break;
          
          case CarRunning:
            DEBUGINFO("CarStatus.xAutoMode:%d, CarStatus.xMotorEnable:%d\r\n",CarStatus.xAutoMode, CarStatus.xMotorEnable);
            // 拨动开关自动档
            if( CarStatus.ToggleSwtichPosition == ToggleFront )
            {
              // 远程自动模式
              if(CarStatus.xAutoMode == Auto)
              {
                DEBUGINFO("remote Auto mode\r\n");

                //车厢锁上才能发车
                if( CarStatus.xBoxLocked == Locked )
                {
                   // 未允许电机运行（通过wifi或串口指令下发允许）
                  if( CarStatus.xMotorEnable == MotorDisable )
                  {
                    DEBUGINFO("ReadyToRun\r\n");
                    CarStatus.xIsCarRunning = CarReadyToRun;
                    //u8 temp[] = "MotionCtrl: ReadyToRun\r\n";
                    //vSendToWifiTX(temp, strlen((char *)temp));
                    break;
                  }
                  // 已允许电机运行
                  else if( CarStatus.xMotorEnable == MotorEnable )
                  {
                    // 电机按预设运行方向 和 预设速度运行
                    CarStatus.xIsCarRunning = CarRunning;
                    CarStatus.xRealDirection = CarStatus.xSetDirection;

                    // 若是由于rfid停止标签导致的停止，则不用再使能电机
                    if(CarStatus.xMotorStopReason == ByStopTag)
                    {
                      vMotorOps(CarStatus.xRealDirection, CarStatus.xSetSpeed);  
                    } else {
                      vMotorEnable();  // 电机使能
                      osDelay(pdMS_TO_TICKS(100));
                      vMotorOps(CarStatus.xRealDirection, CarStatus.xSetSpeed); 
                    }
                    
                    GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                    DEBUGINFO("LED4 ON\r\n");
                  }
                } else {
                  DEBUGINFO("Can't Run!! Box not Locked!!\r\n");
                }
              }

              // 远程手动模式
              else if( CarStatus.xAutoMode == Manual )
              {
                DEBUGINFO("remote Manual mode\r\n");
                // 电机按实际运行方向 和 普通速度运行（手动档下）
                CarStatus.xIsCarRunning = CarRunning;
                vMotorEnable();  // 电机使能
                osDelay(pdMS_TO_TICKS(100));
                vMotorOps(CarStatus.xRealDirection, CarStatus.xSetSpeed);  
                GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                DEBUGINFO("LED4 ON\r\n");
              }


            }
            // 拨动开关手动档
            else if( CarStatus.ToggleSwtichPosition == ToggleBack )
            {
              DEBUGINFO("local Manual mode\r\n");

              // reset按钮没有按下才允许电机运行
              if( GPIO_READ(RESET) == GPIO_PIN_SET )
              {
                // 维修控杆触发才允许电机运行
                if( CarStatus.ServiceJoystickPosition == ServiceFront || CarStatus.ServiceJoystickPosition == ServiceBack )
                {
                  // 电机按最低速度运行（手动档下）
                  CarStatus.xIsCarRunning = CarRunning;
                  vMotorEnable();  // 电机使能
                  osDelay(pdMS_TO_TICKS(100));
                  vMotorOps(CarStatus.xRealDirection, LowSpeed); 
                  GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                  DEBUGINFO("LED4 ON\r\n");
                }
              }
            }
            // 拨动开关停止档
            else if(CarStatus.ToggleSwtichPosition == ToggleStop)
            {
              CarStatus.xIsCarRunning = CarStop; //小车状态记录为停止
              CarStatus.xMotorEnable = MotorDisable; //电机使能状态清除
              CarStatus.xMotorStopReason = ByToggleStop;
              DEBUGINFO("MotorStopReason: ByToggleStop\r\n");
            }
            Robot_Event();
            break;

        }
      }
    }
}


/* 电机反馈任务入口函数 */
void vMotorFeedbackTask(void *argument)
{
  uint32_t ucReciveLen = 0;

  //启动DMA接收
  vMotor_Start_DMA_Receive(ucMotor_Task_Rx_Buffer);

  while (1)
  {

    // 等待DMA接收完成信号
    if (osSemaphoreAcquire(xMotorRxSemHandle, osWaitForever) == osOK)    
    {
      ucReciveLen = ulMotor_Get_DMA_Receive_Len();

      DEBUGINFO("Motor received len:%d",ucReciveLen);
      vPrint_Array(ucMotor_Task_Rx_Buffer, ucReciveLen);

      // 重启DMA接收(DMA单次模式下，重启后从缓冲区起始地址覆盖写入)
      vMotor_Start_DMA_Receive(ucMotor_Task_Rx_Buffer);
    }
  }
}
