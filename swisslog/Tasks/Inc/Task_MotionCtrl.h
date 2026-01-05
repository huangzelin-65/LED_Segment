/*
 * Task_MotorCtrl.h
 *
 *  Created on: Aug 27, 2025
 *      Author: e3lijia25d
 */

#ifndef TASKS_INC_TASK_MOTIONCTRL_H_
#define TASKS_INC_TASK_MOTIONCTRL_H_

#include "main.h"

void vCarRunStatusInit();
void vRemoteModeSet(eAutoModeType xAutoMode); // 设置小车“远程手动模式”/“远程自动模式”
void vRemoteMotionCmd(eDirectionType xDirection, eSpeedType xSpeed); // 远程设置小车“运行方向”和“运行速度”

#endif /* TASKS_INC_TASK_MOTIONCTRL_H_ */
