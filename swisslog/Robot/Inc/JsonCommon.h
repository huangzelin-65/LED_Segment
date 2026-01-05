#ifndef INC_JSON_COMMON_H_
#define INC_JSON_COMMON_H_
#include <stdbool.h>
#include <stdio.h>
#include "cJSON.h"
#include "include_defs.h"

typedef enum {
    JSON_G_HEART = 0,
    JSON_G_STATE,
    JSON_G_ACTION,
    JSON_G_FEATURE,
    JSON_G_CONFIG,
    JSON_G_NOTIFY,
    JSON_G_REGISTER,
} JsonGenerateType_t;

typedef struct {
    JsonGenerateType_t type;
    char *data;
}JsonGenerate_t;

typedef enum {
    JSON_PARSE_NONE = 0,
    JSON_PARSE_ACTION, 
    JSON_PARSE_HEARTBEAT_ACK,
} JsonParseType_t;

typedef struct {
    JsonParseType_t type;
    char *data;
}JsonParse_t;

void Json_GenerateMsg(JsonGenerateType_t type,void *data);
char* Json_Generate_State(void *data);
char* Json_Generate_HeartBeat(void *heart_beat);
char* Json_Generate_Action(void *action);
char* Json_Generate_Feature(void *feature);
char* Json_Generate_Config(void *config);
char* Json_Generate_Notify(void *notify);
char* Json_Generate_Register(void *register_info);
void Json_ParseMsg(JsonParseType_t type,void *data);
int Json_ParseTopic(const char* topic);
int Json_ParseAction(char* data);
int Json_ParseHeartBeat(char* data);
#endif