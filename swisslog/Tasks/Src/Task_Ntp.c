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

void vNtpManagerTask(void *argument)
{
    uint32_t ulNotificationValue;
    BaseType_t xResult;    
    DEBUGINFO("vNtpManagerTask\r\n");
    while (1)
    {
        // 等待通知，超时时间为永远等待
        // 清除方式: 收到通知后清除通知值
        xResult = xTaskNotifyWait(0x00,    // 进入函数前不清除任何位
                                    ULONG_MAX,// 退出函数时清除所有位
                                    &ulNotificationValue, 
                                    portMAX_DELAY);
        
        if (xResult == pdPASS) 
        {
            DEBUGINFO("ulNotificationValue:%lx\n",ulNotificationValue);
            if(ulNotificationValue & NTP_NOTIFY_INIT)
            {
                DEBUGINFO("NTP_NOTIFY_INIT\n");
                sntp_normal_init();
            }            
            if(ulNotificationValue & NTP_NOTIFY_UPDATE)
            {
                DEBUGINFO("NTP_NOTIFY_UPDATE\n");  
                struct tm current_time_val = sntp_get_system_time();
                Rtc_SetDate(current_time_val.tm_year,current_time_val.tm_mon,current_time_val.tm_mday,current_time_val.tm_wday); 
                Rtc_SetTime(current_time_val.tm_hour,current_time_val.tm_min,current_time_val.tm_sec);                
            }                             
        }  
    }
}



