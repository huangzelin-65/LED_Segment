#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Tcp.h"
#include "LogDebugInfo.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "lwip.h"
/* TCP管理任务入口函数 */
void vTcpManagerTask(void *argument)
{
  DEBUGINFO("vTcpManagerTask\r\n");
  MX_LWIP_Init();
  while (1)
  {
	osDelay(pdMS_TO_TICKS(100));
  }
}
    

/* wifi接收任务入口函数 */
void vTcpReceiveTask(void *argument)
{
  DEBUGINFO("vTcpReceiveTask\r\n");
  while (1)
  {
    osDelay(pdMS_TO_TICKS(100));
  } 
}


