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

void vMotorOps(u8 Direction, u8 Speed);
void vMotorRead(u8 Cmd);
void vMotorTest();

#endif /* PERIPHERALS_INC_MOTOR_CTRL_H_ */
