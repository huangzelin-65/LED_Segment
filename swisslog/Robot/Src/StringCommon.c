#include "LogDebugInfo.h"
#include "StringCommon.h"

#define HEADERID_STRING       "pcb:%d" 
#define HEADERID_VERSION      "%d.%d.%d" 
#define HEADERID_TIMESTAMP    "%lu"

int header_id = 0;

void StrCommon_CreateHeadId(char* headerId)
{
    snprintf(headerId, HEAD_ID_LENGTH, HEADERID_STRING, header_id++);
}

void StrCommon_CreateVersion(char* version)
{
    snprintf(version, VERSION_LENGTH, HEADERID_VERSION, 1,0,0);
}

void StrCommon_CreateTimeStamp(char* timestamp)
{
    snprintf(timestamp, TIMESTAMP_LENGTH, HEADERID_TIMESTAMP, Rtc_GetTimeStamp());
}



