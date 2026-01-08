/*
 * Task_MotorCtrl.c
 *
 *  Created on: Aug 27, 2025
 *      Author: e3lijia25d
 */

#include "Task_MotionCtrl.h"
#include "FreeRTOS.h"
#include "task.h"
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

// 小车运行状态初始化
void vCarRunStatusInit()
{
    // 初始化为正向正常速度，禁用电机
    CarStatus.xSetSpeed = NormalSpeed;
    CarStatus.xRealDirection = Forward;
}

// 电机参数初始化
void vMotorInit()
{
    CarStatus.xIsCarRunning = CarStop; //小车状态记录为停止
    CarStatus.xMotorEnable = MotorDisable;
    CarStatus.xMotorStopReason = ByInit; // 电机停止原因
    DEBUGINFO("MotorStopReason: ByInit\r\n");
    vMotorDisable(); // 禁用电机

    osDelay(pdMS_TO_TICKS(500));
    vMotorSetting(MOTOR_SETTING_ACCEL, MOTOR_SETTING_ACCEL_8000); // 设置电机加速度
    osDelay(pdMS_TO_TICKS(500));
    vMotorSetting(MOTOR_SETTING_DECEL, MOTOR_SETTING_DECEL_8000); // 设置电机减速度
}

// 设置小车“远程手动模式”/“远程自动模式”
void vRemoteModeSet(eAutoModeType xAutoMode)
{
    uint8_t mode_change = 0;
    if(xAutoMode != CarStatus.xAutoMode)
    {
        mode_change = 1;
    }
    switch (xAutoMode)
    {
        // 远程手动模式
        case Manual:
            DEBUGINFO("remote manual mode");
            CarStatus.xAutoMode = Manual;
            break;

        //小车远程自动模式 
        case Auto:
            DEBUGINFO("remote auto mode");
            CarStatus.xAutoMode = Auto;
            break;

        default:
            break;
    }
    if(mode_change)Robot_Event();
}

// 远程设置小车“运行方向”和“运行速度”
void vRemoteMotionCmd(eDirectionType xDirection, eSpeedType xSpeed)
{
    uint8_t u8_Motion_msg;

    //************************ 设置电机运行方向 **************************//
    if(Manual == CarStatus.xAutoMode){
        CarStatus.xRealDirection = xDirection; // 手动模式下设置实际方向
    }else if(Auto == CarStatus.xAutoMode){
        CarStatus.xSetDirection = xDirection; // 自动模式下设置预设方向
    }

    //************************ 设置 电机运行速度 和 电机使能 **************************//
    if(xSpeed == ZeroSpeed)
    {
        CarStatus.xMotorEnable = MotorDisable; // 停止电机
        CarStatus.xMotorStopReason = ByCommand; // 电机停止原因
        DEBUGINFO("MotorStopReason: ByCommand");

        u8_Motion_msg = CarStop;  
        // 发送电机控制消息
        if(osMessageQueuePut(xMotion_QueueHandle, &u8_Motion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
        {
          DEBUGINFO("send motion msg error");
        }
    }
    else if((xSpeed == LowSpeed)||(xSpeed == NormalSpeed)||(xSpeed == HighSpeed))
    {
        CarStatus.xSetSpeed = xSpeed; // 设置电机运行速度
        CarStatus.xMotorEnable = MotorEnable; // 电机使能
        u8_Motion_msg = CarRunning;

        // 发送电机控制消息
        if(osMessageQueuePut(xMotion_QueueHandle, &u8_Motion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
        {
          DEBUGINFO("send motion msg error");
        }
    }
}


void vMotionCtrlTask(void *argument)
{
  vCarRunStatusInit(); // 小车运行状态初始化

  vMotorInit(); // 电机参数初始化

  Robot_Event(); // 上报小车状态

  while (1)
    {
      if ( osMessageQueueGet(xMotion_QueueHandle, &MotionRecv_msg, NULL, osWaitForever) == osOK ) 
      {
        DEBUGINFO("osMessageQueueGet:%d",MotionRecv_msg);
        switch( MotionRecv_msg )
        {
          case CarStop:
            if(CarStatus.xIsCarRunning != CarStop)
            {
              CarStatus.xIsCarRunning = CarStop; //小车状态记录为停止
            }
            CarStatus.xMotorEnable = MotorDisable; //电机使能状态清除

            // 若是由于rfid停止标签导致的停止，则不禁用电机
            if(CarStatus.xMotorStopReason == ByStopTag){
              vMotorOps(NoDirection, ZeroSpeed);  // 设置0速度
            } else {
              vMotorDisable();  // 禁用电机
            }
            
            GPIO_WRITE(LED4, GPIO_PIN_RESET); // 关闭LED4
            DEBUGINFO("disable LED4 ");
            break;
          
          case CarRunning:
            DEBUGINFO("CarStatus.xAutoMode:%d, CarStatus.xMotorEnable:%d",CarStatus.xAutoMode, CarStatus.xMotorEnable);
            // 拨动开关自动挡
            if( CarStatus.ToggleSwtichPosition == ToggleFront )
            {
              // 远程自动模式
              if(CarStatus.xAutoMode == Auto)
              {
                DEBUGINFO("remote Auto mode");

                //车厢锁上才能发车
                if( CarStatus.xBoxLocked == Locked )
                {
                   // 未允许电机运行（通过wifi或串口指令下发允许）
                  if( CarStatus.xMotorEnable == MotorDisable )
                  {
                    DEBUGINFO("ReadyToRun");
                    CarStatus.xIsCarRunning = CarReadyToRun;
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
                    DEBUGINFO("LED4 ON");
                  }
                } else {
                  CarStatus.xMotorEnable = MotorDisable;  // 停止电机
                  CarStatus.xMotorStopReason = ByBoxUnlock; // 电机停止原因
                  DEBUGINFO("MotorStopReason: ByBoxUnlock");
                }
              }

              // 远程手动模式
              else if( CarStatus.xAutoMode == Manual )
              {
                DEBUGINFO("remote Manual mode");
                // 电机按实际运行方向 和 普通速度运行（手动档下）
                CarStatus.xIsCarRunning = CarRunning;
                vMotorEnable();  // 电机使能
                osDelay(pdMS_TO_TICKS(100));
                vMotorOps(CarStatus.xRealDirection, CarStatus.xSetSpeed);  
                GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                DEBUGINFO("LED4 ON");
              }


            }
            // 拨动开关手动挡
            else if( CarStatus.ToggleSwtichPosition == ToggleBack )
            {
              DEBUGINFO("local Manual mode");

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
                  DEBUGINFO("LED4 ON");
                }
              }
            }
            // 拨动开关停止挡
            else if(CarStatus.ToggleSwtichPosition == ToggleStop)
            {
              CarStatus.xIsCarRunning = CarStop; //小车状态记录为停止
              CarStatus.xMotorEnable = MotorDisable; //电机使能状态清除
              CarStatus.xMotorStopReason = ByToggleStop;
              DEBUGINFO("MotorStopReason: ByToggleStop");
            }
            break;
        }
        Robot_Event(); // 上报小车状态
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
