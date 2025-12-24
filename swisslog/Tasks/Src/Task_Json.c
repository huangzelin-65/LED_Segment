#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Tcp.h"
#include "LogDebugInfo.h"
// #include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "lwip.h"
#include "adaptor_mqtt.h"
#include "adaptor_ntp.h"
#include "app_freertos.h"
#include "adaptor_rtc.h"

void vJsonGenerateTask(void *argument)
{
    DEBUGINFO("vJsonGenerateTask\n");
    while (1)
    {
        osDelay(1000);
    }
}

void vJsonParseTask(void *argument)
{
    DEBUGINFO("vJsonParseTask\n");
    while (1)
    {
        osDelay(1000);
    }
}



