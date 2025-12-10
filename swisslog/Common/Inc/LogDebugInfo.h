#ifndef  LOGDEBUGINFO_H
#define  LOGDEBUGINFO_H

#include <stdio.h>
#include "main.h"
#include "FreeRTOS.h" 
#include "task.h"

#define LOG_USE_MALLOC 
#define LOG_ARRAY_LEN   8
#define LOG_LENGTH_LONG 400
#define LOG_LENGTH_SINGLE 4

#define DEBUGINFO(fmt, ...) \
    safe_printf_long("[%lu]%s(%d) " fmt"\r\n", \
                     xTaskGetTickCount(), \
                     __func__, \
                     __LINE__, \
                     ##__VA_ARGS__)
// #define DEBUGINFO(fmt, ...) safe_printf_long("[%s] " fmt"\r\n", __func__, ##__VA_ARGS__)
// #define DEBUGINFO_ISR(fmt, ...) safe_printf_isr("[%s] " fmt, __func__, ##__VA_ARGS__)
// #define DEBUGINFO_ALL(fmt, ...) safe_printf_all("[%s] " fmt"\n", __func__, ##__VA_ARGS__)

// #define DEBUGINFO(fmt, ...) printf("[%s] " fmt, __func__, ##__VA_ARGS__)

void safe_printf_long(const char *format, ...);
void safe_printf_single(const char *format, ...);
void vPrint_Array(uint8_t *array, uint8_t len);
#endif
