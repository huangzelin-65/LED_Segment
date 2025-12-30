#ifndef _INC_NOTIFY_H_
#define _INC_NOTIFY_H_
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char headerId[64];     // 最多4字符+":"+Long，预留长度
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
    char type[32];//通知类型
    char message[32];//通知消息
    int code;//通知参数
    int id;//流水号
    int execute;//代表是否执行。0：待执行 1：已经执行   
} Notify_t;

void Notify_Event(Notify_t *notify);
void Notify_Execute(void);
void Notify_ToCmd(Notify_t* notify);
void Notify_Update(int id);
void Notify_Edit(Notify_t *notify);
#endif
