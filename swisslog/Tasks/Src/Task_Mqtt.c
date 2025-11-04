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

#define MQTT_CMD_TIMEOUT_MS    30000

extern osMessageQueueId_t xMqttManagerQueueHandle;
//mqtt主任务，处理初始化，发送消息等
void vMqttManagerTask(void *argument)
{
    DEBUGINFO("vMqttManagerTask\r\n");
    char *manage_data = NULL;
    while (1)
    {
        if(xQueueReceive(xMqttManagerQueueHandle, &manage_data, portMAX_DELAY) == pdTRUE)
        {
            MqttMsgdata_t *msg = (MqttMsgdata_t *)manage_data;
            DEBUGINFO("msg type:%d",msg->type);
            switch(msg->type)
            {
                case MQTT_MSG_WIFI_CHANGE:
                {
                    int rc = MqttInit();
                    if(rc == MQTT_CODE_SUCCESS)
                    {
                        mqtt_isConnected = 1;
                        DEBUGINFO("mqtt_isConnected");
                    }
                }
                break;
                case MQTT_MSG_HEARTBEAT:
                {

                }
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


