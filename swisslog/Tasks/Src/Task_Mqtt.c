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

#define MQTT_TOPIC_NAME                 "tk/v1/slhc/tkv-%d/state" 
#define MQTT_HEARTBEAT_TOPIC_NAME       "tk/v1/slhc/tkv-%d/connection" 
#define MQTT_PUBLISH_MSG                "HEARTBEAT"
#define MQTT_CMD_TIMEOUT_MS             30000

extern CarStatus_t CarStatus;
extern osMessageQueueId_t xMqttManagerQueueHandle;
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
                        Mqtt_SendMsg(MQTT_MSG_SUBSCRIBE,NULL);
                    }
                }
                break;
                case MQTT_MSG_HEARTBEAT:
                {
                    char* robot_json_str = (char*)msg->data;
                    char topic[64] = {0};
                    snprintf(topic, sizeof(topic), MQTT_HEARTBEAT_TOPIC_NAME, CarStatus.usCarID);//usEncoder_Read_Number()                  
                    if(mqtt_isConnected)Mqtt_PublishMsg(topic, robot_json_str, XSTRLEN(robot_json_str), 0, 0);
                    vPortFree(robot_json_str);
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
                        snprintf(topic, sizeof(topic), MQTT_TOPIC_NAME, CarStatus.usCarID);//usEncoder_Read_Number()
                        if(mqtt_isConnected)Mqtt_PublishMsg(topic, robot_json_str, XSTRLEN(robot_json_str), 1, 0);
                        vPortFree(robot_json_str);
                    }
                    DEBUGINFO("MQTT_MSG_ROBOT_EVENT end\n");
                }
                break;
                case MQTT_MSG_SUBSCRIBE:
                {
                    int rc = Mqtt_SubscribeTopicInit();
                    if (rc != MQTT_CODE_SUCCESS) {
                        DEBUGINFO("Mqtt_SubscribeTopicInit fail");
                        //订阅话题失败，尝试再次订阅
                        Mqtt_SendMsg(MQTT_MSG_SUBSCRIBE,NULL);                        
                    }                    
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
    DEBUGINFO("vMqttReceiveTask\r\n");
    int rc = 0;
    while (1)
    {
        if(mqtt_isConnected)
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
        if(!mqtt_isConnected)osDelay(pdMS_TO_TICKS(100));
    }
}


