/*
 * common.h
 *
 *  Created on: Jun 27, 2025
 *      Author: e3lijia25d
 */

#ifndef COMMON_INC_COMMON_H_
#define COMMON_INC_COMMON_H_

#include "main.h"

// 转换状态枚举
typedef enum {
    CONVERT_SUCCESS,        // 转换成功
    CONVERT_NULL_PTR,       // 字符串为空指针
    CONVERT_INVALID_START,  // 起始位置无效
    CONVERT_INVALID_LENGTH, // 长度无效
    CONVERT_NON_DIGIT,      // 包含非数字字符
    CONVERT_OVERFLOW        // 数值溢出
} ConvertStatus;

u32 substring_to_uint(char* str, u16 start, u16 length);

void vParseCommandToCar();

#endif /* COMMON_INC_COMMON_H_ */
