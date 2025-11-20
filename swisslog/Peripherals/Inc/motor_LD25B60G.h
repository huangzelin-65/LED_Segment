/*
 * motor_ctrl.h
 *
 *  Created on: Jul 11, 2025
 *      Author: e3lijia25d
 */

#ifndef PERIPHERALS_INC_MOTOR_CTRL_H_
#define PERIPHERALS_INC_MOTOR_CTRL_H_

#include "main.h"

//----------电机控制方向枚举----------
typedef enum
{
  MOTOR_STOP = 0,
  MOTOR_FORWARD = 1,
  MOTOR_BACKWARD = 2,
}eMotorDirection;

//----------电机控制速度枚举----------
typedef enum
{
  MOTOR_NO_SPEED = 0,
  MOTOR_SPEED_LOW = 1,
  MOTOR_SPEED_NORMAL = 2,
  MOTOR_SPEED_HIGH = 3,
}eMotorSpeed;

//----------电机信息查询枚举----------
typedef enum
{
  MOTOR_GET_SPEED = 0,
  MOTOR_GET_STATUS = 1,
  MOTOR_GET_ERROR = 2,
}eMotorQuery;

//----------电机设置选项枚举----------
typedef enum
{
  MOTOR_SETTING_ACCEL = 0,
  MOTOR_SETTING_DECEL = 1,
}eMotorSetting;

//----------电机设置参数枚举----------
typedef enum
{
  MOTOR_SETTING_ACCEL_10000 = 0,
  MOTOR_SETTING_ACCEL_8000,
  MOTOR_SETTING_ACCEL_5000,
  MOTOR_SETTING_ACCEL_3000,
  MOTOR_SETTING_ACCEL_1000,

  MOTOR_SETTING_DECEL_10000,
  MOTOR_SETTING_DECEL_8000,
  MOTOR_SETTING_DECEL_5000,
  MOTOR_SETTING_DECEL_3000,
  MOTOR_SETTING_DECEL_1000,
}eMotorSettingParam;

void vMotorOps(u8 Direction, u8 Speed);
void vMotorSetting(eMotorSetting xSetting, eMotorSettingParam xParam);
void vMotorRead(u8 Cmd);
void vMotorTest();

#endif /* PERIPHERALS_INC_MOTOR_CTRL_H_ */
