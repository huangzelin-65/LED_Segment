#include "main.h"
#include "Task_rs485.h"
#include "LogDebugInfo.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "Task_LED_Segment.h"

extern void RS485_Init(void);

void vInitTask(void *argument)
{
	DEBUGINFO("InitTask");
	DEBUGINFO("Log system init success!");
	LED_Segment_Init();
	RS485_Init();
	osThreadExit();
}
