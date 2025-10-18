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
extern CarToServerData CarToServerData_obj;
extern _CarRunStatus_obj CarRunStatus_obj;
extern _CarCheckFlag_obj CarCheckFlag_obj;


void vCarRunStatusInit()
{
  //初始化为正向正常速度运行
  CarRunStatus_obj.SetSpeed = NormalSpeed;
  CarToServerData_obj.ucDirection = Forward;
  CarRunStatus_obj.MotorEnable = MotorDisable;
}

void vMotionCtrlTask(void *argument)
{
  
  //小车运行状态初始化
  vCarRunStatusInit();

  while (1)
    {
      if (osMessageQueueGet(xMotion_QueueHandle, &MotionRecv_msg, NULL, osWaitForever) == osOK) 
      {
        switch(MotionRecv_msg)
        {
          case CarStop:
            //CarRunStatus_obj.RealDirection = NoDirection; //清除方向记录
            CarRunStatus_obj.MotorEnable = MotorDisable; //电机使能状态清除
            CarRunStatus_obj.IsCarRunning = CarStop; //小车状态记录为停止
            vMotorOps(NoDirection, ZeroSpeed);  // 电机停止
            GPIO_WRITE(LED4, GPIO_PIN_RESET); // 关闭LED4
            DEBUGINFO("disable LED4 \r\n");
            break;
          
          case CarRunning:
            // 拨动开关自动档
            if(CarCheckFlag_obj.ToggleSwtichPosition == ToggleFront)
            {
              // 远程自动模式
              if(CarRunStatus_obj.AutoMode == Auto)
              {
                DEBUGINFO("remote Auto mode\r\n");

                // 未允许电机运行（通过wifi或串口指令下发允许）
                if(CarRunStatus_obj.MotorEnable == MotorDisable)
                {
                  DEBUGINFO("ReadyToRun\r\n");
                  CarRunStatus_obj.IsCarRunning = CarReadyToRun;
                  u8 temp[] = "MotionCtrl: ReadyToRun\r\n";
                  vSendToWifiTX(temp, strlen((char *)temp));
                  break;
                }
                // 已允许电机运行
                else if(CarRunStatus_obj.MotorEnable == MotorEnable)
                {
                  // 电机按预设运行方向 和 预设速度运行
                  CarRunStatus_obj.IsCarRunning = CarRunning;
                  vMotorOps(CarRunStatus_obj.SetDirection, CarRunStatus_obj.SetSpeed);  
                  GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                  DEBUGINFO("LED4 ON\r\n");
                }

              }
              // 远程手动模式
              else if(CarRunStatus_obj.AutoMode == Manual)
              {
                DEBUGINFO("remote Manual mode\r\n");
                // 电机按实际运行方向 和 普通速度运行（手动档下）
                CarRunStatus_obj.IsCarRunning = CarRunning;
                vMotorOps(CarToServerData_obj.ucDirection, NormalSpeed);  
                GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                DEBUGINFO("LED4 ON\r\n");
              }


            }
            // 拨动开关手动档
            else if(CarCheckFlag_obj.ToggleSwtichPosition == ToggleBack)
            {
              DEBUGINFO("local Manual mode\r\n");

              //reset按钮没有按下才允许电机运行
              if(GPIO_READ(RESET) == GPIO_PIN_SET)
              {
                // 电机按普通速度运行（手动档下），方向相反
                CarRunStatus_obj.IsCarRunning = CarRunning;
                vMotorOps(CarToServerData_obj.ucDirection, NormalSpeed); 
                GPIO_WRITE(LED4, GPIO_PIN_SET); // 打开LED4
                DEBUGINFO("LED4 ON\r\n");
              }

            }

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

      DEBUGINFO("Motor received len:%d\r\n",ucReciveLen);
      vPrint_Array(ucMotor_Task_Rx_Buffer, ucReciveLen);

      // 重启DMA接收(DMA循环模式下，重启后从缓冲区起始地址覆盖写入)
      vMotor_Start_DMA_Receive(ucMotor_Task_Rx_Buffer);
    }
  }
}
