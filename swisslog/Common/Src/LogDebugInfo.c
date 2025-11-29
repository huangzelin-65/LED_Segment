#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdarg.h>
#include "main.h"
#include "LogDebugInfo.h"
#include "cmsis_os2.h"
#include <stdlib.h>

// 静态缓冲区
char printf_arr_long[LOG_ARRAY_LEN][LOG_LENGTH_LONG];
char printf_arr_single[LOG_ARRAY_LEN][LOG_LENGTH_SINGLE];

int printf_pos_long = 0;
int printf_pos_single = 0;

extern osMessageQueueId_t xPrint_QueueHandle;

extern UART_HandleTypeDef huart1;
//普通打印函数
void safe_printf_long(const char *format, ...) {
    va_list args;
    va_start(args, format);
    #ifdef LOG_USE_MALLOC
    char *buffer  = pvPortMalloc(LOG_LENGTH_LONG * sizeof(char));
    #else
    if(printf_pos_long >= LOG_ARRAY_LEN)
    {
        printf_pos_long = 0;
    }
    char *buffer = printf_arr_long[printf_pos_long];//取出对应位号的字符串数组
    printf_pos_long = (printf_pos_long + 1) % LOG_ARRAY_LEN;
    #endif
    if(buffer == NULL)return;//防止malloc失败
    int len = vsnprintf(buffer, LOG_LENGTH_LONG, format, args);
    // 手动添加终止符
    if (len >= LOG_LENGTH_LONG) {
        buffer[LOG_LENGTH_LONG - 1] = '\0';
        len = LOG_LENGTH_LONG - 1;
    }
    va_end(args);

    if(xPrint_QueueHandle != NULL && len > 0) {
        xQueueSend(xPrint_QueueHandle, &buffer, pdMS_TO_TICKS(100));
    }
}

void safe_printf_single(const char *format, ...) {
    va_list args;
    va_start(args, format);
    #ifdef LOG_USE_MALLOC
    char *buffer  = pvPortMalloc(LOG_LENGTH_SINGLE * sizeof(char));
    #else
    if(printf_pos_single >= LOG_ARRAY_LEN)
    {
        printf_pos_single = 0;
    }
    char *buffer = printf_arr_single[printf_pos_single];//取出对应位号的字符串数组
    printf_pos_single = (printf_pos_single + 1) % LOG_ARRAY_LEN;
    #endif    
    int len = vsnprintf(buffer, LOG_LENGTH_SINGLE, format, args);
    // 手动添加终止符
    if (len >= LOG_LENGTH_SINGLE) {
        buffer[LOG_LENGTH_SINGLE - 1] = '\0';
        len = LOG_LENGTH_SINGLE - 1;
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
    char *buffer  = pvPortMalloc(LOG_LENGTH_LONG * sizeof(char));
    int len = vsnprintf(buffer, LOG_LENGTH_LONG, format, args);
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
    char *buffer  = pvPortMalloc(LOG_LENGTH_LONG * sizeof(char));
    int len = vsnprintf(buffer, LOG_LENGTH_LONG, format, args);
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


void vPrint_Array(uint8_t *array, uint8_t len)
{
  uint8_t i;
  for(i=0; i<len; i++)
  {
    safe_printf_single("%X ",array[i]);
  }
  safe_printf_single("\r\n");
}











