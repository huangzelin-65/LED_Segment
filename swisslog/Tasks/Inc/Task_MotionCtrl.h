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
void vRemoteModeSet(eAutoModeType xAutoMode);
void vRemoteMotionCmd(eDirectionType xDirection, eSpeedType xSpeed);

#endif /* TASKS_INC_TASK_MOTIONCTRL_H_ */
