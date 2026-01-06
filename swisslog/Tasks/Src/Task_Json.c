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
#include "Feature.h"
#include "Config.h"
#include "Notify.h"
#include "RegisterInfo.h"
#include "StringCommon.h"

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
                    StrCommon_CreateHeadId(heart_beat->headerId);
                    StrCommon_CreateVersion(heart_beat->version);
                    StrCommon_CreateTimeStamp(heart_beat->timestamp);
                    char* json_str = Json_Generate_HeartBeat(heart_beat);
                    Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,json_str);//mqtt发送完则释放内存                    
                    DEBUGINFO("JSON_G_HEART end\n"); 
                }
                break;
                case JSON_G_STATE:
                {
                    State_t* state = (State_t*)(json_data->data);
                    DEBUGINFO("JSON_G_STATE start\n");  
                    StrCommon_CreateHeadId(state->headerId);
                    StrCommon_CreateVersion(state->version);
                    StrCommon_CreateTimeStamp(state->timestamp);                                        
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
                case JSON_G_FEATURE:
                {
                    Feature_t* feature = (Feature_t*)(json_data->data);
                    DEBUGINFO("JSON_G_FEATURE start\n");  
                    char* json_str = Json_Generate_Feature(feature);
                    Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,json_str);//mqtt发送完则释放内存
                    DEBUGINFO("JSON_G_FEATURE end\n"); 
                }
                break;  
                case JSON_G_CONFIG:
                {
                    Config_t* config = (Config_t*)(json_data->data);
                    DEBUGINFO("JSON_G_CONFIG start\n");  
                    char* json_str = Json_Generate_Config(config);
                    Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,json_str);//mqtt发送完则释放内存
                    DEBUGINFO("JSON_G_CONFIG end\n"); 
                }
                break;
                case JSON_G_NOTIFY:
                {
                    Notify_t* notify = (Notify_t*)(json_data->data);
                    DEBUGINFO("JSON_G_NOTIFY start\n");  
                    char* json_str = Json_Generate_Notify(notify);
                    Mqtt_SendMsg(MQTT_MSG_ROBOT_EVENT,json_str);//mqtt发送完则释放内存
                    DEBUGINFO("JSON_G_NOTIFY end\n"); 
                }
                break; 
                case JSON_G_REGISTER:
                {
                    RegisterInfo_t* register_info = (RegisterInfo_t*)(json_data->data);
                    DEBUGINFO("JSON_G_REGISTER start\n");  
                    char* json_str = Json_Generate_Register(register_info);
                    DEBUGINFO("json_str :%s\n",json_str); 
                    Mqtt_SendMsg(MQTT_MSG_REGISTER,json_str);//mqtt发送完则释放内存
                    DEBUGINFO("JSON_G_REGISTER end\n"); 
                }
                break;                                                                                                                                 
                default:
                break;
            }
            if(json_data->data != NULL)vPortFree(json_data->data);
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
                case JSON_PARSE_ACTION:
                {
                    Json_ParseAction(json_data->data);
                }
                break;
                case JSON_PARSE_HEARTBEAT_ACK:
                {
                    Json_ParseHeartBeat(json_data->data);
                }
                break;
                case JSON_PARSE_REGISTER_RESPONSE:
                {
                    Json_ParseRegister(json_data->data);
                }
                break;                                        
                default:
                break;
            }
            if(json_data->data != NULL)vPortFree(json_data->data);
            vPortFree(json_data);            
        }
    }
}



