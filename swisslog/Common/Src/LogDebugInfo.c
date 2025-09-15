#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdarg.h>
#include "main.h"
#include "LogDebugInfo.h"
#include "cmsis_os2.h"
#include <stdlib.h>

extern osMessageQueueId_t xPrint_QueueHandle;

extern UART_HandleTypeDef huart1;
//普通打印函数
void safe_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buffer  = pvPortMalloc(LOG_LENGTH * sizeof(char));
    int len = vsnprintf(buffer, LOG_LENGTH, format, args);
    // 手动添加终止符
    if (len >= LOG_LENGTH) {
        buffer[LOG_LENGTH - 1] = '\0';
        len = LOG_LENGTH - 1;
    }
    va_end(args);

    if(xPrint_QueueHandle != NULL && len > 0) {
        xQueueSend(xPrint_QueueHandle, &buffer, pdMS_TO_TICKS(100));
    }
}
//中断中打印函数
void safe_printf_isr(const char *format, ...) {

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    va_list args;
    va_start(args, format);
    char *buffer  = pvPortMalloc(LOG_LENGTH * sizeof(char));
    int len = vsnprintf(buffer, LOG_LENGTH, format, args);
    va_end(args);

    if(xPrint_QueueHandle != NULL && len > 0) {
    	if(xQueueSendFromISR(xPrint_QueueHandle, &buffer, &xHigherPriorityTaskWoken) != pdPASS)
    	{

    	}
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
//普通和中断都可以使用的打印函数
void safe_printf_all(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *buffer  = pvPortMalloc(LOG_LENGTH * sizeof(char));
    int len = vsnprintf(buffer, LOG_LENGTH, format, args);
    va_end(args);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(__get_IPSR() != 0) {//中断中
		if(xPrint_QueueHandle != NULL && len > 0) {
			xQueueSendFromISR(xPrint_QueueHandle, &buffer, &xHigherPriorityTaskWoken);
		}
    }
    else
    {
		if(xPrint_QueueHandle != NULL && len > 0) {
			xQueueSend(xPrint_QueueHandle, &buffer, portMAX_DELAY);//pdMS_TO_TICKS(100)
		}
    }
}











