#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_register.h"
#include "LogDebugInfo.h"
#include "queue.h"
#include "app_freertos.h"

RegisterState_t g_register_state;

void Register_Init(void)
{
	memset(&g_register_state,0,sizeof(RegisterState_t));
}

//消息通知
void Register_Notify(uint32_t value)
{
    if(RegisterManagerTaskHandle != NULL)
    {
        DEBUGINFO("value:%lx",value);
        BaseType_t xReturn = pdPASS;
        xReturn = xTaskNotify(RegisterManagerTaskHandle, 
                    value, 
                    eSetValueWithoutOverwrite);
        if(xReturn != pdPASS)
        {
            DEBUGINFO("xReturn is not pdPASS:%ld\n",xReturn);
        } 
    }
    else
    {
        DEBUGINFO("RegisterManagerTaskHandle NULL");
    }
}



