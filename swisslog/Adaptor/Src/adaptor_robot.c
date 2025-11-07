#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_robot.h"
#include "LogDebugInfo.h"
#include "queue.h"
#include "common.h"

extern ServerToCarData_t ServerToCarData;
extern osMessageQueueId_t xRobotQueueHandle;//该消息队列处理事件上报
extern osThreadId_t RobotReceiveTaskHandle;//该任务处理事件上报

//保存服务器下发的动作消息
RobotAction_t robotAction;

//此处根据不同的设备，定义不同的变量
RobotState_t robotSate = {
    .headerId = 1,
    .timestamp = 0,
    .version = 1,
    .manufacturer = NULL,
    .serialNumber = NULL,
    .orderId = NULL,
    .orderUpdateId = NULL,
    .lastNodeId = NULL,
    .lastNodeSequenceId = NULL,
    .nodeStates = NULL,
    .edgeStates = NULL,
    .actionStates = NULL,
    .batteryState = NULL,
    .operatingMode = NULL,
    .rfid = NULL,
    .position = NULL,
    .lockState = false,
    .bumperState = {false, false},
    .hallState = {false, false},
    .runtime = 0,
    .moveState = {0, DIRECTION_FORWARD},
    .disinfectState = {false, 10, 20, 30},
    .errors = {ERROR_TYPE_NONE, ERROR_LEVEL_LOW}
};

//保留创建的robot json 的指针
cJSON* RobotJson = NULL;

//保存robot初始化状态
bool robot_init = false;

//保留创建的heart beat json 的指针
cJSON* Robot_HeartBeatJson = NULL;

