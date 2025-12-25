#include "JsonCommon.h"
#include "LogDebugInfo.h"
#include "app_freertos.h"
#include "queue.h"
#include "semphr.h"
#include "State.h"
#include "HeartBeat.h"

void Json_GenerateMsg(JsonGenerateType_t type,void *data)
{
    if(JsonGenerateQueueHandle != NULL)
    {
        JsonGenerate_t * msg = pvPortMalloc(sizeof(JsonGenerate_t));
        if(msg == NULL)return;
        msg->type = type;
        msg->data = data;        
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(JsonGenerateQueueHandle),uxQueueSpacesAvailable(JsonGenerateQueueHandle));
        if (xQueueSend(JsonGenerateQueueHandle, &msg, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("type :%d\n",type);
        } 
    }     
}

char* Json_Generate_State(void *state)
{
    State_t *data = (State_t *)state;
    cJSON* root = cJSON_CreateObject();
    if(root == NULL)
    {
        DEBUGINFO("cJSON_CreateObject fail\n");
        return NULL;
    }
    //创建单字段json对象
    cJSON_AddStringToObject(root, "headerId", data->headerId);
    cJSON_AddStringToObject(root, "timestamp", data->timestamp);
    cJSON_AddStringToObject(root, "version", data->version);
    cJSON_AddStringToObject(root, "operatingMode", data->operatingMode);
    cJSON_AddBoolToObject(root, "lockState1", data->lockState1);
    cJSON_AddBoolToObject(root, "lockState2", data->lockState2);
    cJSON_AddBoolToObject(root, "lockState", data->lockState);
    cJSON_AddNumberToObject(root, "runtime", data->runtime);
    cJSON_AddNumberToObject(root, "curPos", data->curPos); 
    //创建嵌套json对象
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "front", data->bumperState.front);
    cJSON_AddBoolToObject(obj, "back", data->bumperState.back); 
    cJSON_AddItemToObject(root, "bumperState", obj);   

    obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "front", data->hallState.front);
    cJSON_AddBoolToObject(obj, "back", data->hallState.back);    
    cJSON_AddItemToObject(root, "HallState", obj);

    obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "speedLevel", data->motorState.speedLevel);
    cJSON_AddNumberToObject(obj, "direction", data->motorState.direction);
    cJSON_AddItemToObject(root, "moveState", obj);

    obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "runState", data->disinfect.runState);
    cJSON_AddNumberToObject(obj, "runTime", data->disinfect.runTime);
    cJSON_AddNumberToObject(obj, "startTime", data->disinfect.startTime);
    cJSON_AddNumberToObject(obj, "setTime", data->disinfect.setTime);
    cJSON_AddItemToObject(root, "disinfectState", obj);

    obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "errorType", data->errors.Type);
    cJSON_AddStringToObject(obj, "errorLevel", data->errors.Level);    
    cJSON_AddItemToObject(root, "errors", obj); 
    
    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;
}

char* Json_Generate_HeartBeat(void *heart_beat)
{
    HeartBeat_t *data = (HeartBeat_t *)heart_beat;
    cJSON* root = cJSON_CreateObject();
    if(root == NULL)
    {
        DEBUGINFO("cJSON_CreateObject fail\n");
        return NULL;
    }
    //创建单字段json对象
    cJSON_AddStringToObject(root, "headerId", data->headerId);
    cJSON_AddStringToObject(root, "timestamp", data->timestamp);
    cJSON_AddStringToObject(root, "version", data->version);

    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;
}










