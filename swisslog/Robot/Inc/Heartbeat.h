#ifndef INC_HEARTBEAT_H_
#define INC_HEARTBEAT_H_
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char headerId[64];     // 最多4字符+":"+Long，预留长度
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
} HeartBeat_t;

void Heart_Event(void);

#endif
