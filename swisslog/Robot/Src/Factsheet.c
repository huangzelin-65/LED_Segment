#include "LogDebugInfo.h"
#include "Factsheet.h"
#include "clist.h"
#include "JsonCommon.h"

Factsheet_t factsheet;

void Factsheet_Event(void)
{
    memset(&factsheet,0,sizeof(factsheet));
    Factsheet_t* json_factsheet = pvPortMalloc(sizeof(Factsheet_t));
    memcpy(json_factsheet,&factsheet,sizeof(Factsheet_t));    
    Json_GenerateMsg(JSON_G_FACTSHEET,json_factsheet);
}











