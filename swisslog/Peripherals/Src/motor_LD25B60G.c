/*
 * motor_ctrl.c
 *
 *  Created on: Jul 11, 2025
 *      Author: e3lijia25d
 */

#include "main.h"
#include "motor_LD25B60G.h"
#include "LogDebugInfo.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "adaptor_motor.h"


extern CarToServerData CarToServerData_obj;

u8 MotorStopCmd[8]= {0x01,0x06,0x10,0x04,0x00,0x00,0xCC,0xCB}; //电机停止发送命令

u8 MotorCWCmd_100Speed[8]= {0x01,0x06,0x10,0x04,0x00,0x64,0xCD,0x20};//电机顺时针方向(在reset键那边看)100rpm速度发送命令
u8 MotorCWCmd_500Speed[8]= {0x01,0x06,0x10,0x04,0x01,0xF4,0xCC,0xDC};//电机顺时针方向(在reset键那边看)500rpm速度发送命令
u8 MotorCWCmd_1000Speed[8]= {0x01,0x06,0x10,0x04,0x03,0xE8,0xCC,0x75}; //电机顺时针方向(在reset键那边看)1000rpm速度发送命令

u8 MotorCCWCmd_100Speed[8]= {0x01,0x06,0x10,0x04,0xFF,0x9C,0x8D,0x52}; //电机逆时针方向(在reset键那边看)100rpm速度发送命令
u8 MotorCCWCmd_500Speed[8]= {0x01,0x06,0x10,0x04,0xFE,0x0C,0x8C,0xAE}; //电机逆时针方向(在reset键那边看)500rpm速度发送命令
u8 MotorCCWCmd_1000Speed[8]= {0x01,0x06,0x10,0x04,0xFC,0x18,0x8D,0xC1}; //电机逆时针方向(在reset键那边看)1000rpm速度发送命令

u8 MotorCmd_GetSpeed[8]= {0x01,0x03,0x11,0x66,0x00,0x01,0x61,0x29}; //读取平均速度命令
u8 MotorCmd_GetStatus[8]= {0x01,0x03,0x11,0x6E,0x00,0x01,0xE0,0xEB}; //读驱动器实时状态命令
u8 MotorCmd_GetErr[8]= {0x01,0x03,0x11,0x6C,0x00,0x01,0x41,0x2B}; //读驱动器故障码命令 



//电机控制函数
void vMotorOps(u8 Direction, u8 Speed)
{
	//判断方向
	switch (Direction)
	{
		case NoDirection:
			DEBUGINFO("MOTOR_STOP\r\n");
			vSendToMotor(MotorStopCmd,8);
			CarToServerData_obj.bSpdMode = ZeroSpeed; //实际速度记录为0
			break;

		case Forward:
			//CarToServerData_obj.ucDirection = MOTOR_FORWARD; //记录为正转
			//判断速度
			switch (Speed)
			{
				case LowSpeed:
					vSendToMotor(MotorCCWCmd_100Speed,8);
					DEBUGINFO("MOTOR_FORWARD, LowSpeed\r\n");
					CarToServerData_obj.bSpdMode = LowSpeed;
					break;
				case NormalSpeed:
					vSendToMotor(MotorCCWCmd_500Speed,8);
					DEBUGINFO("MOTOR_FORWARD, NormalSpeed\r\n");
					CarToServerData_obj.bSpdMode = NormalSpeed;
					break;
				case HighSpeed:
					vSendToMotor(MotorCCWCmd_1000Speed,8);
					DEBUGINFO("MOTOR_FORWARD, HighSpeed\r\n");
					CarToServerData_obj.bSpdMode = HighSpeed;
					break;
				default:
					break;
			}
			break;
		
		case Backward:
			//CarToServerData_obj.ucDirection = MOTOR_BACKWARD; //记录为反转
			//判断速度
			switch (Speed)
			{
				case LowSpeed:
					vSendToMotor(MotorCWCmd_100Speed,8);
					DEBUGINFO("MOTOR_BACKWARD, LowSpeed\r\n");
					CarToServerData_obj.bSpdMode = LowSpeed;
					break;
				case NormalSpeed:
					vSendToMotor(MotorCWCmd_500Speed,8);
					DEBUGINFO("MOTOR_BACKWARD, NormalSpeed\r\n");
					CarToServerData_obj.bSpdMode = NormalSpeed;
					break;
				case HighSpeed:
					vSendToMotor(MotorCWCmd_1000Speed,8);
					DEBUGINFO("MOTOR_BACKWARD, HighSpeed\r\n");
					CarToServerData_obj.bSpdMode = HighSpeed;
					break;
				default:
					break;
			}
			break;

		default:
			break;
	}
}

//电机状态读取函数
void vMotorRead(u8 Cmd)
{
	switch (Cmd)
	{
		//获取速度
		case MOTOR_GET_SPEED:
			DEBUGINFO("MOTOR_GET_SPEED\r\n");
			vSendToMotor(MotorCmd_GetSpeed,8);
			break;

		//获取状态
		case MOTOR_GET_STATUS:
			DEBUGINFO("MOTOR_GET_STATUS\r\n");
			vSendToMotor(MotorCmd_GetStatus,8);
			break;
		
		//获取故障码
		case MOTOR_GET_ERROR:
			DEBUGINFO("MOTOR_GET_ERROR\r\n");
			vSendToMotor(MotorCmd_GetErr,8);
			break;

		default:
			break;
	}
}



void vMotorTest()
{
	osDelay(1000);

  //电机正转
	vMotorOps(MOTOR_FORWARD, MOTOR_SPEED_LOW);
	osDelay(500);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(1000);
	vMotorOps(MOTOR_FORWARD, MOTOR_SPEED_NORMAL);
	osDelay(500);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(1000);
	vMotorOps(MOTOR_FORWARD, MOTOR_SPEED_HIGH);
	osDelay(500);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(1000);

	vMotorRead(MOTOR_GET_STATUS);
	osDelay(500);
	vMotorRead(MOTOR_GET_ERROR);
	osDelay(500);

  //电机反转 
	vMotorOps(MOTOR_BACKWARD, MOTOR_SPEED_LOW);
	osDelay(500);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(1000);
	vMotorOps(MOTOR_BACKWARD, MOTOR_SPEED_NORMAL);
	osDelay(500);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(1000);
	vMotorOps(MOTOR_BACKWARD, MOTOR_SPEED_HIGH);
	osDelay(500);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(1000);

	vMotorRead(MOTOR_GET_STATUS);
	osDelay(500);
	vMotorRead(MOTOR_GET_ERROR);
	osDelay(500);

  //电机停转 
	vMotorOps(MOTOR_STOP, MOTOR_NO_SPEED);
	osDelay(1000);
	vMotorRead(MOTOR_GET_SPEED);
	osDelay(500);
	vMotorRead(MOTOR_GET_STATUS);
	osDelay(500);
	vMotorRead(MOTOR_GET_ERROR);
	osDelay(500);
}
