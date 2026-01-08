#ifndef INC_FEATURE_H_
#define INC_FEATURE_H_
#include <stdbool.h>
#include <stdio.h>
#include "include_defs.h"
typedef struct {
    char headerId[HEAD_ID_LENGTH];     // 最多4字符+":"+Long，预留长度
    char timestamp[TIMESTAMP_LENGTH];   // Unix毫秒时间戳（字符串形式）
    char version[VERSION_LENGTH];     // 版本号（如"1.0.0"）
    char name[NAME_LENGTH];//功能名称
    char value[VALUE_LENGTH];//功能参数
    char params[PARAM_MAX_COUNT][PARAM_BUF_LEN];//功能单位
    char cause[CAUSE_LENGTH];//执行出错时的原因
    int mode;//小车模式
    int screen_lock;//车厢屏幕状态
    int id;//流水号
    int execute;//代表是否执行。0：待执行 1：已经执行   
    int code;//执行结果
    int params_cnt;//参数的数量 
} Feature_t;

void Feature_Event(Feature_t *feature);
void Feature_Execute(void);
void Feature_ToCmd(Feature_t* feature);
void Feature_Update(int id);
void Feature_Edit(Feature_t *feature);
#endif
