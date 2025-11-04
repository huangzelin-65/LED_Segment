#ifndef TASKS_INC_TASK_ROBOT_H_
#define TASKS_INC_TASK_ROBOT_H_

#include <stdbool.h>
#include <stdio.h>
#include "cJSON.h"

#define ERROR_LEVEL_LOW "ERROR_LEVEL_LOW"
#define ERROR_LEVEL_MEDIUM "ERROR_LEVEL_MEDIUM"
#define ERROR_LEVEL_HIGH "ERROR_LEVEL_HIGH"

#define ERROR_TYPE_NONE "ERROR_TYPE_NONE"
#define ERROR_TYPE_MOTOR "ERROR_TYPE_MOTOR"
#define ERROR_TYPE_SENSOR "ERROR_TYPE_SENSOR"
#define ERROR_TYPE_COMMUNICATION "ERROR_TYPE_COMMUNICATION"


typedef enum {
    DIRECTION_FORWARD,
    DIRECTION_BACKWARD
} Direction;

typedef struct {
    bool front;
    bool back;
} BumperState;

typedef struct {
    bool front;
    bool back;
} HallState;

typedef struct {
    bool runState;
    int runTime;
    int startTime;
    int setTime;
} DisinfectState;

typedef struct {
    int speedLevel;
    Direction direction;
} MoveState;

typedef struct {
    const char * errorType;
    const char * errorLevel;
} Errors;

// 定义动作参数结构体
typedef struct {
    char operating_mode[20];  // 操作模式
    int speed_level;          // 速度等级
    int direction;            // 方向
    DisinfectState disinfect_state;  // 消毒状态
} ActionParameters;

// 定义动作结构体
typedef struct {
    int action_type;          // 动作类型
    char *action_description; // 动作描述（可能为NULL）
    ActionParameters parameters;  // 动作参数
} Actions;

// 定义主数据结构体
typedef struct {
    int header_id;            // 头部ID
    int timestamp;            // 时间戳
    int version;              // 版本号
    char *manufacturer;       // 制造商（可能为NULL）
    char *serial_number;      // 序列号（可能为NULL）
    Actions actions;          // 动作
} RobotAction_t;

typedef struct {
    int headerId;
    int timestamp;
    int version;
    const char *manufacturer;
    const char *serialNumber;
    const char *orderId;
    const char *orderUpdateId;
    const char *lastNodeId;
    const char *lastNodeSequenceId;
    const char *nodeStates;
    const char *edgeStates;
    bool driving;
    const char *actionStates;
    const char *batteryState;
    const char *operatingMode;
    bool emergencyBtn;
    const char *rfid;
    const char *position;
    bool lockState;
    BumperState bumperState;
    HallState hallState;
    int runtime;
    MoveState moveState;
    DisinfectState disinfectState;
    Errors errors;
} RobotState_t;

extern RobotState_t robotSate;

void vRobotManagerTask(void *argument);
void vRobotReceiveTask(void *argument);

void Robot_Init(void);
void Robot_CreateStateJson(void);
void Robot_UpdateStateJson(cJSON* robotJson, const RobotState_t* robotState);

#endif