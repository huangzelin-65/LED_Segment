#include "JsonCommon.h"
#include "LogDebugInfo.h"
#include "app_freertos.h"
#include "queue.h"
#include "semphr.h"
#include "State.h"
#include "Action.h"
#include "HeartBeat.h"
#include "Encoder.h"
#include <string.h>

#define TOPIC_ACTION        "tk/v1/slhc/tkv-%d/instantactions"
#define TOPIC_CONN_ACK      "tk/v1/slhc/tkv-%d/connection/ack"

/*解析时，字段存储使用以下数组*/
char headerId[64]; 
char timestamp[32];
char version[16];
char cmd[16];
char cmdId[64];
char model[16];
char speedLevel[16];
char sub_topic[64] = {0};
Action_t temp_action;

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

char* Json_Generate_Action(void *action)
{
    Action_t *data = (Action_t *)action;
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
 
    cJSON *actionStates = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "actionStates", actionStates);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "cmdId", data->cmd.cmdId);
    cJSON_AddStringToObject(obj, "cmd", data->cmd.cmd);
    cJSON_AddStringToObject(obj, "status", data->cmd.status);

    cJSON *items = cJSON_CreateArray();
    cJSON_AddItemToArray(items, obj);
    cJSON_AddItemToObject(actionStates, "items", items);
    
    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;
}

void Json_ParseMsg(JsonParseType_t type,void *data)
{
    if(JsonParseQueueHandle != NULL)
    {
        JsonParse_t * msg = pvPortMalloc(sizeof(JsonParse_t));
        if(msg == NULL)return;
        msg->type = type;
        msg->data = data;        
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(JsonParseQueueHandle),uxQueueSpacesAvailable(JsonParseQueueHandle));
        if (xQueueSend(JsonParseQueueHandle, &msg, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("type :%d\n",type);
        } 
    }     
}

int Json_ParseTopic(const char* topic)
{
    DEBUGINFO("topic:%s\n",topic);
    { 
        memset(sub_topic,0,sizeof(sub_topic));
        snprintf(sub_topic, sizeof(sub_topic), TOPIC_ACTION, usEncoder_Read_Number()); 
        char *result = strstr(topic, sub_topic);
        if (result != NULL) {
            return JSON_PARSE_ACTION;
        }
    }
    {
        memset(sub_topic,0,sizeof(sub_topic));
        snprintf(sub_topic, sizeof(sub_topic), TOPIC_CONN_ACK, usEncoder_Read_Number());
        char *result = strstr(topic, sub_topic);
        if (result != NULL) {
            return JSON_PARSE_HEARTBEAT_ACK;
        }  
    }        
    return JSON_PARSE_NONE;
}

