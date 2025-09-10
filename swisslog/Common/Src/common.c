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



extern QueueHandle_t xMotion_QueueHandle;
extern PlcToCarData PlcToCarData_obj;
extern CarToPlcData CarToPlcData_obj;
extern _CarRunStatus_obj CarRunStatus_obj;
extern _CarCheckFlag_obj CarCheckFlagobj;

void vParseCommandToCar()
{
  u8 ucMotion_msg;

  //小车远程手动模式(拨动开关需要在自动档位下才能使用)
  if(PlcToCarData_obj.wCtrl & 0x0001)
  {
    DEBUGINFO_ALL("manual mode\r\n");
    CarRunStatus_obj.AutoMode = Manual;

    //0x0002前进，0x0004后退
    if(PlcToCarData_obj.wCtrl & 0x0002)
    {
      // 手动设置前进(正转)
      DEBUGINFO_ALL("manual Forward\r\n");
    }
    else if(PlcToCarData_obj.wCtrl & 0x0004)
    {
      // 手动设置后退(反转)
      DEBUGINFO_ALL("manual Backward\r\n");
      CarToPlcData_obj.bDire = Backward; //手动模式下直接设置实际运行方向为后退
    }
  }

  //小车远程自动模式 (拨动开关需要在自动档位下才能使用)
  else if((PlcToCarData_obj.wCtrl & 0x0010) \
    && (CarCheckFlagobj.ToggleSwtichPosition == ToggleFront))
  {
    DEBUGINFO_ALL("auto mode\r\n");
    CarRunStatus_obj.AutoMode = Auto;

    // PLC设置前进(正转)
    if(PlcToCarData_obj.bDire == Forward)
    {
      DEBUGINFO_ALL("auto Forward\r\n");
      CarRunStatus_obj.SetDirection = Forward; //小车预设运行方向为前进
    }
    // PLC设置后退(反转)
    else if(PlcToCarData_obj.bDire == Backward)
    {
      DEBUGINFO_ALL("auto Backward\r\n");
      CarRunStatus_obj.SetDirection = Backward; //小车预设运行方向为后退
    }
  }
  // 无效命令
  else
  {
    DEBUGINFO_ALL("Invalid commond\r\n");
  }
  
  //判断启动/停止电机
  if(PlcToCarData_obj.wCtrl & 0x0008) //使能运行位为1
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
  if(xQueueSend(xMotion_QueueHandle, &ucMotion_msg, pdMS_TO_TICKS(100)) != pdPASS)
  {
    DEBUGINFO_ALL("vParseCommandToCar() send motion msg error\r\n");
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


