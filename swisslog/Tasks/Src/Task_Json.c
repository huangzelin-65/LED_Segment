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

                    DEBUGINFO("JSON_G_HEART end\n"); 
                }
                break;
                case ROBOT_MSG_STATE:
                {
                    State_t* state = (State_t*)(json_data->data);
                    DEBUGINFO("JSON_G_STATE start\n");  
                    char* json_str = Json_Generate_State(state);
                    DEBUGINFO("json_str:%s\n",json_str);
                    vPortFree(json_str);
                    DEBUGINFO("JSON_G_STATE end\n"); 
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
    DEBUGINFO("vJsonParseTask\n");
    while (1)
    {
        State_Event(0);
        osDelay(3000);
    }
}



