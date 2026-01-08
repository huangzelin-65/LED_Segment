#ifndef  INCLUDE_DEFS_H
#define  INCLUDE_DEFS_H

#define HEAD_ID_LENGTH      64
#define TIMESTAMP_LENGTH    32
#define VERSION_LENGTH      16
#define CMD_LENGTH          16
#define CMD_ID_LENGTH       64
#define NAME_LENGTH         32
#define VALUE_LENGTH        32
#define PARAMS_LENGTH       32
#define TYPE_LENGTH         32
#define MESSAGE_LENGTH      32
#define MODEL_LENGTH        16
#define SPEEDLEVEL_LENGTH   16
#define SUBTOPIC_LENGTH     64
#define CAUSE_LENGTH        32
#define IP_LENGTH           36
#define DEVICE_CODE_LENGTH  50
#define USER_LENGTH         30
#define PWD_LENGTH          30
#define SN_LENGTH           50

// 定义静态二维数组的尺寸（适配需求：最多8个参数，每个参数最长32字符+1个'\0'）
#define PARAM_MAX_COUNT 8   // params最多8个元素
#define PARAM_BUF_LEN   (PARAMS_LENGTH + 1)  // 包含'\0'的总长度

#include "Task_MotionCtrl.h"
#include "Task_BoxCtrl.h"
#include "adaptor_wifi.h"
#include "adaptor_mqtt.h"
#include "adaptor_ntp.h"
#include "Register.h"
#include "app_freertos.h"
#include "DwinHMI.h"
#endif



