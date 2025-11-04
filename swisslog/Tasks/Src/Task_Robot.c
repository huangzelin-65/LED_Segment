#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Robot.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "adaptor_mqtt.h"
#include "adaptor_robot.h"

extern osMessageQueueId_t xRobotQueueHandle;//该消息队列处理事件上报

//处理robot相关任务，心跳包等
void vRobotManagerTask(void *argument)
{
    int heartbeat_cnt = 0;
    Robot_Init();
    while (1)
    {
        if(mqtt_isConnected)
        {
            heartbeat_cnt++;
            if(heartbeat_cnt >= 50)//5秒发送一次心跳包
            {
                heartbeat_cnt = 0;
                DEBUGINFO("MQTT_MSG_HEARTBEAT\n");  
                Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,NULL);
            }
        }
        osDelay(pdMS_TO_TICKS(100));
    }
}
//处理事件发生时，更新robot相关的结构体和对应的json
void vRobotReceiveTask(void *argument)
{
    char *robot_data = NULL;
    while (1)
    {
        if(xQueueReceive(xRobotQueueHandle, &robot_data, portMAX_DELAY) == pdTRUE)
        {
            if(robot_init)//保证robot json已经被创建
            {
                //此处需增加一个更新robot state 的接口
                Robot_UpdateStateJson(RobotJson,&robotSate);//根据robot state更新对应的json字段
                //发送消息给mqtt队列，让最新robot状态发布给服务器
                Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,Robot_GetStateJsonStr());
            }
        }
    }
}



