#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdarg.h>
#include "main.h"
#include "LogDebugInfo.h"
#include "cmsis_os2.h"
#include <stdlib.h>

extern osMessageQueueId_t xPrint_QueueHandle;


//打印函数
void safe_printf_all(const char *format, ...) {
    if (format == NULL) {  // 检查格式字符串有效性
        return;
    }

    va_list args;
    va_start(args, format);

    // 1. 分配缓冲区并检查是否成功
    char *buffer = pvPortMalloc(LOG_LENGTH * sizeof(char));
    if (buffer == NULL) {  // 处理内存分配失败
        va_end(args);
        return;
    }

    // 2. 格式化字符串（确保LOG_LENGTH至少为1，避免vsnprintf异常）
    int len = vsnprintf(buffer, LOG_LENGTH, format, args);
    va_end(args);

    // 3. 检查格式化结果（len<0表示格式错误，可能由invalid format导致）
    if (len < 0) {
        vPortFree(buffer);  // 释放内存
        return;
    }

    // 4. 向消息队列发送数据（传递指针本身，而非指针的地址）
    if (xPrint_QueueHandle != NULL) {
        // 等待队列有空间（超时时间可根据需求调整，如osWaitForever）
        osStatus_t status = osMessageQueuePut(xPrint_QueueHandle, 
                                             &buffer,  // 传递char*指针
                                             0, 
                                             0);  // 超时时间0表示不等待
        if (status != osOK) {  // 队列满或失败时释放内存，避免泄漏
            vPortFree(buffer);
        }
    } else {  // 队列未初始化，直接释放
        vPortFree(buffer);
    }
}










