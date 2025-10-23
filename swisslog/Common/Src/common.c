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
  
  //*******************************电机控制命令解析****************************** 
  //**********************方向解析********************
  // PLC下发设置前进(正转)
  if(ServerToCarData.ucDirection == Forward)
  {
    DEBUGINFO("auto Forward\r\n");
    CarStatus.xSetDirection = Forward; //小车预设运行方向为前进
  }
  // PLC下发设置后退(反转)
  else if(ServerToCarData.ucDirection == Backward)
  {
    DEBUGINFO("auto Backward\r\n");
    CarStatus.xSetDirection = Backward; //小车预设运行方向为后退
  }


  //***********************动作解析*********************
  //小车远程手动模式(拨动开关需要在自动档位下才能使用)
  if(ServerToCarData.wCtrl & 0x01)
  {
    DEBUGINFO("manual mode\r\n");
    CarStatus.xAutoMode = Manual;

    //0x0002前进，0x0004后退
    if(ServerToCarData.wCtrl & 0x02)
    {
      // 手动设置前进(正转)
      DEBUGINFO("manual Forward\r\n");
      CarStatus.xRealDirection = Forward;
    }
    else if(ServerToCarData.wCtrl & 0x04)
    {
      // 手动设置后退(反转)
      DEBUGINFO("manual Backward\r\n");
      CarStatus.xRealDirection = Backward; //手动模式下直接设置实际运行方向为后退
    }
  }

  //小车远程自动模式 (拨动开关需要在自动档位下才能使用)
  else if ((ServerToCarData.wCtrl & 0x10) && 
    (CarStatus.ToggleSwtichPosition == ToggleFront))
  {
    DEBUGINFO("auto mode\r\n");
    CarStatus.xAutoMode = Auto;
  }

  
  //判断启动/停止电机
  if( ServerToCarData.wCtrl & 0x08 ) //使能运行位为1
  {
    //车厢锁上且未到站才能发车
    if( CarStatus.xBoxLocked == Locked && ServerToCarData.xStationStatus == OutStation )
    {
      //启动电机
      CarStatus.xMotorEnable = MotorEnable;
      ucMotion_msg = CarRunning;
    } else {
      // 停止电机
      CarStatus.xMotorEnable = MotorDisable;
      ucMotion_msg = CarStop;
      DEBUGINFO("Can't Run!! check Box LockStatus or StationStatus!!\r\n");
    }
  }
  else //使能运行位为0
  {
    // 停止电机
    CarStatus.xMotorEnable = MotorDisable;
    ucMotion_msg = CarStop;
  }

  // 发送电机控制消息
  if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
  {
    DEBUGINFO("send motion msg error\r\n");
  }


  //*********************************小车到站状态解析**********************************
  if(ServerToCarData.xStationStatus != Car_Get_Station_Status())
  {
    // 发送进出站消息
    eBoxCtrlType box_msg = UpdateStationStatus;
    if(osMessageQueuePut(xBox_Ctrl_QueueHandle, &box_msg, 0, pdMS_TO_TICKS(100)) != osOK)
    {
      DEBUGINFO("send motion msg error\r\n");
    }
  }

}