int Json_ParseAction(char* data)
{
    if (data == NULL) {
        return -1;
    }
    // 1. 解析整个JSON
    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        DEBUGINFO("cJSON_Parse fail\n");
        return -1;
    }
    // 2. 解析顶层字段: headerId
    cJSON *headerid = cJSON_GetObjectItem(root, "headerId");
    if (headerid == NULL || !cJSON_IsString(headerid)) {
        DEBUGINFO("headerId Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(headerId, headerid->valuestring, sizeof(headerId)-1);
    headerId[sizeof(headerId)-1] = '\0';  

    // 3. 解析顶层字段: timestamp
    cJSON *time_stamp = cJSON_GetObjectItem(root, "timestamp");
    if (time_stamp == NULL || !cJSON_IsString(time_stamp)) {
        DEBUGINFO("timestamp Not find\n");
        cJSON_Delete(root);
        return -1;
    }
 
    strncpy(timestamp, time_stamp->valuestring, sizeof(timestamp)-1);
    timestamp[sizeof(timestamp)-1] = '\0';

    // 4. 解析顶层字段: version
    cJSON *version_js = cJSON_GetObjectItem(root, "version");
    if (version_js == NULL || !cJSON_IsString(version_js)) {
        DEBUGINFO("version Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(version, version_js->valuestring, sizeof(version)-1);
    version[sizeof(version)-1] = '\0';

    // 5. 解析action对象
    cJSON *action_obj = cJSON_GetObjectItem(root, "action");
    if (action_obj == NULL || !cJSON_IsObject(action_obj)) {
        DEBUGINFO("action Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    // 5.1 解析action.Type
    cJSON *type = cJSON_GetObjectItem(action_obj, "Type");
    if (type == NULL || !cJSON_IsNumber(type)) {
        DEBUGINFO("action.Type Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    int Type = type->valueint;

    // 5.2 解析action.cmds数组
    cJSON *cmds_array = cJSON_GetObjectItem(action_obj, "cmds");
    if (cmds_array == NULL || !cJSON_IsArray(cmds_array)) {
        DEBUGINFO("action.cmds Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    //限制最多八条命令
    int cmd_count = cJSON_GetArraySize(cmds_array);
    if (cmd_count <= 0 || cmd_count > 8) {
        DEBUGINFO("cmds Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    else
    {
        // 遍历cmds数组，解析每个命令
        for (int i = 0; i < cmd_count; i++) {
            cJSON *cmd_obj = cJSON_GetArrayItem(cmds_array, i);
            if (cmd_obj == NULL || !cJSON_IsObject(cmd_obj)) {
                DEBUGINFO("cmds %d is not object\n", i);
                cJSON_Delete(root);
                return -1;
            }

            // 解析cmd字段
            cJSON *cmd_js = cJSON_GetObjectItem(cmd_obj, "cmd");
            if (cmd_js == NULL || !cJSON_IsString(cmd_js)) {
                DEBUGINFO("cmd [%d] is not string\n", i);
                cJSON_Delete(root);
                return -1;
            }

            strncpy(cmd, cmd_js->valuestring, sizeof(cmd)-1);
            cmd[sizeof(cmd)-1] = '\0';

            // 解析cmdId字段
            cJSON *cmdId_js = cJSON_GetObjectItem(cmd_obj, "cmdId");
            if (cmdId_js == NULL || !cJSON_IsString(cmdId_js)) {
                DEBUGINFO("cmdId[%d] is not string\n", i);
                cJSON_Delete(root);
                return -1;
            }

            strncpy(cmdId, cmdId_js->valuestring, sizeof(cmdId)-1);
            cmdId[sizeof(cmdId)-1] = '\0';

            // 解析params对象（model字段）
            cJSON *params_obj = cJSON_GetObjectItem(cmd_obj, "params");
            if (params_obj == NULL || !cJSON_IsObject(params_obj)) {
                DEBUGINFO("params[%d] is not object\n", i);
                cJSON_Delete(root);
                return -1;
            }

            cJSON *model_js = cJSON_GetObjectItem(params_obj, "model");
            if (model_js != NULL && cJSON_IsString(model_js)) {  
                strncpy(model, model_js->valuestring, sizeof(model)-1);
                model[sizeof(model)-1] = '\0';
            } else {
                model[0] = '\0'; 
            }

            cJSON *speedLevel_js = cJSON_GetObjectItem(params_obj, "speedLevel");
            if (speedLevel_js != NULL && cJSON_IsString(speedLevel_js)) { 
                strncpy(speedLevel, speedLevel_js->valuestring, sizeof(speedLevel)-1);
                speedLevel[sizeof(speedLevel)-1] = '\0';
            } else {
                speedLevel[0] = '\0';  
            }
            
            memcpy(temp_action.headerId,headerId,sizeof(headerId));
            memcpy(temp_action.timestamp,timestamp,sizeof(timestamp));
            memcpy(temp_action.version,version,sizeof(version));
            memcpy(temp_action.cmd.cmd,cmd,sizeof(cmd));
            memcpy(temp_action.cmd.cmdId,cmdId,sizeof(cmdId));
            memcpy(temp_action.cmd.params.model,model,sizeof(model));
            memcpy(temp_action.cmd.params.speedLevel,speedLevel,sizeof(speedLevel));
            temp_action.id = 0;//后续有多个机器时这里有多个id,需要根据解析的id赋值
            Action_Event(&temp_action);
        }
        // 释放cJSON资源
        cJSON_Delete(root);
        // 打印顶层结构体成员
        DEBUGINFO("  headerId: %s\n", headerId);
        DEBUGINFO("  timestamp: %s\n",timestamp);
        DEBUGINFO("  version: %s\n", version);
        // 打印RobotAction成员
        DEBUGINFO("  Type: %d\n", Type);
        DEBUGINFO("  cmd_count: %d\n", cmd_count);  
    }
    return 0;
}

int Json_ParseHeartBeat(char* data)
{
    if (data == NULL) {
        return -1;
    }
    // 1. 解析整个JSON
    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        DEBUGINFO("cJSON_Parse fail\n");
        return -1;
    }
    // 2. 解析顶层字段: headerId
    cJSON *headerid = cJSON_GetObjectItem(root, "headerId");
    if (headerid == NULL || !cJSON_IsString(headerid)) {
        DEBUGINFO("headerId Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(headerId, headerid->valuestring, sizeof(headerId)-1);
    headerId[sizeof(headerId)-1] = '\0';  

    // 3. 解析顶层字段: timestamp
    cJSON *time_stamp = cJSON_GetObjectItem(root, "timestamp");
    if (time_stamp == NULL || !cJSON_IsString(time_stamp)) {
        DEBUGINFO("timestamp Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(timestamp, time_stamp->valuestring, sizeof(timestamp)-1);
    timestamp[sizeof(timestamp)-1] = '\0';

    // 4. 解析顶层字段: version
    cJSON *version_js = cJSON_GetObjectItem(root, "version");
    if (version_js == NULL || !cJSON_IsString(version_js)) {
        DEBUGINFO("version Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(version, version_js->valuestring, sizeof(version)-1);
    version[sizeof(version)-1] = '\0';

    // 释放cJSON资源
    cJSON_Delete(root);    

    DEBUGINFO("  headerId: %s\n", headerId);
    DEBUGINFO("  timestamp: %s\n",timestamp);
    DEBUGINFO("  version: %s\n", version);

    return 0;
}


