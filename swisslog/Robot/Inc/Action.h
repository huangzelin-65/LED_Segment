#ifndef INC_ACTION_H_
#define INC_ACTION_H_
#include <stdbool.h>
#include <stdio.h>
#include "include_defs.h"

typedef struct {
    char model[MODEL_LENGTH];  // 存储"auto"或"manual"，预留足够长度
    char speedLevel[SPEEDLEVEL_LENGTH];//速度数值
}CmdParams;

typedef struct {
    char cmd[CMD_LENGTH];     // 存储"forward"/"back"/"stop"/"runModel"
    char cmdId[CMD_ID_LENGTH];   // 存储命令ID（如"1"）
    CmdParams params;  // 命令参数
    char status[24];  //执行结果
} Cmd_t;

typedef struct {
    int Type;             // 动作类型（如1）
    Cmd_t cmds[8];        // 命令数组（假设最多8条命令）
    int cmd_count;        // 实际命令数量
} ActionParams;

typedef struct {
    char headerId[HEAD_ID_LENGTH];    // 最多4字符+":"+Long，预留长度
    char timestamp[TIMESTAMP_LENGTH];   // Unix毫秒时间戳（字符串形式）
    char version[VERSION_LENGTH];     // 版本号（如"1.0.0"）
    int id;//流水号
    int Type; 
    Cmd_t cmd;
    int execute;//代表是否执行。0：待执行 1：已经执行
    int running;//代表机器是否在执行动作
    uint32_t curPos;//当前最新的标签   
} Action_t;	

typedef enum {
    ACTION_STATUS_ACK = 0, 
    ACTION_STATUS_RUNNING,
    ACTION_STATUS_FINISHED,
    ACTION_STATUS_FAILED,
    ACTION_STATUS_PAUSE
} ActionStatus_t;

void Action_ListInit(void);
void Action_Init(void);
void Action_Event(Action_t *action);
void Action_Execute(void);
void Action_ToCmd(Action_t *action);
void Action_Update(int id);
#endif
