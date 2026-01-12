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
    DEBUGINFO("vRegisterManagerTask\r\n");
    //测试用
    // bEraseRegisterArea();
    // memset(&g_register_info,0x0,sizeof(Stru_Field_Register_Typedef));
    // bReadFieldRegisterInfo(&g_register_info);    
    while(1)
    {
        osDelay(1000);
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










