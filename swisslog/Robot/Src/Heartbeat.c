#include "LogDebugInfo.h"
#include "HeartBeat.h"
#include "clist.h"
#include "JsonCommon.h"

HeartBeat_t heart_beat;

void Heart_Event(void)
{
    memset(&heart_beat,0,sizeof(HeartBeat_t));
    
    Json_GenerateMsg(JSON_G_HEART,&heart_beat);
}











