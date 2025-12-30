#ifndef _INC_CONFIG_H_
#define _INC_CONFIG_H_
#include <stdbool.h>
#include <stdio.h>

typedef struct {
    char headerId[64];     // 最多4字符+":"+Long，预留长度
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
    char name[32];//功能名称
    char value[32];//功能参数
    char params[32];//功能单位
    char cause[32];//执行出错时的原因
    int id;//流水号
    int execute;//代表是否执行。0：待执行 1：已经执行   
    int code;//执行结果 
} Config_t;

void Config_Event(Config_t *config);
void Config_Execute(void);
void Config_ToCmd(Config_t* config);
void Config_Update(int id);
void Config_Edit(Config_t *config);
#endif
