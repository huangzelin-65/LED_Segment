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
		if(tcp_msg == NULL)return;
        tcp_msg->type = type;
        tcp_msg->data = data;        
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(xTcpManageQueueHandle),uxQueueSpacesAvailable(xTcpManageQueueHandle));
        if (xQueueSend(xTcpManageQueueHandle, &tcp_msg, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("xTcpManageQueueHandle add success");
        } 
    }     
}
//当作为tcp服务端时，发送数据回客户端用此函数
uint32_t Tcp_ServerSend(struct tcp_pcb *tpcb, const void* buf, uint32_t len)
{
	uint32_t nwrite = 0, total = 0;
	const uint8_t* p = (const uint8_t *) buf;
	err_t err = ERR_OK;
	if (!tpcb)
		return 0;
	while ((err == ERR_OK) && (len != 0) && (tcp_sndbuf(tpcb) > 0))
	{
		nwrite = tcp_sndbuf(tpcb) >= len ? len : tcp_sndbuf(tpcb);
		err = tcp_write(tpcb, p, nwrite, 1);
		if (err == ERR_OK)
		{
			len -= nwrite;
			total += nwrite;
			p += nwrite;
		}
		tcp_output(tpcb);
	}
	return total;
}








