#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Mqtt.h"
#include "LogDebugInfo.h"
#include "adaptor_mqtt.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "Robot.h"
#include "Encoder.h"
#include "semphr.h"
#include <limits.h>

#define MQTT_TOPIC_NAME                 "tk/v1/slhc/tkv-%d/state" 
#define MQTT_HEARTBEAT_TOPIC_NAME       "tk/v1/slhc/tkv-%d/connection" 
#define MQTT_FACTSHEET_TOPIC_NAME       "tk/v1/slhc/tkv-%d/factsheet" 
#define MQTT_PUBLISH_MSG                "HEARTBEAT"
#define MQTT_CMD_TIMEOUT_MS             30000

extern CarStatus_t CarStatus;
extern osMessageQueueId_t xMqttManagerQueueHandle;

int MqttReadReady = 0;

//mqtt主任务，处理初始化，发送消息等
void vMqttManagerTask(void *argument)
{
    DEBUGINFO("vMqttManagerTask\r\n");
    Mqtt_ListInit();
    char *manage_data = NULL;
    while (1)
    {
        if(xQueueReceive(xMqttManagerQueueHandle, &manage_data, portMAX_DELAY) == pdTRUE)
        {
            MqttMsgdata_t *msg = (MqttMsgdata_t *)manage_data;
            DEBUGINFO("msg type:%d\n",msg->type);
            switch(msg->type)
            {
                case MQTT_MSG_START:
                {
                    int rc = MqttInit(robotSate.client_id);
                    if(rc == MQTT_CODE_SUCCESS)
                    {
                        mqtt_isConnected = 1;
                        DEBUGINFO("mqtt_isConnected");
                        //服务器连接成功，需要订阅话题
                        Mqtt_Notify(MQTT_NOTIFY_SUBSCRIBE);
                    }
                }
                break;
                case MQTT_MSG_HEARTBEAT:
                {
                    DEBUGINFO("MQTT_MSG_HEARTBEAT start\n");
                    char* robot_json_str = (char*)msg->data;
                    char topic[64] = {0};
                    snprintf(topic, sizeof(topic), MQTT_HEARTBEAT_TOPIC_NAME, robotSate.encode_number);                  
                    if(mqtt_isConnected)Mqtt_PublishMsg(topic, robot_json_str, XSTRLEN(robot_json_str), 0, 0);
                    vPortFree(robot_json_str);
                    // DEBUGINFO("MQTT_MSG_HEARTBEAT end\n");
                }
                break;
                case MQTT_MSG_ROBOT_EVENT:
                {
                    DEBUGINFO("MQTT_MSG_ROBOT_EVENT start\n");
                    char* robot_json_str = (char*)msg->data;
                    if(robot_json_str != NULL)
                    {
                        DEBUGINFO("robot_json_str:%s\n",robot_json_str);
                        char topic[64] = {0};
                        snprintf(topic, sizeof(topic), MQTT_TOPIC_NAME, robotSate.encode_number);
                        if(mqtt_isConnected)Mqtt_PublishMsg(topic, robot_json_str, XSTRLEN(robot_json_str), 1, 0);
                        vPortFree(robot_json_str);
                    }
                    DEBUGINFO("MQTT_MSG_ROBOT_EVENT end\n");
                }
                break;
                case MQTT_MSG_SUBSCRIBE:
                {
                    int rc = Mqtt_SubscribeTopicInit(robotSate.encode_number);
                    if (rc != MQTT_CODE_SUCCESS) {
                        DEBUGINFO("Mqtt_SubscribeTopicInit fail");
                        //订阅话题失败，尝试再次订阅
                        Mqtt_Notify(MQTT_NOTIFY_SUBSCRIBE);                        
                    }  
                    else
                    {
                        DEBUGINFO("Mqtt_SubscribeTopicInit success,start online");
                        Mqtt_Notify(MQTT_NOTIFY_ONLINE);    
                    }                                       
                }
                break; 
                case MQTT_MSG_ONLINE:
                {
                    DEBUGINFO("MQTT_MSG_ONLINE start\n");
                    //此处需修改为online的具体内容
                    char* robot_json_str = (char*)msg->data;
                    char topic[64] = {0};
                    snprintf(topic, sizeof(topic), MQTT_FACTSHEET_TOPIC_NAME, robotSate.encode_number);                     
                    if(mqtt_isConnected)Mqtt_PublishMsg(topic, robot_json_str, XSTRLEN(robot_json_str), 0, 0);
                    vPortFree(robot_json_str);

                    MqttReadReady = 1;//发布话题后既可正常等待话题
                    DEBUGINFO("MQTT_MSG_ONLINE end\n");
                }
                break;
                case MQTT_MSG_OFFLINE:
                {
                    DEBUGINFO("MQTT_MSG_OFFLINE start\n");
                    //此处需修改为online的具体内容
                    char* robot_json_str = (char*)msg->data;
                    char topic[64] = {0};
                    snprintf(topic, sizeof(topic), MQTT_FACTSHEET_TOPIC_NAME, robotSate.encode_number);                     
                    if(mqtt_isConnected)Mqtt_PublishMsg(topic, robot_json_str, XSTRLEN(robot_json_str), 0, 0);
                    vPortFree(robot_json_str);

                    MqttReadReady = 1;//发布话题后既可正常等待话题
                    DEBUGINFO("MQTT_MSG_OFFLINE end\n");
                }
                break;                                                
                default:break;
            }
            vPortFree(manage_data);
        }
    }
}
//mqtt接收任务，处理接收消息
void vMqttReceiveTask(void *argument)
{
    MqttObject mqttObj;
    XMEMSET(&mqttObj, 0, sizeof(mqttObj));
    DEBUGINFO("vMqttReceiveTask\r\n");
    int rc = 0;
    while (1)
    {
        if(MqttReadReady)
        {
            rc = MqttClient_WaitMessage_ex(&mClient, &mqttObj, MQTT_CMD_TIMEOUT_MS);
            if (rc == MQTT_CODE_ERROR_TIMEOUT) {
                rc = MqttClient_Ping_ex(&mClient, &mqttObj.ping);
                if (rc != MQTT_CODE_SUCCESS) {
                    DEBUGINFO("MqttClient_Ping_ex fail");
                }
                else
                {
                    DEBUGINFO("MQTT Keep-Alive Ping");
                } 
            }
            else if (rc != MQTT_CODE_SUCCESS) {
                DEBUGINFO("MqttClient_WaitMessage_ex:%d",rc);
            }
        }
        if(!MqttReadReady)osDelay(pdMS_TO_TICKS(100));
    }
}

