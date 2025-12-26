#include "LogDebugInfo.h"
#include "HeartBeat.h"
#include "clist.h"
#include "JsonCommon.h"

HeartBeat_t heart_beat;

void Heart_Event(void)
{
    memset(&heart_beat,0,sizeof(HeartBeat_t));
    HeartBeat_t* json_heart_beat = pvPortMalloc(sizeof(HeartBeat_t));
    memcpy(json_heart_beat,&heart_beat,sizeof(HeartBeat_t));    
    Json_GenerateMsg(JSON_G_HEART,json_heart_beat);
}











