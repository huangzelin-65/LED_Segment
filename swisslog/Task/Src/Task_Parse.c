// 1. 补充cmsis_os.h（定义osSemaphoreId_t）+ 修正FreeRTOS.h大小写
#include "main.h"
#include "FreeRTOS.h"  // 小写reertos，标准写法
#include "LogDebugInfo.h"
#include "semphr.h"
#include "cmsis_os.h"  // 关键：添加这个头文件，识别osSemaphoreId_t

// 2. 声明的名称和freertos.c一致（带Handle后缀）
extern osSemaphoreId_t xRS485RxSemHandle;

void xPackage_Parse(void *argument)
{
    while(1)
    {
        // 3. 名称改为xRS485RxSemHandle + 强转类型适配原生API
        if(xSemaphoreTake((SemaphoreHandle_t)xRS485RxSemHandle, portMAX_DELAY) == pdPASS)
        {
            DEBUGINFO("GET SEM SUCCEED");
        }
    }
}
