#ifndef  LOGDEBUGINFO_H
#define  LOGDEBUGINFO_H

#include <stdio.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

// 核心宏定义（无需修改其他文件，与你的配置一致）
// #define LOG_USE_MALLOC  // 保持注释，禁用动态内存
#define LOG_ARRAY_LEN       10      // 静态缓冲区数量
#define LOG_LENGTH_LONG     1000    // 长日志缓冲区长度
#define LOG_LENGTH_SINGLE   4       // 短日志缓冲区长度

// 修复格式符错误：原宏中 %s(%d) 对应 __func__(%s) + __LINE__(%d)，多余一个%s
#define DEBUGINFO(fmt, ...) \
    safe_printf_long("[%s](%d) " fmt"\r\n", \
                     __func__, \
                     __LINE__, \
                     ##__VA_ARGS__)

// 函数声明（保持原有接口，无需修改调用处）
void safe_printf_long(const char *format, ...);
void safe_printf_single(const char *format, ...);
void vPrint_Array(uint8_t *array, uint8_t len);

#endif
