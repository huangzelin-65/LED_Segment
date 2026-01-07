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

#include "Task_MotionCtrl.h"
#include "Task_BoxCtrl.h"
#include "adaptor_wifi.h"
#include "adaptor_mqtt.h"
#include "adaptor_ntp.h"
#include "Register.h"
#include "app_freertos.h"

#endif



