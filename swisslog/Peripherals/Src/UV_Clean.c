/*
* Swisslog HCS
* FileName：		UVClean.c
* Description：	Uv 消毒控制
*/
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "timers.h"
#include "Calculate.h"
#include "NumDisplay.h"
#include "adaptor_eeprom.h"
#include "adaptor_box.h"
#include "UV_Clean.h"
#include "LogDebugInfo.h"

extern CarStatus_t CarStatus;
extern osTimerId_t xUVTimerHandle;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;

static uint8_t u8_uvCleanRunning=0;
//static uint8_t UvTimeCnt=0;
static uint8_t asu8_cleanTime[6];
static uint8_t su8_cleanTime = 0;

/**
是否正在消毒
*/
uint8_t u8_UvClean_IsRunning(void)
{
	return u8_uvCleanRunning;
}

/*
开始消毒
**/
void v_UvClean_Start(uint8_t u8_timeMin,uint8_t au8_startTime[6])
{
	osDelay(pdMS_TO_TICKS(200));
	if(u8_timeMin == 0)
		u8_timeMin = 5;
	if(u8_uvCleanRunning == 0){
	 	memcpy(asu8_cleanTime,au8_startTime,6);
 		su8_cleanTime = u8_timeMin;
		CarStatus.u8_uvTimeRemain = u8_timeMin; // 更新消毒剩余时间
		u8_uvCleanRunning = 1;
		CarStatus.u8_uvCleanRunning = 1;
		osTimerStart(xUVTimerHandle,pdMS_TO_TICKS(1000*60));
		//HMI_Check_Uv_Clean(UvTimeCnt);
	}
	v_UVClean_Enable();
	Robot_Event(); //上报小车状态
}

/**
停止消毒
*/
void v_UvClean_Stop(void)
{
	u8_uvCleanRunning = 0;
	CarStatus.u8_uvCleanRunning = 0;
    osTimerStop(xUVTimerHandle);
	v_UVClean_Disable();
	Robot_Event(); //上报小车状态
}

// 保存本次消毒开始的rtc时间 + 消毒时长
void v_UvClean_Save_Record(void){
	uint8_t temp[7];
	if(su8_cleanTime < CarStatus.u8_uvTimeRemain) return;
	memcpy(temp,asu8_cleanTime,6);
	temp[6] = su8_cleanTime - CarStatus.u8_uvTimeRemain;
	b_Eeprom_Check_Conn();
	b_Eeprom_Write_Buf(EEP_ADD_LAST_UVCLEAN_DATE,temp,7);
}

// 获取上次消毒rtc时间 + 消毒时长
void v_UvClean_Get_Record(uint8_t time[7]){
	uint8_t temp[7];
	b_Eeprom_Check_Conn();
	b_Eeprom_Read_Buf(EEP_ADD_LAST_UVCLEAN_DATE,temp,7);
	memcpy(time,temp,7);	
}

// 设置默认消毒时长(更新eeprom里的消毒时长设置 和 CarStatus里的消毒时长设置)
bool b_UvClean_Set_Duration(uint8_t u8_Duration){
	if(b_Eeprom_Check_Conn()== false)
		return false;
	b_Eeprom_Write_Byte(EEP_ADD_UVCLEAN_TIME_MINUTES,u8_Duration); // 更新eeprom里的默认消毒时长
	HMI_Update_DefaultUVTime_Req(u8_Duration); // 更新默认消毒时长到HMI
	CarStatus.u8_uvDuration = u8_Duration;
	return true;
}

// 获取默认消毒时长
uint8_t u8_UvClean_Get_Duration(void){
	uint8_t u8_duration;
	if(b_Eeprom_Check_Conn()== false)
		return 0;
	b_Eeprom_Read_Byte(EEP_ADD_UVCLEAN_TIME_MINUTES,&u8_duration);
	return u8_duration;
}

/*
消毒定时器任务
*/
void v_UvClean_TimerCallback(void *argument) 
{
	eBoxCtrlType x_BoxMsg;
	
	CarStatus.u8_uvTimeRemain = (CarStatus.u8_uvTimeRemain - 1 > 0) ? (CarStatus.u8_uvTimeRemain - 1) : 0; // 更新消毒剩余时间

	if(CarStatus.u8_uvTimeRemain>0){
		osTimerStart(xUVTimerHandle,pdMS_TO_TICKS(1000*60));
	}else{
		u8_uvCleanRunning = 0;
		CarStatus.u8_uvCleanRunning = 0;
		v_UVClean_Disable();
	}
	
	x_BoxMsg = UpdateUVCleanStatus;
	if(osMessageQueuePut(xBox_Ctrl_QueueHandle, &x_BoxMsg, 0, 0)!= osOK) {
		DEBUGINFO("Uv clean msg send failed!");
	}
}
