#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Register.h"
#include "LogDebugInfo.h"
#include "adaptor_mqtt.h"
#include "adaptor_wifi.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "Robot.h"
#include "Encoder.h"
#include "semphr.h"
#include <limits.h>
#include "Register.h"
#include "StringEdit.h"
#include "RegisterInfo.h"

//处理注册消息，获取工程注册信息或已经获取到的注册信息,存入mqtt登录信息中
void vRegisterManagerTask(void *argument)
{
    uint32_t ulNotificationValue;
    BaseType_t xResult;      
    DEBUGINFO("vRegisterManagerTask\r\n");
    //测试用
    // bEraseRegisterArea();
    // memset(&g_register_info,0x0,sizeof(Stru_Field_Register_Typedef));
    // bReadFieldRegisterInfo(&g_register_info);    
    while(1)
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
      }
    }
}
//从服务器中获取注册信息（主要获取流水号）
void vRegisterHandleTask(void *argument)
{
    DEBUGINFO("vRegisterHandleTask\r\n");
    while(1)
    {
        osDelay(1000);
    }
}










