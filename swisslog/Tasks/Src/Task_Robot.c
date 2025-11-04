#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Wifi.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "adaptor_mqtt.h"

void vRobotManagerTask(void *argument)
{
  while (1)
  {
	osDelay(pdMS_TO_TICKS(100));
  }
}

void vRobotReceiveTask(void *argument)
{
  while (1)
  {
	osDelay(pdMS_TO_TICKS(100));
  }
}


