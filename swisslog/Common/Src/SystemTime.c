#include "stm32h5xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include "SystemTime.h"
#include "LogDebugInfo.h"

// 用于跟踪系统滴答溢出的变量
static uint32_t tick_overflow_count = 0;
static uint32_t last_tick_count = 0;
static SemaphoreHandle_t tick_mutex = NULL;

// 初始化时间跟踪系统
void time_tracker_init(void)
{
    // 创建互斥锁保护共享变量
    tick_mutex = xSemaphoreCreateMutex();
    if(tick_mutex == NULL)
    {
        // 互斥锁创建失败，根据实际情况处理错误
    	DEBUGINFO("tick_mutex create fail\r\n");
    }
    
    // 初始化最后一次的滴答计数
    last_tick_count = xTaskGetTickCount();
}

// 检查并处理滴答计数溢出
static void check_for_tick_overflow(void)
{
    if(tick_mutex != NULL && xSemaphoreTake(tick_mutex, portMAX_DELAY) == pdTRUE)
    {
        uint32_t current_tick = xTaskGetTickCount();
        
        // 检测溢出：当前值小于上次值表示发生了溢出
        if(current_tick < last_tick_count)
        {
            tick_overflow_count++;
        }
        
        last_tick_count = current_tick;
        xSemaphoreGive(tick_mutex);
    }
}

// 获取系统上电以来的毫秒数，带溢出处理
uint64_t get_system_uptime_ms(void)
{
    uint64_t total_ms = 0;
    uint32_t current_tick = 0;
    uint32_t overflow_count = 0;
    
    check_for_tick_overflow();
    
    if(tick_mutex != NULL && xSemaphoreTake(tick_mutex, portMAX_DELAY) == pdTRUE)
    {
        current_tick = last_tick_count;
        overflow_count = tick_overflow_count;
        xSemaphoreGive(tick_mutex);
    }
    
    // 计算总毫秒数：溢出次数 * 最大值 + 当前计数值
    total_ms = (uint64_t)overflow_count * (uint64_t)UINT32_MAX + 
               (uint64_t)current_tick;
    // 转换为毫秒（假设configTICK_RATE_HZ为1000）
    total_ms *= (1000 / configTICK_RATE_HZ);
    
    return total_ms;
}

// 将毫秒数转换为时分秒
void convert_ms_to_hms(uint64_t ms, uint16_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    uint64_t total_seconds = ms / 1000;
    
    // 计算小时数，限制在0-9999范围内
    *hours = (uint16_t)(total_seconds / 3600);
    if(*hours > 9999)
    {
        *hours = 9999;
    }
    
    total_seconds %= 3600;
    
    // 计算分钟数，限制在0-59范围内
    *minutes = (uint8_t)(total_seconds / 60);
    if(*minutes > 59)
    {
        *minutes = 59;
    }
    
    // 计算秒数，限制在0-59范围内
    *seconds = (uint8_t)(total_seconds % 60);
}

// 将时间转换为指定格式的ASCII码
// hours_ascii: 4字节，分钟和秒各2字节
void convert_time_to_ascii(uint16_t hours, uint8_t minutes, uint8_t seconds, 
                          char *hours_ascii, char *minutes_ascii, char *seconds_ascii)
{
    // 确保数组有足够空间（包括终止符）
    memset(hours_ascii, 0, 5);
    memset(minutes_ascii, 0, 3);
    memset(seconds_ascii, 0, 3);
    
    // 转换小时为4字节ASCII (0000-9999)
    sprintf(hours_ascii, "%04d", hours);
    
    // 转换分钟为2字节ASCII (00-59)
    sprintf(minutes_ascii, "%02d", minutes);
    
    // 转换秒为2字节ASCII (00-59)
    sprintf(seconds_ascii, "%02d", seconds);
}

// 获取当前时间
void System_Get_RunTime(uint16_t hours, uint8_t minutes, uint8_t seconds)
{
    uint64_t uptime_ms = get_system_uptime_ms();
    convert_ms_to_hms(uptime_ms, &hours, &minutes, &seconds);
		DEBUGINFO("hours:%d,minutes:%d,seconds:%d\n",hours,minutes,seconds);
}

// 获取当前时间并转换为ASCII
void System_Get_RunTime_Ascii(uint8_t *hours_ascii, uint8_t *minutes_ascii, uint8_t *seconds_ascii)
{
    uint64_t uptime_ms = get_system_uptime_ms();
    uint16_t hours;
    uint8_t minutes, seconds;
    
    convert_ms_to_hms(uptime_ms, &hours, &minutes, &seconds);
		DEBUGINFO("hours:%d,minutes:%d,seconds:%d\n",hours,minutes,seconds);
    convert_time_to_ascii(hours, minutes, seconds, (char *)hours_ascii, (char *)minutes_ascii, (char *)seconds_ascii);
		DEBUGINFO("hours_ascii:%s,minutes_ascii:%s,seconds_ascii:%s\n",hours_ascii,minutes_ascii,seconds_ascii);
}

