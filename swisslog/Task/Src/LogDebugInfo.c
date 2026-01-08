#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdarg.h>
#include "main.h"
#include "LogDebugInfo.h"
#include "cmsis_os2.h"
#include <stdlib.h>

// 静态缓冲区（循环复用，无内存泄漏）
static char printf_arr_long[LOG_ARRAY_LEN][LOG_LENGTH_LONG];
static char printf_arr_single[LOG_ARRAY_LEN][LOG_LENGTH_SINGLE];

// 缓冲区索引（静态变量，仅本文件可见）
static int printf_pos_long = 0;
static int printf_pos_single = 0;

// 串口2句柄（确保main.h中已定义huart2且完成初始化）
extern UART_HandleTypeDef huart2;

/**
 * @brief  长日志安全打印函数（串口2输出，多任务安全，支持格式化）
 * @param  format: 格式化字符串（同printf）
 * @param  ...: 可变参数
 */
void safe_printf_long(const char *format, ...) {
    if (format == NULL) return;

    // 进入FreeRTOS临界区：防止多任务同时操作缓冲区/串口，避免打印错乱
    taskENTER_CRITICAL();

    // 1. 循环获取静态缓冲区（索引越界自动重置）
    char *buffer = printf_arr_long[printf_pos_long];
    printf_pos_long = (printf_pos_long + 1) % LOG_ARRAY_LEN;

    // 2. 清空缓冲区（避免残留脏数据）
    memset(buffer, 0, LOG_LENGTH_LONG);

    // 3. 格式化字符串（支持printf风格的可变参数）
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, LOG_LENGTH_LONG, format, args);
    // 缓冲区溢出保护：手动添加终止符
    if (len >= LOG_LENGTH_LONG) {
        buffer[LOG_LENGTH_LONG - 1] = '\0';
        len = LOG_LENGTH_LONG - 1;
    }
    va_end(args);

    // 4. 串口2直接发送（缩短超时到10ms，避免任务阻塞）
    if (len > 0) {
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, 10);
    }

    // 退出临界区：恢复多任务调度
    taskEXIT_CRITICAL();
}

/**
 * @brief  短日志安全打印函数（串口2输出，适配短缓冲区）
 * @param  format: 格式化字符串
 * @param  ...: 可变参数
 */
void safe_printf_single(const char *format, ...) {
    if (format == NULL) return;

    taskENTER_CRITICAL();

    // 1. 循环获取短缓冲区
    char *buffer = printf_arr_single[printf_pos_single];
    printf_pos_single = (printf_pos_single + 1) % LOG_ARRAY_LEN;

    // 2. 清空缓冲区
    memset(buffer, 0, LOG_LENGTH_SINGLE);

    // 3. 格式化字符串
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, LOG_LENGTH_SINGLE, format, args);
    if (len >= LOG_LENGTH_SINGLE) {
        buffer[LOG_LENGTH_SINGLE - 1] = '\0';
        len = LOG_LENGTH_SINGLE - 1;
    }
    va_end(args);

    // 4. 串口2发送
    if (len > 0) {
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, len, 10);
    }

    taskEXIT_CRITICAL();
}

/**
 * @brief  uint8_t数组转十六进制字符串（带空格），串口2打印
 * @param  array: 源数组
 * @param  len: 数组长度
 */
void vPrint_Array(uint8_t *array, uint8_t len) {
    if (array == NULL || len == 0) return;

    // 目标缓冲区：3*len 满足 "XX XX XX" 格式（含终止符）
    char print_str[3 * len];
    memset(print_str, 0, sizeof(print_str));

    // 数组转十六进制字符串
    for (uint8_t i = 0; i < len; i++) {
        if (i == len - 1) {
            snprintf(print_str + 3*i, 3, "%02X", array[i]);
        } else {
            snprintf(print_str + 3*i, 4, "%02X ", array[i]);
        }
    }

    // 调用长日志函数打印（串口2输出）
    safe_printf_long("%s\r\n", print_str);
}
