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
#include <stdlib.h>
#include "Feature.h"
#include "Config.h"
#include "Notify.h"
#include "RegisterInfo.h"
#include "Factsheet.h"

#define TOPIC_ACTION                 "tk/v1/slhc/tkv-%s/instantactions"
#define TOPIC_CONN_ACK               "tk/v1/slhc/tkv-%s/connection/ack"
#define TOPIC_REGISTER_RESPONSE      "bcss/v1/slhc/register/response"

/*解析时，字段存储使用以下数组*/
char headerId[HEAD_ID_LENGTH]; 
char timestamp[TIMESTAMP_LENGTH];
char version[VERSION_LENGTH];
char cmd[CMD_LENGTH];
char cmdId[CMD_ID_LENGTH];
char model[MODEL_LENGTH];
char speedLevel[SPEEDLEVEL_LENGTH];
char sub_topic[SUBTOPIC_LENGTH] = {0};
char name[NAME_LENGTH];
char value[VALUE_LENGTH];
char params[PARAMS_LENGTH];
char type[TYPE_LENGTH];
char message[MESSAGE_LENGTH];
char user[USER_LENGTH];
char pwd[PWD_LENGTH];
char deviceCode[DEVICE_CODE_LENGTH];
char sn[SN_LENGTH];
int Type;
int code;
Action_t temp_action;
Feature_t temp_feature;
Config_t temp_config;
Notify_t temp_notify;

Stru_Field_Register_Typedef s_register;

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
    cJSON_AddItemToObject(root, "hallState", obj);

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

    cJSON *errors = cJSON_CreateArray();
    cJSON *errorItem = cJSON_CreateObject();
    cJSON_AddStringToObject(errorItem, "errorType", data->errors.Type);
    cJSON_AddStringToObject(errorItem, "errorLevel", data->errors.Level);
    cJSON_AddNumberToObject(errorItem, "errorCode", data->errors.Code);
    cJSON_AddItemToArray(errors, errorItem);
    cJSON_AddItemToObject(root, "errors", errors);

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
    cJSON_AddNumberToObject(root, "curPos", data->curPos);
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

char* Json_Generate_Feature(void *feature)
{
    Feature_t *data = (Feature_t *)feature;
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
 
    cJSON *featureStates = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "featureStates", featureStates);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "name", data->name);
    cJSON_AddNumberToObject(obj, "code", data->code);

    cJSON *cause_array = cJSON_CreateArray();
    cJSON *cause_str = cJSON_CreateString(data->cause);
    cJSON_AddItemToArray(cause_array, cause_str);
    cJSON_AddItemToObject(obj, "cause", cause_array);

    cJSON *items = cJSON_CreateArray();
    cJSON_AddItemToArray(items, obj);
    cJSON_AddItemToObject(featureStates, "items", items);
    
    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;
}

char* Json_Generate_Config(void *config)
{
    Config_t *data = (Config_t *)config;
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
 
    cJSON *configStates = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "configStates", configStates);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "name", data->name);
    cJSON_AddNumberToObject(obj, "code", data->code);

    cJSON *cause_array = cJSON_CreateArray();
    cJSON *cause_str = cJSON_CreateString(data->cause);
    cJSON_AddItemToArray(cause_array, cause_str);
    cJSON_AddItemToObject(obj, "cause", cause_array);

    cJSON *items = cJSON_CreateArray();
    cJSON_AddItemToArray(items, obj);
    cJSON_AddItemToObject(configStates, "items", items);
    
    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;
}

char* Json_Generate_Notify(void *notify)
{
    Notify_t *data = (Notify_t *)notify;
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
 
    cJSON *notifyStates = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "notifyStates", notifyStates);

    cJSON *obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "type", data->type);
    cJSON_AddNumberToObject(obj, "code", data->code);

    cJSON *items = cJSON_CreateArray();
    cJSON_AddItemToArray(items, obj);
    cJSON_AddItemToObject(notifyStates, "items", items);
    
    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;
}

