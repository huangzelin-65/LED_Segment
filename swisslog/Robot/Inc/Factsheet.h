#ifndef INC_FACTSHEET_H_
#define INC_FACTSHEET_H_
#include <stdbool.h>
#include <stdio.h>
#include "include_defs.h"
typedef struct {
    char headerId[HEAD_ID_LENGTH];     // 最多4字符+":"+Long，预留长度
    char timestamp[TIMESTAMP_LENGTH];   // Unix毫秒时间戳（字符串形式）
    char version[VERSION_LENGTH];     // 版本号（如"1.0.0"）
} Factsheet_t;

void Factsheet_Event(void);

#endif