// 辅助函数：将Direction枚举转换为字符串
const char* Robot_DirectionToString(Direction dir) 
{
    switch (dir) {
        case DIRECTION_FORWARD: return "DIRECTION_FORWARD";
        case DIRECTION_BACKWARD: return "DIRECTION_BACKWARD";
        default: return "UNKNOWN_DIRECTION";
    }
}
//创建vda5050必要字段，赋值为NULL
void Robot_AddNullFields(cJSON* root) 
{
    const char* null_fields[] = {
        "manufacturer", "serialNumber", "orderId", "orderUpdateId",
        "lastNodeId", "lastNodeSequenceId", "nodeStates", "edgeStates",
        "driving", "actionStates", "batteryState"
    };
    for(int i=0; i<sizeof(null_fields)/sizeof(null_fields[0]); i++) {
        cJSON_AddNullToObject(root, null_fields[i]);
    }
}
//小车前后碰把开关的状态
cJSON* Robot_CreateBumperState()
{
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "front", robotSate.bumperState.front);
    cJSON_AddBoolToObject(obj, "back", robotSate.bumperState.back);
    return obj;
}
//小车速度的状态
cJSON* Robot_CreateMoveState()
{
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "speedLevel", robotSate.moveState.speedLevel);
    cJSON_AddNumberToObject(obj, "direction", robotSate.moveState.direction);
    return obj;
}
//车厢消毒净化状态
cJSON* Robot_CreateDisinfectState() 
{
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "runState", robotSate.disinfectState.runState);
    cJSON_AddNumberToObject(obj, "runTime", robotSate.disinfectState.runTime);
    cJSON_AddNumberToObject(obj, "startTime", robotSate.disinfectState.startTime);
    cJSON_AddNumberToObject(obj, "setTime", robotSate.disinfectState.setTime);
    return obj;
}
//小车报错的消息
cJSON* Robot_CreateErrors() 
{
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddStringToObject(obj, "errorType", robotSate.errors.errorType);
    cJSON_AddStringToObject(obj, "errorLevel", robotSate.errors.errorLevel);
    return obj;
}
//robot json 创建之后不做释放，避免重复申请和释放内存，造成碎片化
void Robot_CreateStateJson(void) 
{
    DEBUGINFO("start\n");

    if(RobotJson != NULL)   
    {
        DEBUGINFO("RobotJson has been created\n");
        return;
    }
    // 1. 创建根对象
    RobotJson = cJSON_CreateObject();
    if(RobotJson == NULL)
    {
        DEBUGINFO("cJSON_CreateObject RobotJson fail\n");
        return;
    }

    // 2. 添加基础字段
    cJSON_AddNumberToObject(RobotJson, "headerId", robotSate.headerId);
    cJSON_AddNumberToObject(RobotJson, "timestamp", robotSate.timestamp);
    cJSON_AddNumberToObject(RobotJson, "version", robotSate.version);
    cJSON_AddStringToObject(RobotJson, "operatingMode", robotSate.operatingMode);
    cJSON_AddBoolToObject(RobotJson, "emergencyBtn", robotSate.emergencyBtn);
    cJSON_AddStringToObject(RobotJson, "rfid", robotSate.rfid);
    cJSON_AddStringToObject(RobotJson, "position", robotSate.position);
    cJSON_AddBoolToObject(RobotJson, "lockState", robotSate.lockState);
    cJSON_AddNumberToObject(RobotJson, "runtime", robotSate.runtime);
    
    // 3. 添加NULL字段
    Robot_AddNullFields(RobotJson);
    
    // 4. 添加嵌套对象
    cJSON_AddItemToObject(RobotJson, "bumperState", Robot_CreateBumperState());
    cJSON_AddItemToObject(RobotJson, "moveState", Robot_CreateMoveState());
    cJSON_AddItemToObject(RobotJson, "disinfectState", Robot_CreateDisinfectState());
    cJSON_AddItemToObject(RobotJson, "errors", Robot_CreateErrors());

    DEBUGINFO("end\n");
}
// 更新已有cJSON对象（RobotJson）为robotState的最新状态
void Robot_UpdateStateJson(cJSON* robotJson, const RobotState_t* robotState) {
    if (robotJson == NULL || robotState == NULL) {
        return; // 入参无效，直接返回
    }

    // 1. 更新int类型成员
    cJSON* headerIdItem = cJSON_CreateNumber(robotState->headerId);
    if (headerIdItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "headerId", headerIdItem);
        if (!replaceRet) { // 替换失败，主动销毁新创建的项
            cJSON_Delete(headerIdItem);
        }
    }

    cJSON* timestampItem = cJSON_CreateNumber(robotState->timestamp);
    if (timestampItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "timestamp", timestampItem);
        if (!replaceRet) {
            cJSON_Delete(timestampItem);
        }
    }

    cJSON* versionItem = cJSON_CreateNumber(robotState->version);
    if (versionItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "version", versionItem);
        if (!replaceRet) {
            cJSON_Delete(versionItem);
        }
    }

    cJSON* runtimeItem = cJSON_CreateNumber(robotState->runtime);
    if (runtimeItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "runtime", runtimeItem);
        if (!replaceRet) {
            cJSON_Delete(runtimeItem);
        }
    }

    // 2. 更新字符串类型成员（处理NULL情况）
    const char* manufacturerStr = robotState->manufacturer ? robotState->manufacturer : "";
    cJSON* manufacturerItem = cJSON_CreateString(manufacturerStr);
    if (manufacturerItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "manufacturer", manufacturerItem);
        if (!replaceRet) {
            cJSON_Delete(manufacturerItem);
        }
    }

    const char* serialNumberStr = robotState->serialNumber ? robotState->serialNumber : "";
    cJSON* serialNumberItem = cJSON_CreateString(serialNumberStr);
    if (serialNumberItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "serialNumber", serialNumberItem);
        if (!replaceRet) {
            cJSON_Delete(serialNumberItem);
        }
    }

    const char* orderIdStr = robotState->orderId ? robotState->orderId : "";
    cJSON* orderIdItem = cJSON_CreateString(orderIdStr);
    if (orderIdItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "orderId", orderIdItem);
        if (!replaceRet) {
            cJSON_Delete(orderIdItem);
        }
    }

    const char* orderUpdateIdStr = robotState->orderUpdateId ? robotState->orderUpdateId : "";
    cJSON* orderUpdateIdItem = cJSON_CreateString(orderUpdateIdStr);
    if (orderUpdateIdItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "orderUpdateId", orderUpdateIdItem);
        if (!replaceRet) {
            cJSON_Delete(orderUpdateIdItem);
        }
    }

    const char* lastNodeIdStr = robotState->lastNodeId ? robotState->lastNodeId : "";
    cJSON* lastNodeIdItem = cJSON_CreateString(lastNodeIdStr);
    if (lastNodeIdItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "lastNodeId", lastNodeIdItem);
        if (!replaceRet) {
            cJSON_Delete(lastNodeIdItem);
        }
    }

    const char* lastNodeSequenceIdStr = robotState->lastNodeSequenceId ? robotState->lastNodeSequenceId : "";
    cJSON* lastNodeSequenceIdItem = cJSON_CreateString(lastNodeSequenceIdStr);
    if (lastNodeSequenceIdItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "lastNodeSequenceId", lastNodeSequenceIdItem);
        if (!replaceRet) {
            cJSON_Delete(lastNodeSequenceIdItem);
        }
    }

    const char* nodeStatesStr = robotState->nodeStates ? robotState->nodeStates : "";
    cJSON* nodeStatesItem = cJSON_CreateString(nodeStatesStr);
    if (nodeStatesItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "nodeStates", nodeStatesItem);
        if (!replaceRet) {
            cJSON_Delete(nodeStatesItem);
        }
    }

    const char* edgeStatesStr = robotState->edgeStates ? robotState->edgeStates : "";
    cJSON* edgeStatesItem = cJSON_CreateString(edgeStatesStr);
    if (edgeStatesItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "edgeStates", edgeStatesItem);
        if (!replaceRet) {
            cJSON_Delete(edgeStatesItem);
        }
    }

    const char* actionStatesStr = robotState->actionStates ? robotState->actionStates : "";
    cJSON* actionStatesItem = cJSON_CreateString(actionStatesStr);
    if (actionStatesItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "actionStates", actionStatesItem);
        if (!replaceRet) {
            cJSON_Delete(actionStatesItem);
        }
    }

    const char* batteryStateStr = robotState->batteryState ? robotState->batteryState : "";
    cJSON* batteryStateItem = cJSON_CreateString(batteryStateStr);
    if (batteryStateItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "batteryState", batteryStateItem);
        if (!replaceRet) {
            cJSON_Delete(batteryStateItem);
        }
    }

    const char* operatingModeStr = robotState->operatingMode ? robotState->operatingMode : "";
    cJSON* operatingModeItem = cJSON_CreateString(operatingModeStr);
    if (operatingModeItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "operatingMode", operatingModeItem);
        if (!replaceRet) {
            cJSON_Delete(operatingModeItem);
        }
    }

    const char* rfidStr = robotState->rfid ? robotState->rfid : "";
    cJSON* rfidItem = cJSON_CreateString(rfidStr);
    if (rfidItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "rfid", rfidItem);
        if (!replaceRet) {
            cJSON_Delete(rfidItem);
        }
    }

    const char* positionStr = robotState->position ? robotState->position : "";
    cJSON* positionItem = cJSON_CreateString(positionStr);
    if (positionItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "position", positionItem);
        if (!replaceRet) {
            cJSON_Delete(positionItem);
        }
    }

    // 3. 更新bool类型成员
    cJSON* drivingItem = robotState->driving ? cJSON_CreateTrue() : cJSON_CreateFalse();
    if (drivingItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "driving", drivingItem);
        if (!replaceRet) {
            cJSON_Delete(drivingItem);
        }
    }

    cJSON* emergencyBtnItem = robotState->emergencyBtn ? cJSON_CreateTrue() : cJSON_CreateFalse();
    if (emergencyBtnItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "emergencyBtn", emergencyBtnItem);
        if (!replaceRet) {
            cJSON_Delete(emergencyBtnItem);
        }
    }

    cJSON* lockStateItem = robotState->lockState ? cJSON_CreateTrue() : cJSON_CreateFalse();
    if (lockStateItem != NULL) {
        cJSON_bool replaceRet = cJSON_ReplaceItemInObject(robotJson, "lockState", lockStateItem);
        if (!replaceRet) {
            cJSON_Delete(lockStateItem);
        }
    }

    // 4. 更新BumperState（front/back均为bool）
    cJSON* bumperJson = cJSON_GetObjectItem(robotJson, "bumperState");
    if (bumperJson != NULL) {
        cJSON* frontItem = robotState->bumperState.front ? cJSON_CreateTrue() : cJSON_CreateFalse();
        if (frontItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(bumperJson, "front", frontItem);
            if (!replaceRet) {
                cJSON_Delete(frontItem);
            }
        }

        cJSON* backItem = robotState->bumperState.back ? cJSON_CreateTrue() : cJSON_CreateFalse();
        if (backItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(bumperJson, "back", backItem);
            if (!replaceRet) {
                cJSON_Delete(backItem);
            }
        }
    }

    // 5. 更新HallState（front/back均为bool）
    cJSON* hallJson = cJSON_GetObjectItem(robotJson, "hallState");
    if (hallJson != NULL) {
        cJSON* frontItem = robotState->hallState.front ? cJSON_CreateTrue() : cJSON_CreateFalse();
        if (frontItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(hallJson, "front", frontItem);
            if (!replaceRet) {
                cJSON_Delete(frontItem);
            }
        }

        cJSON* backItem = robotState->hallState.back ? cJSON_CreateTrue() : cJSON_CreateFalse();
        if (backItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(hallJson, "back", backItem);
            if (!replaceRet) {
                cJSON_Delete(backItem);
            }
        }
    }

    // 6. 更新DisinfectState（runState为bool，其余为int）
    cJSON* disinfectJson = cJSON_GetObjectItem(robotJson, "disinfectState");
    if (disinfectJson != NULL) {
        cJSON* runStateItem = robotState->disinfectState.runState ? cJSON_CreateTrue() : cJSON_CreateFalse();
        if (runStateItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(disinfectJson, "runState", runStateItem);
            if (!replaceRet) {
                cJSON_Delete(runStateItem);
            }
        }

        cJSON* runTimeItem = cJSON_CreateNumber(robotState->disinfectState.runTime);
        if (runTimeItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(disinfectJson, "runTime", runTimeItem);
            if (!replaceRet) {
                cJSON_Delete(runTimeItem);
            }
        }

        cJSON* startTimeItem = cJSON_CreateNumber(robotState->disinfectState.startTime);
        if (startTimeItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(disinfectJson, "startTime", startTimeItem);
            if (!replaceRet) {
                cJSON_Delete(startTimeItem);
            }
        }

        cJSON* setTimeItem = cJSON_CreateNumber(robotState->disinfectState.setTime);
        if (setTimeItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(disinfectJson, "setTime", setTimeItem);
            if (!replaceRet) {
                cJSON_Delete(setTimeItem);
            }
        }
    }

    // 7. 更新MoveState（speedLevel为int，direction为枚举）
    cJSON* moveJson = cJSON_GetObjectItem(robotJson, "moveState");
    if (moveJson != NULL) {
        cJSON* speedLevelItem = cJSON_CreateNumber(robotState->moveState.speedLevel);
        if (speedLevelItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(moveJson, "speedLevel", speedLevelItem);
            if (!replaceRet) {
                cJSON_Delete(speedLevelItem);
            }
        }

        const char* directionStr = Robot_DirectionToString(robotState->moveState.direction);
        cJSON* directionItem = cJSON_CreateString(directionStr);
        if (directionItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(moveJson, "direction", directionItem);
            if (!replaceRet) {
                cJSON_Delete(directionItem);
            }
        }
    }

    // 8. 更新Errors（errorType和errorLevel为字符串）
    cJSON* errorsJson = cJSON_GetObjectItem(robotJson, "errors");
    if (errorsJson != NULL) {
        const char* errorTypeStr = robotState->errors.errorType ? robotState->errors.errorType : "";
        cJSON* errorTypeItem = cJSON_CreateString(errorTypeStr);
        if (errorTypeItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(errorsJson, "errorType", errorTypeItem);
            if (!replaceRet) {
                cJSON_Delete(errorTypeItem);
            }
        }

        const char* errorLevelStr = robotState->errors.errorLevel ? robotState->errors.errorLevel : "";
        cJSON* errorLevelItem = cJSON_CreateString(errorLevelStr);
        if (errorLevelItem != NULL) {
            cJSON_bool replaceRet = cJSON_ReplaceItemInObject(errorsJson, "errorLevel", errorLevelItem);
            if (!replaceRet) {
                cJSON_Delete(errorLevelItem);
            }
        }
    }
}
//解析服务器发来的json数据
int Robot_ParseJson(char *json_str,RobotAction_t *robot) 
{
    TickType_t start_tick = xTaskGetTickCount();
    DEBUGINFO("start:%ld\n",start_tick);
    // 解析JSON数据
    if (json_str == NULL || robot == NULL) {
        DEBUGINFO("parameters null\n");
        return -1;
    }

    // 1. 解析整个JSON
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL) {
        DEBUGINFO("cJSON_Parse fail\n");
        return -1;
    }

    // 2. 解析顶层字段: headerId
    cJSON *headerId = cJSON_GetObjectItem(root, "headerId");
    if (headerId == NULL || !cJSON_IsString(headerId)) {
        DEBUGINFO("headerId Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    strncpy(robot->headerId, headerId->valuestring, sizeof(robot->headerId)-1);
    robot->headerId[sizeof(robot->headerId)-1] = '\0';  // 确保字符串终止

    // 3. 解析顶层字段: timestamp
    cJSON *timestamp = cJSON_GetObjectItem(root, "timestamp");
    if (timestamp == NULL || !cJSON_IsString(timestamp)) {
        DEBUGINFO("timestamp Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    strncpy(robot->timestamp, timestamp->valuestring, sizeof(robot->timestamp)-1);
    robot->timestamp[sizeof(robot->timestamp)-1] = '\0';

    // 4. 解析顶层字段: version
    cJSON *version = cJSON_GetObjectItem(root, "version");
    if (version == NULL || !cJSON_IsString(version)) {
        DEBUGINFO("version Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    strncpy(robot->version, version->valuestring, sizeof(robot->version)-1);
    robot->version[sizeof(robot->version)-1] = '\0';

    // 5. 解析action对象
    cJSON *action_obj = cJSON_GetObjectItem(root, "action");
    if (action_obj == NULL || !cJSON_IsObject(action_obj)) {
        DEBUGINFO("action Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    // 5.1 解析action.Type
    cJSON *type = cJSON_GetObjectItem(action_obj, "Type");
    if (type == NULL || !cJSON_IsNumber(type)) {
        DEBUGINFO("action.Type Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    robot->action.Type = type->valueint;

    // 5.2 解析action.cmds数组
    cJSON *cmds_array = cJSON_GetObjectItem(action_obj, "cmds");
    if (cmds_array == NULL || !cJSON_IsArray(cmds_array)) {
        DEBUGINFO("action.cmds Not find\n");
        cJSON_Delete(root);
        return -1;
    }
    //限制最多八条命令
    robot->action.cmd_count = cJSON_GetArraySize(cmds_array);
    if (robot->action.cmd_count <= 0 || robot->action.cmd_count > 8) {
        DEBUGINFO("cmds Not find\n");
        cJSON_Delete(root);
        return -1;
    }

    // 遍历cmds数组，解析每个命令
    for (int i = 0; i < robot->action.cmd_count; i++) {
        cJSON *cmd_obj = cJSON_GetArrayItem(cmds_array, i);
        if (cmd_obj == NULL || !cJSON_IsObject(cmd_obj)) {
            DEBUGINFO("cmds %d is not object\n", i);
            cJSON_Delete(root);
            return -1;
        }

        // 解析cmd字段
        cJSON *cmd = cJSON_GetObjectItem(cmd_obj, "cmd");
        if (cmd == NULL || !cJSON_IsString(cmd)) {
            DEBUGINFO("cmd [%d] is not string\n", i);
            cJSON_Delete(root);
            return -1;
        }
        strncpy(robot->action.cmds[i].cmd, cmd->valuestring, sizeof(robot->action.cmds[i].cmd)-1);
        robot->action.cmds[i].cmd[sizeof(robot->action.cmds[i].cmd)-1] = '\0';

        // 解析cmdId字段
        cJSON *cmdId = cJSON_GetObjectItem(cmd_obj, "cmdId");
        if (cmdId == NULL || !cJSON_IsString(cmdId)) {
            DEBUGINFO("cmdId[%d] is not string\n", i);
            cJSON_Delete(root);
            return -1;
        }
        strncpy(robot->action.cmds[i].cmdId, cmdId->valuestring, sizeof(robot->action.cmds[i].cmdId)-1);
        robot->action.cmds[i].cmdId[sizeof(robot->action.cmds[i].cmdId)-1] = '\0';

        // 解析params对象（model字段）
        cJSON *params_obj = cJSON_GetObjectItem(cmd_obj, "params");
        if (params_obj == NULL || !cJSON_IsObject(params_obj)) {
            DEBUGINFO("params[%d] is not object\n", i);
            cJSON_Delete(root);
            return -1;
        }
        cJSON *model = cJSON_GetObjectItem(params_obj, "model");
        if (model != NULL && cJSON_IsString(model)) {  // model可能在非runModel命令中不存在
            strncpy(robot->action.cmds[i].params.model, model->valuestring, sizeof(robot->action.cmds[i].params.model)-1);
            robot->action.cmds[i].params.model[sizeof(robot->action.cmds[i].params.model)-1] = '\0';
        } else {
            robot->action.cmds[i].params.model[0] = '\0';  // 空字符串表示无参数
        }
    }
    // 释放cJSON资源
    cJSON_Delete(root);
    //统计解析耗时
    TickType_t end_tick = xTaskGetTickCount();
    TickType_t elapsed_tick = end_tick - start_tick; 
    uint32_t elapsed_ms = pdMS_TO_TICKS(elapsed_tick);    
    DEBUGINFO("elapsed_ms:%ld\n",elapsed_ms);

    // 打印顶层结构体成员
    DEBUGINFO("  headerId: %s\n", robot->headerId);
    DEBUGINFO("  timestamp: %s\n", robot->timestamp);
    DEBUGINFO("  version: %s\n", robot->version);
    
    // 打印RobotAction成员
    DEBUGINFO("  Type: %d\n", robot->action.Type);
    DEBUGINFO("  cmd_count: %d\n", robot->action.cmd_count);
    
    // 打印命令数组（cmds）中的每个命令
    for (int i = 0; i < robot->action.cmd_count; i++) {
        DEBUGINFO("      cmd number %d\n", i + 1);
        DEBUGINFO("      cmd: %s\n", robot->action.cmds[i].cmd);
        DEBUGINFO("      cmdId: %s\n", robot->action.cmds[i].cmdId);
        DEBUGINFO("      params.model: %s\n", robot->action.cmds[i].params.model);
    }

    return 0;
}
//获取robot json转成字符串的接口，返回值需要释放
char* Robot_GetStateJsonStr(void) 
{
    if(RobotJson == NULL)
    {
        DEBUGINFO("robot_GetStateJsonStr fail\n");
        return NULL;
    }    
    char* json_str = cJSON_PrintUnformatted(RobotJson);
    return json_str;
}
//创建心跳包的json
void Robot_CreateHeartBeatJson(void) 
{
    DEBUGINFO("start\n");

    if(Robot_HeartBeatJson != NULL)   
    {
        DEBUGINFO("Robot_HeartBeatJson has been created\n");
        return;
    }
    // 创建根JSON对象
    Robot_HeartBeatJson = cJSON_CreateObject();
    if (Robot_HeartBeatJson == NULL) {
        DEBUGINFO("Robot_HeartBeatJson fail\n");
        return;
    }
    // 向JSON对象添加键值对
    // 添加整数类型：headerId
    cJSON_AddNumberToObject(Robot_HeartBeatJson, "headerId", 125);
    
    // 添加整数类型：timestamp（大整数可正常存储为cJSON的number类型）
    cJSON_AddNumberToObject(Robot_HeartBeatJson, "timestamp", 1);
    
    // 添加字符串类型：version
    cJSON_AddStringToObject(Robot_HeartBeatJson, "version", "1.0.0");
    
    // 添加字符串类型：manufacturer
    cJSON_AddStringToObject(Robot_HeartBeatJson, "manufacturer", "slhc");
    
    // 添加字符串类型：serialNumber
    cJSON_AddStringToObject(Robot_HeartBeatJson, "serialNumber", "bcss.v1.0.0");
    
    // 添加字符串类型：connectionState
    cJSON_AddStringToObject(Robot_HeartBeatJson, "connectionState", "ONLINE");  

    DEBUGINFO("end\n");
}
//获取robot heart beat json转成字符串的接口，返回值需要释放
char* Robot_GetHeartBeatJsonStr(void) 
{
    if(Robot_HeartBeatJson == NULL)
    {
        DEBUGINFO("Robot_HeartBeatJson fail\n");
        return NULL;
    }    
    char* json_str = cJSON_PrintUnformatted(Robot_HeartBeatJson);
    return json_str;    
}
//ROBOT 相关的初始化
void Robot_Init(void)
{
    if(robot_init)return;

    DEBUGINFO("start\n");

    //重新定义cjson中的申请内存和释放内存的函数
    cJSON_Hooks hooks;

    hooks.malloc_fn = pvPortMalloc;

    hooks.free_fn = vPortFree;

    cJSON_InitHooks(&hooks);

    //创建robot json 对象

    Robot_CreateStateJson();

    Robot_CreateHeartBeatJson();

    //初始化状态
    robot_init = true;

    DEBUGINFO("end\n");
}
//通知robot接收任务去发送指定的消息类型或解析来
void Robot_SendMsg(RobotMsgType_t type,char *data)
{
    if(xRobotQueueHandle != NULL)
    {
        RobotMsg_t * robot_msg = pvPortMalloc(sizeof(RobotMsg_t));
        robot_msg->type = type;
        robot_msg->data = data;        
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(xRobotQueueHandle),uxQueueSpacesAvailable(xRobotQueueHandle));
        if (xQueueSend(xRobotQueueHandle, &robot_msg, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("xRobotQueueHandle add success");
        } 
    }     
}
//事件通知更新状态到服务器接口
void Robot_UpdateState(void)
{
    Robot_SendMsg(ROBOT_MSG_SEND,NULL);
}
//将解析后再到实际的控制接口
void Robot_Action2Cmd(void)
{
    for (int i = 0; i < robotAction.action.cmd_count; i++) 
    {
        ServerToCarData.wCtrl = 0x18; // 控制信号 
        ServerToCarData.xStationStatus = 0x01; // 到站状态 
        char *res = strstr(robotAction.action.cmds[i].cmd, "forward");
        if (res != NULL) {
            DEBUGINFO("forward\n"); 
            ServerToCarData.ucDirection = 1; // 小车运行方向 1=正转 2=反转                                                       
        } 
        else
        {
            char *res = strstr(robotAction.action.cmds[i].cmd, "back");
            if (res != NULL) {
                DEBUGINFO("backward\n"); 
                ServerToCarData.ucDirection = 2; // 小车运行方向 1=正转 2=反转 
            }
        }
        vParseCommandToCar(); 
    }   
}





