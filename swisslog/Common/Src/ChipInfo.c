#include "ChipInfo.h"
#include "LogDebugInfo.h"
#include <ctype.h>

//获取芯片uid
void Chip_GetUId(uint32_t *uid)
{
    HAL_ICACHE_Disable();
    *uid = HAL_GetUIDw0();
    *(uid + 1) = HAL_GetUIDw1();
    *(uid + 2) = HAL_GetUIDw2(); 
    HAL_ICACHE_Enable();     
}




