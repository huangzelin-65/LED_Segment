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

static uint8_t uvCleanRunning=0;
//static uint8_t UvTimeCnt=0;
static uint8_t saveStartTime[6];
static uint8_t cleanTime = 0;

/**
是否正在消毒
*/
uint8_t UvClean_IsRunning(void)
{
	return uvCleanRunning;
}

/*
开始消毒
**/
void UvClean_Start(uint8_t timeMin,uint8_t startTime[6])
{
	osDelay(pdMS_TO_TICKS(200));
	if(timeMin == 0)
		timeMin = 5;
	if(uvCleanRunning==0){
	 	memcpy(saveStartTime,startTime,6);
 		cleanTime = timeMin;
		CarStatus.ucUVTimeRemain = timeMin;
		uvCleanRunning=1;
		osTimerStart(xUVTimerHandle,pdMS_TO_TICKS(1000*60));
		//HMI_Check_Uv_Clean(UvTimeCnt);
	}
	vUV_Clean_enable();
}

/**
停止消毒
*/
void UvClean_Stop(void)
{
	uvCleanRunning=0;
    osTimerStop(xUVTimerHandle);
	vUV_Clean_disable();
}

// 保存本次消毒开始的rtc时间 + 消毒时长
void UvClean_Save_Record(void){
	uint8_t temp[7];
	if(cleanTime < CarStatus.ucUVTimeRemain) return;
	memcpy(temp,saveStartTime,6);
	temp[6] = cleanTime - CarStatus.ucUVTimeRemain;
	bEeprom_Check_Conn();
	bEeprom_Write_Buf(EEP_ADD_LAST_UVCLEAN_DATE,temp,7);
}

// 获取上次消毒rtc时间 + 消毒时长
void UvClean_Get_Record(uint8_t time[7]){
	uint8_t temp[7];
	bEeprom_Check_Conn();
	bEeprom_Read_Buf(EEP_ADD_LAST_UVCLEAN_DATE,temp,7);
	memcpy(time,temp,7);	
}

/*
消毒定时器任务
*/
// void vUVTimerCallback(void *argument) {
// 	uint8_t UvTimeCntTmp;
	
// 	UvTimeCntTmp = (UvTimeCnt > 0) ? (UvTimeCnt-1) : 0;

// 	if(UvTimeCntTmp>0){
// 		osTimerStart(xUVTimerHandle,pdMS_TO_TICKS(1000*60));
// 	}else{
// 		uvCleanRunning=0;
// 		vUV_Clean_disable();
// 	}
	
// 	if(osMessageQueuePut(xUV_QueueHandle, &UvTimeCntTmp, 0, 0)!= osOK) {
// 		DEBUGINFO("Uv clean msg send failed!");
// 		osTimerStart(xUVTimerHandle,1);
// 		return;
// 	}

// 	UvTimeCnt = UvTimeCntTmp;
// }
void vUVTimerCallback(void *argument) 
{
	eBoxCtrlType box_msg;
	
	CarStatus.ucUVTimeRemain = (CarStatus.ucUVTimeRemain - 1 > 0) ? (CarStatus.ucUVTimeRemain - 1) : 0;

	if(CarStatus.ucUVTimeRemain>0){
		osTimerStart(xUVTimerHandle,pdMS_TO_TICKS(1000*60));
	}else{
		uvCleanRunning = 0;
		vUV_Clean_disable();
	}
	
	box_msg = UpdateUVCleanStatus;
	if(osMessageQueuePut(xBox_Ctrl_QueueHandle, &box_msg, 0, 0)!= osOK) {
		DEBUGINFO("Uv clean msg send failed!");
	}
}
