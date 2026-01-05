#include "LogDebugInfo.h"
#include "RegisterInfo.h"
#include "clist.h"
#include "JsonCommon.h"

#define REGISTER_NAME "TK2.1_CCUB100_VCB_MB" //不同的设备，修改这里的描述

RegisterInfo_t register_info;

void Register_Event(void)
{
    RegisterInfo_t* json_register_info = pvPortMalloc(sizeof(RegisterInfo_t));

    memcpy(register_info.ip,wifi_status.ip,36);  //填入ip信息

    strcpy(register_info.deviceCode,mqtt_info.uuid); //填入uuid

    strcpy(register_info.name,REGISTER_NAME);

    register_info.type = 1; 
    register_info.deviceType = 17;
    register_info.bizSystem = 2;

    memcpy(json_register_info,&register_info,sizeof(RegisterInfo_t));    
    Json_GenerateMsg(JSON_G_REGISTER,json_register_info);
}