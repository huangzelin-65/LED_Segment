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
#include "Robot.h"
#include "semphr.h"
#include <limits.h>
#include "app_freertos.h"
// #define   TEST_MONITOR
extern CarStatus_t CarStatus;
extern osMessageQueueId_t xRobotQueueHandle;//该消息队列处理事件上报

#ifdef TEST_MONITOR
int test_cnt = 0;
#endif
//处理robot相关任务，心跳包等
void vRobotManagerTask(void *argument)
{
    uint32_t * ntf_value = NULL;     
    Robot_Init();
    DEBUGINFO("vRobotManagerTask\n"); 
    while (1)
    { 
        if(xQueueReceive(xRobotNotifyQueueHandle, &ntf_value, portMAX_DELAY) == pdTRUE)
        {
            DEBUGINFO("ntf_value:%lx\n",*ntf_value);
            if(*ntf_value & ROBOT_ACTIONACK)
            {
                DEBUGINFO("ROBOT_ACTIONACK\n");  
                //需要回复服务器ack，在state中的actionstate回复状态
                Robot_ActionAck();
                #ifdef TEST_MONITOR
                test_cnt = 6;
                #endif
            } 
            if(*ntf_value & ROBOT_ACTIONCMD)
            {
                DEBUGINFO("ROBOT_ACTIONCMD\n");  
                //将解析后再到实际的控制接口
                Robot_Action2Cmd();
            }          
            if(*ntf_value & ROBOT_HEARTBEAT)
            {
                DEBUGINFO("ROBOT_HEARTBEAT\n");  
                Robot_SendMsg(ROBOT_MSG_HEART,NULL);            
            }
            if(*ntf_value & ROBOT_ACTIONNTF)
            {
                DEBUGINFO("ROBOT_ACTIONNTF\n");  
                Robot_ActionNotify();              
            }            
            vPortFree(ntf_value);
        }          
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
                        DEBUGINFO("ROBOT_MSG_HEART start\n"); 
                        Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,Robot_GetHeartBeatJsonStr());
                        DEBUGINFO("ROBOT_MSG_HEART end\n"); 
                    }
                    break;
                    case ROBOT_MSG_STATE://代表需要把消息发送到服务器
                    {
                        //此处需增加一个更新robot state 的接口
                        DEBUGINFO("ROBOT_MSG_STATE start\n");  
                        //根据robot state更新对应的json字段
                        RobotState_t *robot_state_data = (RobotState_t *)robot_msg->data; 

                        Robot_UpdateStateJson(RobotJson,robot_state_data);
                        //发送消息给mqtt队列，让最新robot状态发布给服务器
                        char* json_str = Robot_GetStateJsonStr();
                        if(json_str != NULL)
                        {
                            Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,json_str);
                        }
                        else
                        {
                            DEBUGINFO("Robot_GetStateJsonStr fail\n");  
                            DEBUGINFO("xPortGetFreeHeapSize %u bytes\r\n", xPortGetFreeHeapSize()); 
                        }
                        //释放内存
                        vPortFree(robot_msg->data);

                        DEBUGINFO("ROBOT_MSG_STATE end\n"); 
                    }
                    break;
                    case ROBOT_MSG_PARSE://代表从服务器获取到消息
                    {
                        MqttRcMsg_t *mqtt_msg = (MqttRcMsg_t *)robot_msg->data;
                        DEBUGINFO("ROBOT_MSG_PARSE start:%s\n",mqtt_msg->data); 
                        //解析来自mqtt的数据
                        Robot_ParseJson(mqtt_msg->topic_name,mqtt_msg->data);

                        vPortFree(mqtt_msg->topic_name);

                        vPortFree(mqtt_msg->data);

                        vPortFree(mqtt_msg);
                        
                        DEBUGINFO("ROBOT_MSG_PARSE end\n"); 
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

void vRobotHeartBeatTask(void *argument)
{      
    DEBUGINFO("vRobotHeartBeatTask\n"); 
    while (1)
    {
        if(mqtt_isConnected)
        {
            if(robot_init)
            {
                #ifdef TEST_MONITOR
                if(test_cnt)
                {
                    test_cnt--;
                    DEBUGINFO("test_cnt:%d\n",test_cnt); 
                    if(test_cnt == 5)
                    {
                        Robot_ActionAckUpdate(ROBOT_ACTION_STATUS_RUNNING);
                        Robot_State();
                    }
                    if(test_cnt == 1)
                    {
                        Robot_ActionAckUpdate(ROBOT_ACTION_STATUS_FINISHED);
                        Robot_State();
                    }
                }
                #endif
                // DEBUGINFO("heartbeat_cnt:%d\n",robotSate.heartbeat_cnt); 
                if(robotSate.heartbeat_cnt++ >= 10)//10秒一次心跳
                {
                    DEBUGINFO("heartbeat\n");
                    robotSate.heartbeat_cnt = 0;
                    Robot_Notify(ROBOT_HEARTBEAT);
                }
            }
        }
        static int cnt = 0;
        if(cnt++ >= 10)
        {
            cnt = 0;
            DEBUGINFO("xPortGetFreeHeapSize %u bytes\r\n", xPortGetFreeHeapSize());
            DEBUGINFO("robotSate.encode_number: %d", robotSate.encode_number);
        }
        osDelay(1000);
    }    
}

