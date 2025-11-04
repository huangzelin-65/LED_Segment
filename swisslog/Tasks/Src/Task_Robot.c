#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Wifi.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "adaptor_mqtt.h"
//处理robot相关任务，心跳包等
void vRobotManagerTask(void *argument)
{
    int heartbeat_cnt = 0;
    while (1)
    {
        if(mqtt_isConnected)
        {
            heartbeat_cnt++;
            if(heartbeat_cnt >= 50)//5秒发送一次心跳包
            {
                heartbeat_cnt = 0;
                Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,NULL);
            }
        }
        osDelay(pdMS_TO_TICKS(100));
    }
}
//处理事件发生时，更新robot相关的结构体和对应的json
void vRobotReceiveTask(void *argument)
{
  while (1)
  {
	osDelay(pdMS_TO_TICKS(100));
  }
}