char* Json_Generate_Register(void *register_info)
{
    RegisterInfo_t *data = (RegisterInfo_t *)register_info;
    cJSON* root = cJSON_CreateObject();
    if(root == NULL)
    {
        DEBUGINFO("cJSON_CreateObject fail\n");
        return NULL;
    }
    //创建单字段json对象
    cJSON_AddStringToObject(root, "name",data->name);//"TK2.1_CCUB100_VCB_MB"
    cJSON_AddNumberToObject(root, "bizSystem", data->bizSystem);
    cJSON_AddStringToObject(root, "ip", data->ip);
    cJSON_AddNumberToObject(root, "type", data->type);
    cJSON_AddStringToObject(root, "deviceCode", data->deviceCode);
    cJSON_AddNumberToObject(root, "deviceType", data->deviceType);

    char* json_str = cJSON_PrintUnformatted(root);//需free

    cJSON_Delete(root);

    return json_str;    
}

char* Json_Generate_Factsheet(void *factsheet)
{
    Factsheet_t *data = (Factsheet_t *)factsheet;
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
        snprintf(sub_topic, sizeof(sub_topic), TOPIC_ACTION, mqtt_info.id); 
        char *result = strstr(topic, sub_topic);
        if (result != NULL) {
            return JSON_PARSE_ACTION;
        }
    }
    {
        memset(sub_topic,0,sizeof(sub_topic));
        snprintf(sub_topic, sizeof(sub_topic), TOPIC_CONN_ACK, mqtt_info.id);
        char *result = strstr(topic, sub_topic);
        if (result != NULL) {
            return JSON_PARSE_HEARTBEAT_ACK;
        }  
    }   
    {
        memset(sub_topic,0,sizeof(sub_topic));
        snprintf(sub_topic, sizeof(sub_topic), TOPIC_REGISTER_RESPONSE);
        char *result = strstr(topic, sub_topic);
        if (result != NULL) {
            return JSON_PARSE_REGISTER_RESPONSE;
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
    }
    else
    {
        // 5.1 解析action.Type
        cJSON *type = cJSON_GetObjectItem(action_obj, "Type");
        if (type == NULL || !cJSON_IsNumber(type)) {
            DEBUGINFO("action.Type Not find\n");
            cJSON_Delete(root);
            return -1;
        }
        Type = type->valueint;
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
            DEBUGINFO("cmds Not find or over size \n");
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

                temp_action.Type = Type;
                temp_action.id = 0;//后续有多个机器时这里有多个id,需要根据解析的id赋值
                Action_Event(&temp_action);                
            }               
        }
    }
    // 6. 解析feature对象
    cJSON *feature_obj = cJSON_GetObjectItem(root, "feature");
    if (feature_obj == NULL || !cJSON_IsObject(feature_obj)) {
        DEBUGINFO("feature Not find\n");
    }
    else
    {
        // 5.2 解析items数组
        cJSON *items_array = cJSON_GetObjectItem(feature_obj, "items");
        if (items_array == NULL || !cJSON_IsArray(items_array)) {
            DEBUGINFO("items_array Not find\n");
            cJSON_Delete(root);
            return -1;
        }
        else
        {
            //限制最多八条命令
            int items_count = cJSON_GetArraySize(items_array);
            if (items_count <= 0 || items_count > 8) {
                DEBUGINFO("items Not find or over size \n");
                cJSON_Delete(root);
                return -1;
            } 
            else
            {
                // 遍历数组，解析每个命令
                for (int i = 0; i < items_count; i++) {
                    cJSON *feature_obj = cJSON_GetArrayItem(items_array, i);
                    if (feature_obj == NULL || !cJSON_IsObject(feature_obj)) {
                        DEBUGINFO("feature_obj %d is not object\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    // 解析name字段
                    cJSON *name_js = cJSON_GetObjectItem(feature_obj, "name");
                    if (name_js == NULL || !cJSON_IsString(name_js)) {
                        DEBUGINFO("name [%d] is not string\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    strncpy(name, name_js->valuestring, sizeof(name)-1);
                    name[sizeof(name)-1] = '\0';
                    // 解析value字段
                    cJSON *value_js = cJSON_GetObjectItem(feature_obj, "value");
                    if (value_js == NULL || !cJSON_IsString(value_js)) {
                        DEBUGINFO("value [%d] is not string\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    strncpy(value, value_js->valuestring, sizeof(value)-1);
                    value[sizeof(value)-1] = '\0';

                    // 解析params对象
                    cJSON *params_array = cJSON_GetObjectItem(feature_obj, "params");
                    if (params_array == NULL || !cJSON_IsArray(params_array)) {
                        DEBUGINFO("params[%d] is not array\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }

                    int params_count = cJSON_GetArraySize(params_array);
                    if(params_count > PARAM_MAX_COUNT)params_count = PARAM_MAX_COUNT;
                    temp_feature.params_cnt =  params_count;
                    for (int i = 0; i < params_count; i++)
                    {
                        cJSON *params_js = cJSON_GetArrayItem(params_array, i);
                        if (params_js == NULL || !cJSON_IsString(params_js)) {
                            DEBUGINFO("params_js %d is not string\n", i);
                            cJSON_Delete(root);
                            return -1;
                        }   
                        strncpy(params, params_js->valuestring, sizeof(params)-1);
                        params[sizeof(params)-1] = '\0';                        
                        DEBUGINFO("  params(%d): %s\n", i ,params);
                        strncpy(temp_feature.params[i], params_js->valuestring, sizeof(params)-1);
                        temp_feature.params[i][sizeof(params)-1] = '\0';
                    }

                    //feature中参数数组暂时没有params
                    DEBUGINFO("  name: %s\n", name);
                    DEBUGINFO("  value: %s\n", value);
                    

                    memcpy(temp_feature.headerId,headerId,sizeof(headerId));
                    memcpy(temp_feature.timestamp,timestamp,sizeof(timestamp));
                    memcpy(temp_feature.version,version,sizeof(version));
                    memcpy(temp_feature.name,name,sizeof(name));
                    memcpy(temp_feature.value,value,sizeof(value));   
                    // memcpy(temp_feature.params,params,sizeof(params)); 
                    temp_feature.id = 0;
                    Feature_Event(&temp_feature);
                }                
            }           
        }
    }
    // 6. 解析config对象
    cJSON *config_obj = cJSON_GetObjectItem(root, "config");
    if (config_obj == NULL || !cJSON_IsObject(config_obj)) {
        DEBUGINFO("config Not find\n");
    }
    else
    {
        // 5.2 解析items数组
        cJSON *items_array = cJSON_GetObjectItem(config_obj, "items");
        if (items_array == NULL || !cJSON_IsArray(items_array)) {
            DEBUGINFO("items_array Not find\n");
            cJSON_Delete(root);
            return -1;
        }
        else
        {
            //限制最多八条命令
            int items_count = cJSON_GetArraySize(items_array);
            if (items_count <= 0 || items_count > 8) {
                DEBUGINFO("items Not find or over size \n");
                cJSON_Delete(root);
                return -1;
            } 
            else
            {
                // 遍历数组，解析每个命令
                for (int i = 0; i < items_count; i++) {
                    cJSON *config_obj = cJSON_GetArrayItem(items_array, i);
                    if (config_obj == NULL || !cJSON_IsObject(config_obj)) {
                        DEBUGINFO("config_obj %d is not object\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    // 解析name字段
                    cJSON *name_js = cJSON_GetObjectItem(config_obj, "name");
                    if (name_js == NULL || !cJSON_IsString(name_js)) {
                        DEBUGINFO("name [%d] is not string\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    strncpy(name, name_js->valuestring, sizeof(name)-1);
                    name[sizeof(name)-1] = '\0';
                    // 解析value字段
                    cJSON *value_js = cJSON_GetObjectItem(config_obj, "value");
                    if (value_js == NULL || !cJSON_IsString(value_js)) {
                        DEBUGINFO("value [%d] is not string\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    strncpy(value, value_js->valuestring, sizeof(value)-1);
                    value[sizeof(value)-1] = '\0';

                    // 解析params对象
                    cJSON *params_array = cJSON_GetObjectItem(config_obj, "params");
                    if (params_array == NULL || !cJSON_IsArray(params_array)) {
                        DEBUGINFO("params[%d] is not array\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }

                    int params_count = cJSON_GetArraySize(params_array);
                    if(params_count > PARAM_MAX_COUNT)params_count = PARAM_MAX_COUNT;
                    temp_config.params_cnt =  params_count;
                    for (int i = 0; i < params_count; i++)
                    {
                        cJSON *params_js = cJSON_GetArrayItem(params_array, i);
                        if (params_js == NULL || !cJSON_IsString(params_js)) {
                            DEBUGINFO("params_js %d is not string\n", i);
                            cJSON_Delete(root);
                            return -1;
                        }   
                        strncpy(params, params_js->valuestring, sizeof(params)-1);
                        params[sizeof(params)-1] = '\0';                        
                        DEBUGINFO("  params(%d): %s\n", i,params);
                        strncpy(temp_config.params[i], params_js->valuestring, sizeof(params)-1);
                        temp_config.params[i][sizeof(params)-1] = '\0';                        
                    }

                    //config
                    DEBUGINFO("  name: %s\n", name);
                    DEBUGINFO("  value: %s\n", value);
                    

                    memcpy(temp_config.headerId,headerId,sizeof(headerId));
                    memcpy(temp_config.timestamp,timestamp,sizeof(timestamp));
                    memcpy(temp_config.version,version,sizeof(version));
                    memcpy(temp_config.name,name,sizeof(name));
                    memcpy(temp_config.value,value,sizeof(value));   
                    // memcpy(temp_config.params,params,sizeof(params)); 
                    temp_config.id = 0;
                    Config_Event(&temp_config);
                }                
            }           
        }
    }

    // 解析notify对象
    cJSON *notify_obj = cJSON_GetObjectItem(root, "notify");
    if (notify_obj == NULL || !cJSON_IsObject(notify_obj)) {
        DEBUGINFO("notify Not find\n");
    }
    else
    {
        // 5.2 解析items数组
        cJSON *items_array = cJSON_GetObjectItem(notify_obj, "items");
        if (items_array == NULL || !cJSON_IsArray(items_array)) {
            DEBUGINFO("items_array Not find\n");
            cJSON_Delete(root);
            return -1;
        }
        else
        {
            //限制最多八条命令
            int items_count = cJSON_GetArraySize(items_array);
            if (items_count <= 0 || items_count > 8) {
                DEBUGINFO("items Not find or over size \n");
                cJSON_Delete(root);
                return -1;
            } 
            else
            {
                // 遍历cmds数组，解析每个命令
                for (int i = 0; i < items_count; i++) {
                    cJSON *notify_obj = cJSON_GetArrayItem(items_array, i);
                    if (notify_obj == NULL || !cJSON_IsObject(notify_obj)) {
                        DEBUGINFO("notify_obj %d is not object\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    // 解析type字段
                    cJSON *type_js = cJSON_GetObjectItem(notify_obj, "type");
                    if (type_js == NULL || !cJSON_IsString(type_js)) {
                        DEBUGINFO("type_js [%d] is not string\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    strncpy(type, type_js->valuestring, sizeof(type)-1);
                    type[sizeof(type)-1] = '\0';
                    // 解析code字段
                    cJSON *code_js = cJSON_GetObjectItem(notify_obj, "code");
                    if (code_js == NULL || !cJSON_IsNumber(code_js)) {
                        DEBUGINFO("code [%d] is not number\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }

                    code = code_js->valueint;

                    // 解析message对象
                    cJSON *msg_js = cJSON_GetObjectItem(notify_obj, "message");
                    if (msg_js == NULL || !cJSON_IsString(msg_js)) {
                        DEBUGINFO("message [%d] is not string\n", i);
                        cJSON_Delete(root);
                        return -1;
                    }
                    strncpy(message, msg_js->valuestring, sizeof(message)-1);
                    message[sizeof(message)-1] = '\0';

                    //config
                    DEBUGINFO("  type: %s\n", type);
                    DEBUGINFO("  code: %d\n", code);
                    DEBUGINFO("  message: %s\n", message);

                    memcpy(temp_notify.headerId,headerId,sizeof(headerId));
                    memcpy(temp_notify.timestamp,timestamp,sizeof(timestamp));
                    memcpy(temp_notify.version,version,sizeof(version));
                    memcpy(temp_notify.type,type,sizeof(type));  
                    memcpy(temp_notify.message,message,sizeof(message)); 
                    temp_notify.code = code;
                    temp_notify.id = 0;
                    Notify_Event(&temp_notify);
                }                
            }           
        }
    }    
    // 释放cJSON资源
    cJSON_Delete(root);
    // 打印顶层结构体成员
    DEBUGINFO("  headerId: %s\n", headerId);
    DEBUGINFO("  timestamp: %s\n",timestamp);
    DEBUGINFO("  version: %s\n", version);    
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

    long long temp_timestamp = atoll(timestamp);
    sntp_set_system_time(temp_timestamp/1000);

    return 0;
}

int Json_ParseRegister(char* data)
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
    // 2. 解析顶层字段: user
    cJSON *user_json = cJSON_GetObjectItem(root, "user");
    if (user_json == NULL || !cJSON_IsString(user_json)) {
        DEBUGINFO("user Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(user, user_json->valuestring, sizeof(user)-1);
    user[sizeof(user)-1] = '\0';  

    // 3. 解析顶层字段: pwd
    cJSON *pwd_json = cJSON_GetObjectItem(root, "pwd");
    if (pwd_json == NULL || !cJSON_IsString(pwd_json)) {
        DEBUGINFO("pwd Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(pwd, pwd_json->valuestring, sizeof(pwd)-1);
    pwd[sizeof(pwd)-1] = '\0';

    // 4. 解析顶层字段: deviceCode
    cJSON *deviceCode_js = cJSON_GetObjectItem(root, "deviceCode");
    if (deviceCode_js == NULL || !cJSON_IsString(deviceCode_js)) {
        DEBUGINFO("deviceCode Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(deviceCode, deviceCode_js->valuestring, sizeof(deviceCode)-1);
    deviceCode[sizeof(deviceCode)-1] = '\0';

    // 5. 解析顶层字段: sn
    cJSON *sn_js = cJSON_GetObjectItem(root, "sn");
    if (sn_js == NULL || !cJSON_IsString(sn_js)) {
        DEBUGINFO("sn Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    strncpy(sn, sn_js->valuestring, sizeof(sn)-1);
    sn[sizeof(sn)-1] = '\0';

    // 6. 解析顶层字段: timestamp
    // cJSON *time_stamp = cJSON_GetObjectItem(root, "ts");
    // if (time_stamp == NULL || !cJSON_IsString(time_stamp)) {
    //     DEBUGINFO("ts Not find\n");
    //     cJSON_Delete(root);
    //     return -1;
    // }

    // strncpy(timestamp, time_stamp->valuestring, sizeof(timestamp)-1);
    // timestamp[sizeof(timestamp)-1] = '\0';

    // 释放cJSON资源
    cJSON_Delete(root);    

    DEBUGINFO("  user: %s\n", user);
    DEBUGINFO("  pwd: %s\n",pwd);
    DEBUGINFO("  deviceCode: %s\n", deviceCode);
    DEBUGINFO("  sn: %s\n",sn);
    // DEBUGINFO("  timestamp: %s\n", timestamp);

    mqtt_info.Register = 0;//无需再发布注册消息

    memset(&s_register,0,sizeof(Stru_Field_Register_Typedef));

    memcpy(s_register.name,user,USER_LENGTH); 

    memcpy(s_register.pwd,pwd,PWD_LENGTH); 

    memcpy(s_register.sn,sn,SN_LENGTH);

    bool rc = bWriteFieldRegisterInfo(&s_register);

    if(rc)
    {
        DEBUGINFO("bWriteFieldRegisterInfo success\n"); 
        //通知mqtt重新断开并重新连接
        Mqtt_SendMsg(MQTT_MSG_DISCONNECT,NULL);   
    }

    return 0;
}
