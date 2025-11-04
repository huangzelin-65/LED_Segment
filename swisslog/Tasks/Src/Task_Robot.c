#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Robot.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "adaptor_mqtt.h"

RobotAction_t robotAction = {
    // 顶层字段初始化
    .header_id = 331,                  // 消息头部ID，递增序列
    .timestamp = 1695000000,           // 时间戳（示例：2023-09-18 12:00:00）
    .version = 2,                      // 协议版本号
    .manufacturer = "RoboTech Inc",    // 制造商名称
    .serial_number = "RT-2023-0045",   // 设备序列号
    
    // 动作字段初始化
    .actions = {
        .action_type = 1,              // 动作类型：1表示移动控制
        .action_description = "Manual movement control",  // 动作描述
        .parameters = {
            .operating_mode = "MANUAL",       // 操作模式：手动模式
            .speed_level = 3,                 // 速度等级：3级（1-5级）
            .direction = DIRECTION_FORWARD,   // 方向：前进（对应枚举值）
            .disinfect_state = {
                .runState = false,            // 消毒状态：未运行
                .runTime = 0,                 // 已运行时间：0秒
                .startTime = 0,               // 开始时间戳：未启动
                .setTime = 120                // 设定消毒时长：120秒
            }
        }
    }
};

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

//处理robot相关任务，心跳包等
void vRobotManagerTask(void *argument)
{
    int heartbeat_cnt = 0;
    Robot_Init();
    while (1)
    {
        if(mqtt_isConnected)
        {
            heartbeat_cnt++;
            if(heartbeat_cnt >= 50)//5秒发送一次心跳包
            {
                heartbeat_cnt = 0;
                DEBUGINFO("MQTT_MSG_HEARTBEAT\n");  
                Mqtt_SendMsg(MQTT_MSG_HEARTBEAT,NULL);
            }
        }
        osDelay(pdMS_TO_TICKS(100));
    }
}
//处理事件发生时，更新robot相关的结构体和对应的json
void vRobotReceiveTask(void *argument)
{
  while (1)
  {
	osDelay(pdMS_TO_TICKS(100));
  }
}

// 辅助函数：将Direction枚举转换为字符串
const char* Robot_DirectionToString(Direction dir) {
    switch (dir) {
        case DIRECTION_FORWARD: return "DIRECTION_FORWARD";
        case DIRECTION_BACKWARD: return "DIRECTION_BACKWARD";
        default: return "UNKNOWN_DIRECTION";
    }
}
//创建vda5050必要字段，赋值为NULL
void Robot_AddNullFields(cJSON* root) {
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
cJSON* Robot_CreateBumperState() {
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "front", robotSate.bumperState.front);
    cJSON_AddBoolToObject(obj, "back", robotSate.bumperState.back);
    return obj;
}
//小车速度的状态
cJSON* Robot_CreateMoveState() {
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "speedLevel", robotSate.moveState.speedLevel);
    cJSON_AddNumberToObject(obj, "direction", robotSate.moveState.direction);
    return obj;
}
//车厢消毒净化状态
cJSON* Robot_CreateDisinfectState() {
    cJSON* obj = cJSON_CreateObject();
    cJSON_AddBoolToObject(obj, "runState", robotSate.disinfectState.runState);
    cJSON_AddNumberToObject(obj, "runTime", robotSate.disinfectState.runTime);
    cJSON_AddNumberToObject(obj, "startTime", robotSate.disinfectState.startTime);
    cJSON_AddNumberToObject(obj, "setTime", robotSate.disinfectState.setTime);
    return obj;
}
//小车报错的消息
cJSON* Robot_CreateErrors() {
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

    //初始化状态
    robot_init = true;

    DEBUGINFO("end\n");
}



