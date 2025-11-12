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
#include "semphr.h"
#include <limits.h>


extern osMessageQueueId_t xRobotQueueHandle;//该消息队列处理事件上报

//处理robot相关任务，心跳包等
void vRobotManagerTask(void *argument)
{
    int cnt = 0;
    Robot_Init();
    while (1)
    {
        if(mqtt_isConnected)
        {
            cnt++;
            if(cnt == 100)//10秒发送一次心跳包
            {
                DEBUGINFO("ROBOT_MSG_HEART\n");  
                Robot_SendMsg(ROBOT_MSG_HEART,NULL);
                cnt = 0;
            }
            if(cnt == 50)//这里只是模拟事件发生，测试代码
            {
                // DEBUGINFO("ROBOT_MSG_STATE\n");  
                // Robot_State(); 
            }                
        }
        osDelay(pdMS_TO_TICKS(100));
    }
}
//处理事件发生时，更新robot相关的结构体和对应的json
void vRobotReceiveTask(void *argument)
{ 
    RobotMsg_t * robot_msg = NULL;
    DEBUGINFO("xRobotQueueHandle:%p\n",xRobotQueueHandle); 
    while (1)
    {
        if(xQueueReceive(xRobotQueueHandle, &robot_msg, portMAX_DELAY) == pdTRUE)
        {
            if(robot_init)//保证robot json已经被创建
            {
                DEBUGINFO("type:%d\n",robot_msg->type); 
                switch (robot_msg->type)
                {
                    case ROBOT_MSG_HEART://发送心跳包到服务器
                    {
                        DEBUGINFO("ROBOT_MSG_HEART\n"); 
                        Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,Robot_GetHeartBeatJsonStr());
                    }
                    break;
                    case ROBOT_MSG_STATE://代表需要把消息发送到服务器
                    {
                        //此处需增加一个更新robot state 的接口
                        DEBUGINFO("ROBOT_MSG_STATE\n");  
                        //根据robot state更新对应的json字段
                        RobotState_t *robot_state_data = (RobotState_t *)robot_msg->data; 

                        Robot_UpdateStateJson(RobotJson,robot_state_data);
                        //发送消息给mqtt队列，让最新robot状态发布给服务器
                        Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,Robot_GetStateJsonStr());
                        //释放内存
                        vPortFree(robot_msg->data);
                    }
                    break;
                    case ROBOT_MSG_PARSE://代表从服务器获取到消息
                    {
                        DEBUGINFO("ROBOT_MSG_PARSE:%s\n",robot_msg->data); 
                        //解析来自mqtt的数据
                        int result = Robot_ParseJson(robot_msg->data,&robotAction);

                        DEBUGINFO("parse result:%d\n",result); 

                        //将解析后再到实际的控制接口
                        Robot_Action2Cmd();

                        vPortFree(robot_msg->data);

                        //需要回复服务器ack，在state中的actionstate回复状态

                        Robot_ActionAckUpdate(ROBOT_ACTION_STATUS_ACK);

                        Robot_State();
                    }
                    break;  
                    case ROBOT_MSG_ACTION_STATUS://返回action的状态，如running或finish
                    {
                        DEBUGINFO("ROBOT_MSG_ACTION_STATUS\n");
                        //此处需要更新robot实际运行状态
                        Robot_State();  
                    }
                    break;                                        
                    default:
                    break;
                }

            }
            vPortFree(robot_msg);
        }
    }
}