//处理mqtt消息
void vMqttNotifyTask(void *argument)
{
    uint32_t ulNotificationValue;
    BaseType_t xResult;     
    DEBUGINFO("vMqttNotifyTask\n"); 
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
          if(ulNotificationValue & MQTT_NOTIFY_SUBSCRIBE)
          {
            DEBUGINFO("MQTT_NOTIFY_SUBSCRIBE\n");  
            Mqtt_SendMsg(MQTT_MSG_SUBSCRIBE,NULL);
          } 
          if(ulNotificationValue & MQTT_NOTIFY_ONLINE)
          {
            DEBUGINFO("MQTT_NOTIFY_ONLINE\n");
            char src[] = "ONLINE";
            memcpy(robotOnOffLine.onoffline,src,strlen(src) + 1);
            Robot_UpdateOnOffLineJson(Robot_OnOffLineJson,&robotOnOffLine); 
            Mqtt_SendMsg(MQTT_MSG_ONLINE,Robot_GetOnOffLineJsonStr());
          }          
          if(ulNotificationValue & MQTT_NOTIFY_OFFLINE)
          {
            DEBUGINFO("MQTT_NOTIFY_OFFLINE\n");  
            char src[] = "OFFLINE";
            memcpy(robotOnOffLine.onoffline,src,strlen(src) + 1);
            Robot_UpdateOnOffLineJson(Robot_OnOffLineJson,&robotOnOffLine); 
            Mqtt_SendMsg(MQTT_MSG_OFFLINE,Robot_GetOnOffLineJsonStr());            
          }                                                           
      }        
    }
}
