#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Tcp.h"
#include "LogDebugInfo.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "lwip.h"
#include "adaptor_tcp.h"
#include "adaptor_mqtt.h"

extern osMessageQueueId_t xTcpManageQueueHandle;

/* TCP管理任务入口函数 */
void vTcpManagerTask(void *argument)
{
  TcpMsg_t *tcp_msg = NULL;
  DEBUGINFO("vTcpManagerTask\r\n");
  MX_LWIP_Init();
  while (1)
  {
    if(xQueueReceive(xTcpManageQueueHandle, &tcp_msg, portMAX_DELAY) == pdTRUE)
    {
        DEBUGINFO("type:%d\n",tcp_msg->type); 
        switch (tcp_msg->type)
        {
            case TCP_MSG_MQTT://启动mqtt服务
            {
                DEBUGINFO("TCP_MSG_MQTT\n"); 
                #ifdef MQTT_WIFI
                    DEBUGINFO("MQTT START BY WIFI\n"); 
                #else
                    Mqtt_SendMsg(MQTT_MSG_START,NULL);
                #endif
            }
            break;
            case TCP_MSG_SERVER://启动tcp客户端
            {
                DEBUGINFO("TCP_MSG_SERVER\n");                      
            }
            break;                                       
            default:
            break;
        }
        vPortFree(tcp_msg);
    }
  }
}
    

/* tcp接收任务入口函数,后续做为server使用，接收数据，实现ota*/
void vTcpReceiveTask(void *argument)
{
  DEBUGINFO("vTcpReceiveTask\r\n");
  while (1)
  {
    osDelay(pdMS_TO_TICKS(100));
  } 
}


