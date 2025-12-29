#include "LogDebugInfo.h"
#include "StringCommon.h"


int header_id = 0;

static char headerId[64];     // 最多4字符+":"+Long，预留长度
static char timestamp[32];   // Unix毫秒时间戳（字符串形式）
static char version[16];     // 版本号（如"1.0.0"）

#define HEADERID_STRING       "pcb:%d" 
#define HEADERID_VERSION      "%d.%d.%d" 
#define HEADERID_TIMESTAMP    "%ld" 

char* StrCommon_CreateHeadId(void)
{
    snprintf(headerId, sizeof(headerId), HEADERID_STRING, header_id++);
    return headerId;
}

char* StrCommon_CreateVersion(void)
{
    snprintf(version, sizeof(version), HEADERID_VERSION, 1,0,0);
    return version;
}

char* StrCommon_CreateTimeStamp(void)
{
    snprintf(timestamp, sizeof(timestamp), HEADERID_TIMESTAMP, Rtc_GetTimeStamp());
    return timestamp;
}



