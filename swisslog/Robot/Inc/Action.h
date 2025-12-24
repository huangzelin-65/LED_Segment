#ifndef INC_ACTION_H_
#define INC_ACTION_H_
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char model[16];  // 存储"auto"或"manual"，预留足够长度
    char speedLevel[16];//速度数值
}CmdParams;

typedef struct {
    char cmd[16];     // 存储"forward"/"back"/"stop"/"runModel"
    char cmdId[64];   // 存储命令ID（如"1"）
    CmdParams params;  // 命令参数
} Cmd_t;

typedef struct {
    int Type;             // 动作类型（如1）
    Cmd_t cmds[8];        // 命令数组（假设最多8条命令）
    int cmd_count;        // 实际命令数量
} ActionParams;

typedef struct {
    char headerId[64];    // 最多4字符+":"+Long，预留长度
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
    ActionParams action;   // 动作内容
} Action_t;	

typedef enum {
    ACTION_STATUS_ACK = 0, 
    ACTION_STATUS_RUNNING,
    ACTION_STATUS_FINISHED,
    ACTION_STATUS_FAILED,
    ACTION_STATUS_PAUSE
} ActionStatus_t;

void Action_ListInit(void);


#endif
