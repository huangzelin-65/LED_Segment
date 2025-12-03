#include <stdio.h>
#include <string.h>
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

/**
 * @brief  uint8_t数组（十六进制数据）→ 带空格分隔的十六进制字符串（用于打印）
 * @param  src:       源uint8_t数组（存储十六进制数据）
 * @param  src_len:   源数组长度
 * @param  dst:       目标字符串缓冲区（存储转换结果）
 * @param  dst_len:   目标缓冲区最大长度（需≥3*src_len，否则转换失败）
 * @return int:       0=成功，-1=失败（参数错误/缓冲区不足）
 */
int uint8_hex_to_space_str(const uint8_t* src, uint32_t src_len, char* dst, uint32_t dst_len) {
    // 1. 参数校验（避免空指针、缓冲区不足）
    if (src == NULL || dst == NULL || src_len == 0) {
        DEBUGINFO("Error: Source array/target buffer is null or the array length is 0");
        return -1;
    }
    // 缓冲区最小需求：3*src_len（3*n = 2位十六进制+1空格（最后1个字节无空格） + '\0'）
    if (dst_len < 3 * src_len) {
        DEBUGINFO("Error: Insufficient buffer! At least %d bytes are required, only %d bytes available", 3*src_len, dst_len);
        return -1;
    }

    // 2. 清空缓冲区（避免残留脏数据）
    memset(dst, 0, dst_len);

    // 3. 循环转换每个字节（最后1个字节不加空格）
    for (uint32_t i = 0; i < src_len; i++) {
        if (i == src_len - 1) {
            // 最后1个字节：仅格式化2位十六进制（无空格）
            snprintf(dst + 3*i, 3, "%02X", src[i]);
        } else {
            // 中间字节：格式化2位十六进制 + 空格（占3个字符位置）
            snprintf(dst + 3*i, 4, "%02X ", src[i]);
        }
    }

    return 0;
}


void vPrint_Array(uint8_t *array, uint8_t len)
{
    // 目标字符串缓冲区
    char print_str[3 * len];

    // 转换并检查结果
    if (uint8_hex_to_space_str(array, len, print_str, sizeof(print_str)) == 0) {
        safe_printf_long("%s\r\n", print_str);
    }
}











