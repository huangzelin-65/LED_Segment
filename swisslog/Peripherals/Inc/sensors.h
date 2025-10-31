/*
 * sensors.h
 *
 *  Created on: Jun 20, 2025
 *      Author: e3lijia25d
 */

#ifndef PERIPHERALS_INC_SENSORS_H_
#define PERIPHERALS_INC_SENSORS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "queue.h"
#include <stdio.h>


void vSensorStatusCheck(void);
void vToggleSwitchStatusCheck(void);
void vResetLedStatusCheck(void);
void vServiceJoystickStatusCheck(void);

void vSensorDebounceCallback(void *argument);
void vToggleSwitchCallback(void *argument);
void vResetButtonCallback(void *argument);


#endif /* PERIPHERALS_INC_SENSORS_H_ */
