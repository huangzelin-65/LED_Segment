#include "LogDebugInfo.h"
#include "StringCommon.h"

#define HEADERID_STRING       "pcb:%d" 
#define HEADERID_VERSION      "%d.%d.%d" 
#define HEADERID_TIMESTAMP    "%lu"

int header_id = 0;

void StrCommon_CreateHeadId(char* headerId)
{
    snprintf(headerId, 64, HEADERID_STRING, header_id++);
}

void StrCommon_CreateVersion(char* version)
{
    snprintf(version, 16, HEADERID_VERSION, 1,0,0);
}

void StrCommon_CreateTimeStamp(char* timestamp)
{
    snprintf(timestamp, 32, HEADERID_TIMESTAMP, Rtc_GetTimeStamp());
}



