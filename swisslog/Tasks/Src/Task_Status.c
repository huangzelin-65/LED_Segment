#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Status.h"
#include "LogDebugInfo.h"
#include <stdbool.h>
#include "queue.h"

char buffer[512];

void vStatus_Task(void *argument)
{
  DEBUGINFO("vStatus_Task\r\n");   
  while (1)
  {
	// R = 运行中（Running）、B = 阻塞（Blocked）、S = 挂起（Suspended）、D = 删除（Deleted）    
    vTaskList(buffer);
    DEBUGINFO("task state priority reststack index\n%s", buffer);

    // 获取当前剩余空闲内存（字节）
    size_t xFreeHeapSize = xPortGetFreeHeapSize();
    // 获取历史最小空闲内存（字节）
    size_t xMinFreeHeapSize = xPortGetMinimumEverFreeHeapSize();

    // 打印（可转换为KB更易读，1KB=1024字节）
    DEBUGINFO("rest: %u byte (%u KB)\r\n", 
           (unsigned int)xFreeHeapSize, 
           (unsigned int)(xFreeHeapSize / 1024));
    DEBUGINFO("min: %u byte (%u KB)\r\n", 
           (unsigned int)xMinFreeHeapSize, 
           (unsigned int)(xMinFreeHeapSize / 1024));    
    osDelay(60000);
  }
}




