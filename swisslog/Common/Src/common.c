/*
 * common.c
 *
 *  Created on: Jun 27, 2025
 *      Author: e3lijia25d
 */

#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "common.h"
#include "motor_LD25B60G.h"
#include "LogDebugInfo.h"
#include "Task_BoxCtrl.h"



extern osMessageQueueId_t xMotion_QueueHandle;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;

extern ServerToCarData_t ServerToCarData;
extern CarToServerData_t CarToServerData;

extern CarStatus_t CarStatus;

void vParseCommandToCar()
{
  u8 ucMotion_msg;
  
  //*********************************************电机控制命令解析*************************************** 
  //************************************模式解析+方向解析*****************************
  // 拨动开关拨到自动档位下才能使用远程控制
  if(CarStatus.ToggleSwtichPosition == ToggleFront)
  {
    switch (ServerToCarData.xAutoMode)
    {
      // 远程手动模式
      case Manual:
        DEBUGINFO("remote manual mode\r\n");
        CarStatus.xAutoMode = Manual;
        CarStatus.xRealDirection = ServerToCarData.xDirection; // 手动模式下直接设置实际运行方向
        break;

      //小车远程自动模式 
      case Auto:
          DEBUGINFO("remote auto mode\r\n");
          CarStatus.xAutoMode = Auto;
          CarStatus.xSetDirection = ServerToCarData.xDirection; // 自动模式下设置预设方向
        break;

      default:
        break;
    }
  }

  
  //***********************************速度解析******************************
  // 服务器下发设置速度
  DEBUGINFO("SetSpeed: %d\r\n",ServerToCarData.xSetSpeed);
  CarStatus.xSetSpeed = ServerToCarData.xSetSpeed; //小车预设运行速度

  
  //***********************************电机使能解析******************************
  // 判断 电机使能运行
  switch (ServerToCarData.xMotorEnable)
  {
    case MotorDisable:
      // 停止电机
      CarStatus.xMotorEnable = MotorDisable;

      // 电机停止原因
      CarStatus.xMotorStopReason = ByCommand;
      DEBUGINFO("MotorStopReason: ByCommand\r\n");

      ucMotion_msg = CarStop;
      break;

    case MotorEnable:
      //车厢锁上且未到站才能发车
      if( CarStatus.xBoxLocked == Locked )
      {
        //启动电机
        CarStatus.xMotorEnable = MotorEnable;
        ucMotion_msg = CarRunning;
      } else {
        // 停止电机
        CarStatus.xMotorEnable = MotorDisable;

        // 电机停止原因
        if(CarStatus.xBoxLocked == UnLock){
          CarStatus.xMotorStopReason = ByBoxUnlock;
          DEBUGINFO("MotorStopReason: ByBoxUnlock\r\n");
        }
        
        ucMotion_msg = CarStop;
        DEBUGINFO("Can't Run!! check Box LockStatus !!\r\n");
      }
      break;
    
    default:
      break;
  }

  // 发送电机控制消息
  DEBUGINFO("send motion msg:%d\r\n",ucMotion_msg);
  if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
  {
    DEBUGINFO("send motion msg error\r\n");
  }


}


