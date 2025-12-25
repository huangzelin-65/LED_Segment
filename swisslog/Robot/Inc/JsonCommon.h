#ifndef INC_JSON_COMMON_H_
#define INC_JSON_COMMON_H_
#include <stdbool.h>
#include <stdio.h>
#include "cJSON.h"

typedef enum {
    JSON_G_HEART = 0,
    JSON_G_STATE,
} JsonGenerateType_t;

typedef struct {
    JsonGenerateType_t type;
    char *data;
}JsonGenerate_t;


void Json_GenerateMsg(JsonGenerateType_t type,void *data);

#endif