#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_tcp.h"
#include "LogDebugInfo.h"
#include "queue.h"
#include "common.h"

extern osMessageQueueId_t xTcpManageQueueHandle;

//通知TCP任务处理对应信息
void Tcp_SendMsg(TcpMsgType_t type,char *data)
{
    if(xTcpManageQueueHandle != NULL)
    {
        TcpMsg_t * tcp_msg = pvPortMalloc(sizeof(TcpMsg_t));
        tcp_msg->type = type;
        tcp_msg->data = data;        
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(xTcpManageQueueHandle),uxQueueSpacesAvailable(xTcpManageQueueHandle));
        if (xQueueSend(xTcpManageQueueHandle, &tcp_msg, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("xTcpManageQueueHandle add success");
        } 
    }     
}










