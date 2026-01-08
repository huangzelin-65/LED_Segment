#include "main.h"
#include "Task_rs485.h"
#include "LogDebugInfo.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"


void vInitTask(void *argument)
{
	DEBUGINFO("InitTask");
	DEBUGINFO("Log system init success!");
	RS485_Interrupt_Receive_Init();
	osThreadExit();
}
