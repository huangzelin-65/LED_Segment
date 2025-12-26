#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Json.h"
#include "LogDebugInfo.h"
#include "semphr.h"
#include "queue.h"
#include "lwip.h"
#include "adaptor_mqtt.h"
#include "adaptor_ntp.h"
#include "app_freertos.h"
#include "JsonCommon.h"
#include "State.h"
#include "Action.h"
#include "HeartBeat.h"

void vJsonGenerateTask(void *argument)
{
    JsonGenerate_t *json_data = NULL;
    DEBUGINFO("vJsonGenerateTask\n");
    while (1)
    {
        if(xQueueReceive(JsonGenerateQueueHandle, &json_data, portMAX_DELAY) == pdTRUE)
        {
            DEBUGINFO("type:%d\n",json_data->type); 
            switch (json_data->type)
            {
                case JSON_G_HEART:
                {
                    DEBUGINFO("JSON_G_HEART start\n");  
                    HeartBeat_t* heart_beat = (HeartBeat_t*)(json_data->data);
                    char* json_str = Json_Generate_HeartBeat(heart_beat);
                    Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,json_str);//mqtt发送完则释放内存                    
                    DEBUGINFO("JSON_G_HEART end\n"); 
                }
                break;
                case JSON_G_STATE:
                {
                    State_t* state = (State_t*)(json_data->data);
                    DEBUGINFO("JSON_G_STATE start\n");  
                    char* json_str = Json_Generate_State(state);
                    Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,json_str);//mqtt发送完则释放内存
                    DEBUGINFO("JSON_G_STATE end\n"); 
                }
                break;  
                case JSON_G_ACTION:
                {
                    Action_t* action = (Action_t*)(json_data->data);
                    DEBUGINFO("JSON_G_ACTION start\n");  
                    char* json_str = Json_Generate_Action(action);
                    Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,json_str);//mqtt发送完则释放内存
                    DEBUGINFO("JSON_G_ACTION end\n"); 
                }
                break;                                                       
                default:
                break;
            }
            vPortFree(json_data);            
        }
    }
}

void vJsonParseTask(void *argument)
{
    JsonParse_t *json_data = NULL;
    DEBUGINFO("vJsonParseTask\n");
    while (1)
    {
        if(xQueueReceive(JsonParseQueueHandle, &json_data, portMAX_DELAY) == pdTRUE)
        {
            DEBUGINFO("type:%d\n",json_data->type); 
            switch (json_data->type)
            {
                case JSON_ACTION:
                {
                    DEBUGINFO("JSON_ACTION start\n"); 
                    // DEBUGINFO("data:%s\n",json_data->data); 
                    Json_ParseAction(json_data->data);
                    DEBUGINFO("JSON_ACTION end\n"); 
                }
                break;
                case JSON_HEARTBEAT_ACK:
                {
                    DEBUGINFO("JSON_HEARTBEAT_ACK start\n");  

                    DEBUGINFO("JSON_HEARTBEAT_ACK end\n"); 
                }
                break;                                        
                default:
                break;
            }
            vPortFree(json_data);            
        }

    }
}



