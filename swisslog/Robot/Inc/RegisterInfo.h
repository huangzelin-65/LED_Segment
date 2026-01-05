#ifndef INC_REGISTER_INFO_H_
#define INC_REGISTER_INFO_H_
#include <stdbool.h>
#include <stdio.h>
#include "include_defs.h"

typedef struct {
    char name[NAME_LENGTH];   
    char ip[IP_LENGTH];   
    char deviceCode[DEVICE_CODE_LENGTH];  
    int type;//1:ccub100、2:ccub500
    int deviceType;//17:轨道小车 18：转轨器 
    int bizSystem;//业务系统类型：1:箱式物流、2 轨道物流、3:PA、4:UnisSmart、5:UniScan     
} RegisterInfo_t;

extern RegisterInfo_t register_info;

void Register_Event(void);

#endif
