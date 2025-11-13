#ifndef ADAPTOR_INC_ADAPTOR_ROBOT_H_
#define ADAPTOR_INC_ADAPTOR_ROBOT_H_
#include <stdbool.h>
#include "cJSON.h"
#include <stdio.h>

#define ERROR_LEVEL_LOW "ERROR_LEVEL_LOW"
#define ERROR_LEVEL_MEDIUM "ERROR_LEVEL_MEDIUM"
#define ERROR_LEVEL_HIGH "ERROR_LEVEL_HIGH"

#define ERROR_TYPE_NONE "ERROR_TYPE_NONE"
#define ERROR_TYPE_MOTOR "ERROR_TYPE_MOTOR"
#define ERROR_TYPE_SENSOR "ERROR_TYPE_SENSOR"
#define ERROR_TYPE_COMMUNICATION "ERROR_TYPE_COMMUNICATION"

#define MODE_TYPE_AUTO      "AUTO"
#define MODE_TYPE_MANUAL    "MANUAL"

typedef enum {
    DIRECTION_STOP = 0,
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

typedef struct {
    char cmd[16];     // 存储"forward"/"back"/"stop"/"runModel"
    char cmdId[16];   // 存储命令ID（如"1"）
    char status[24];  // 动作执行状态
} RobotCmdAck;

typedef struct {
    char headerId[8];     // 最多4字符+":"+Long，预留长度
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
    RobotCmdAck actionStates[8];   // 动作内容，最多8组命令
    int states_count;
} RobotActionAck_t;

typedef struct {
    char headerId[8];
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
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
    int car_running;
    uint32_t curPos;
    bool lockState;
    BumperState bumperState;
    HallState hallState;
    int runtime;
    MoveState moveState;
    DisinfectState disinfectState;
    Errors errors;
    uint32_t UID[3];
    char client_id[64];
    RobotActionAck_t robotActionAck;
    void* mutex;           // 线程安全锁（FreeRTOS互斥锁）
} RobotState_t;



// 命令参数结构体（对应params字段）
typedef struct {
    char model[16];  // 存储"auto"或"manual"，预留足够长度
} RobotCmdParams;

// 命令结构体（对应cmds数组中的元素）
typedef struct {
    char cmd[16];     // 存储"forward"/"back"/"stop"/"runModel"
    char cmdId[16];   // 存储命令ID（如"1"）
    RobotCmdParams params;  // 命令参数
} RobotCmd;

// 动作结构体（对应action字段）
typedef struct {
    int Type;             // 动作类型（如1）
    RobotCmd cmds[8];     // 命令数组（假设最多8条命令）
    int cmd_count;        // 实际命令数量
} RobotAction;

// 顶层结构体（对应整个JSON）
typedef struct {
    char headerId[8];     // 最多4字符+":"+Long，预留长度
    char timestamp[32];   // Unix毫秒时间戳（字符串形式）
    char version[16];     // 版本号（如"1.0.0"）
    RobotAction action;   // 动作内容
} RobotAction_t;	

typedef enum {
    ROBOT_MSG_HEART = 0,//发送心跳包到服务器
    ROBOT_MSG_STATE,//代表需要把消息发送到服务器
    ROBOT_MSG_PARSE,//代表从服务器获取到消息
    ROBOT_MSG_ACTION_STATUS,//回复下发的action的状态
} RobotMsgType_t;


typedef struct {
    RobotMsgType_t type;
    char *data;
}RobotMsg_t;

typedef enum {
    ROBOT_ACTION_STATUS_ACK = 0,//回复action ack
    ROBOT_ACTION_STATUS_RUNNING,
    ROBOT_ACTION_STATUS_FINISHED,
    ROBOT_ACTION_STATUS_FAILED
} RobotActionStatus_t;

extern RobotAction_t robotAction;
extern RobotState_t robotSate;
extern bool robot_init;
extern cJSON* RobotJson;

void Robot_Init(void);
void Robot_CreateStateJson(void);
void Robot_UpdateStateJson(cJSON* robotJson, const RobotState_t* robotState);
void Robot_CreateHeartBeatJson(void);
char* Robot_GetHeartBeatJsonStr(void);
int Robot_ParseJson(char* json_str,RobotAction_t *robot);
char* Robot_GetStateJsonStr(void);
void Robot_SendMsg(RobotMsgType_t type,void *data);
void Robot_UpdateState(void);
void Robot_Action2Cmd(void);
void Robot_ActionAckUpdate(RobotActionStatus_t status);
void Robot_Event(void);
void Robot_State(void);
void Robot_ActionAck(void);
#endif