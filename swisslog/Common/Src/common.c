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



extern osMessageQueueId_t xMotion_QueueHandle;
extern ServerToCarData ServerToCarData_obj;
extern CarToServerData CarToServerData_obj;
extern _CarRunStatus_obj CarRunStatus_obj;
extern _CarCheckFlag_obj CarCheckFlag_obj;

void vParseCommandToCar()
{
  u8 ucMotion_msg;
  
  // PLC下发设置前进(正转)
  if(ServerToCarData_obj.ucDirection == Forward)
  {
    DEBUGINFO("auto Forward\r\n");
    CarRunStatus_obj.SetDirection = Forward; //小车预设运行方向为前进
  }
  // PLC下发设置后退(反转)
  else if(ServerToCarData_obj.ucDirection == Backward)
  {
    DEBUGINFO("auto Backward\r\n");
    CarRunStatus_obj.SetDirection = Backward; //小车预设运行方向为后退
  }

  //小车远程手动模式(拨动开关需要在自动档位下才能使用)
  if(ServerToCarData_obj.wCtrl & 0x0001)
  {
    DEBUGINFO("manual mode\r\n");
    CarRunStatus_obj.AutoMode = Manual;

    //0x0002前进，0x0004后退
    if(ServerToCarData_obj.wCtrl & 0x0002)
    {
      // 手动设置前进(正转)
      DEBUGINFO("manual Forward\r\n");
      CarToServerData_obj.ucDirection = Forward;
    }
    else if(ServerToCarData_obj.wCtrl & 0x0004)
    {
      // 手动设置后退(反转)
      DEBUGINFO("manual Backward\r\n");
      CarToServerData_obj.ucDirection = Backward; //手动模式下直接设置实际运行方向为后退
    }
  }

  //小车远程自动模式 (拨动开关需要在自动档位下才能使用)
  else if((ServerToCarData_obj.wCtrl & 0x0010) \
    && (CarCheckFlag_obj.ToggleSwtichPosition == ToggleFront))
  {
    DEBUGINFO("auto mode\r\n");
    CarRunStatus_obj.AutoMode = Auto;
  }
  // 无效命令
  // else
  // {
  //   DEBUGINFO("Invalid commond\r\n");
  // }
  
  //判断启动/停止电机
  if(ServerToCarData_obj.wCtrl & 0x0008) //使能运行位为1
  {
    //启动电机
    CarRunStatus_obj.MotorEnable = MotorEnable;
    ucMotion_msg = CarRunning;
  }
  else
  {
    // 停止电机
    CarRunStatus_obj.MotorEnable = MotorDisable;
    ucMotion_msg = CarStop;
  }

  // 发送电机控制消息
  if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
  {
    DEBUGINFO("vParseCommandToCar() send motion msg error\r\n");
  }

}

/**
 * 将字符串中指定开始位置和长度的子串转换为无符号整数
 */
u32 substring_to_uint(char* str, u16 start, u16 length) 
{
  u32 value = 0;
  u32 digit = 0;
  // 检查参数合法性
  if (str == NULL) {
      return CONVERT_NULL_PTR;
  }
  
  size_t str_len = strlen(str);
  if (start >= str_len) {
      return CONVERT_INVALID_START;
  }
  
  if (length == 0 || start + length > str_len) {
      return CONVERT_INVALID_LENGTH;
  }
  
  // 检查所有字符是否都是数字
  for (size_t i = 0; i < length; i++) {
      if (str[start + i] < '0' || str[start + i] > '9') {
          return CONVERT_NON_DIGIT;
      }
  }
  
  // 转换为无符号整数
  for (size_t i = 0; i < length; i++) {
      // 检查溢出
      if (value > UINT32_MAX / 10) {
          return CONVERT_OVERFLOW;
      }
      value *= 10;
      
      digit = str[start + i] - '0';
      if (value > UINT32_MAX - digit) {
          return CONVERT_OVERFLOW;
      }
      value += digit;
  }
  
  return value;
}


