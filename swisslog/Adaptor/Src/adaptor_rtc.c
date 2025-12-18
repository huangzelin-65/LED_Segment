#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_rtc.h"
#include "LogDebugInfo.h"
#include "queue.h"
#include <time.h>

#define WAKEUP_TIMER_ENABLE 0x32F2

extern RTC_HandleTypeDef hrtc;
//初始化rtc功能，使用内部时钟
void Rtc_Init(void)
{
  /*##-1- Check if data stored in BackUp register1: Wakeup timer enable #######*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == WAKEUP_TIMER_ENABLE)
  {
    /* if the wakeup timer is enabled then deactivate it to disable the wakeup timer interrupt */
    if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK)
    {
      /* Initialization Error */
    //   DEBUGINFO("Initialization Error");
      return;
    }
  }

  /*##-2- Configure the RTC Wakeup peripheral #################################*/
  /* Setting the Wakeup time to 1 s
       If RTC_WAKEUPCLOCK_CK_SPRE_16BITS is selected, the frequency is 1Hz,
       this allows to get a wakeup time equal to 1 s if the counter is 0x0 */
  HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS,0);

  /*##-3- Write 'wakeup timer enabled' tag in RTC Backup data Register 1 #######*/
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, WAKEUP_TIMER_ENABLE);      
}
//启动备份区，时钟启动前调用
void Rtc_PowerEnable(void)
{
  /* Reset the RTC peripheral and the RTC clock source selection */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_BACKUPRESET_FORCE();
  __HAL_RCC_BACKUPRESET_RELEASE();    
}
//获取rtc时间
int Rtc_GetTime(RTC_TimeTypeDef *sTime)
{
    HAL_StatusTypeDef status;
    // read rtc time
    status = HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BIN);
    if (status != HAL_OK) {
        DEBUGINFO("get time fail, status is %d\r\n", status);
        return -1;
    }
    DEBUGINFO("Hours:%d Minutes:%d Seconds:%d\r\n",sTime->Hours, sTime->Minutes, sTime->Seconds); 
    return 0;       
}
//获取日期
int Rtc_GetDate(RTC_DateTypeDef *sDate)
{
    HAL_StatusTypeDef status;
    // read rtc date
    status = HAL_RTC_GetDate(&hrtc, sDate, RTC_FORMAT_BIN);
    if (status != HAL_OK) {
        DEBUGINFO("get date fail, status is %d\r\n", status);
        return -1;
    }
    DEBUGINFO("%d:%d:%d:%d\r\n", sDate->Year, sDate->Month, sDate->Date, sDate->WeekDay);
    return 0;     
}
//设置日期
int Rtc_SetDate(uint8_t Year,uint8_t Month,uint8_t Date,uint8_t WeekDay)
{
    HAL_StatusTypeDef status;
    RTC_DateTypeDef sDate = {0}; 
    sDate.Year = Year;
    sDate.Date = Date;
    sDate.Month = Month;
    sDate.WeekDay = WeekDay;
    status = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);  
    if (status != HAL_OK) {
        DEBUGINFO("set date fail, status is %d\r\n", status);
        return -1;
    } 
    DEBUGINFO("Year:%d Month:%d Date:%d WeekDay:%d\r\n", Year, Month, Date, WeekDay);
    return 0;          
}

//设置时间
int Rtc_SetTime(uint8_t Hours,uint8_t Minutes,uint8_t Seconds)
{
    HAL_StatusTypeDef status;
    RTC_TimeTypeDef sTime = {0};
    sTime.Hours = Hours;
    sTime.Minutes = Minutes;
    sTime.Seconds = Seconds;
    status = HAL_RTC_SetTime(&hrtc,&sTime,RTC_FORMAT_BIN); 
    if (status != HAL_OK) {
        DEBUGINFO("set time fail, status is %d\r\n", status);
        return -1;
    } 
    DEBUGINFO("Hours:%d Minutes:%d Seconds:%d\r\n",sTime.Hours, sTime.Minutes, sTime.Seconds);  
    return 0;           
}

static char rtc_time_buf[32] = {0};

/**
 * @brief  获取格式化的RTC实时时间字符串
 * @retval 时间字符串指针（格式：2024-10-22 10:30:00）
 */
char* Rtc_GetTimeStr(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  // ---------- 关键：先读时间，再读日期 ----------
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  // 格式化时间字符串（snprintf避免缓冲区溢出）
  snprintf(rtc_time_buf, sizeof(rtc_time_buf),
           "20%02d-%02d-%02d %02d:%02d:%02d",
           sDate.Year,    // 年（00-99，对应2000-2099）
           sDate.Month,   // 月（1-12）
           sDate.Date,    // 日（1-31）
           sTime.Hours,   // 时（0-23）
           sTime.Minutes, // 分（0-59）
           sTime.Seconds  // 秒（0-59）
          );

  return rtc_time_buf;
}

uint32_t Rtc_GetTimeStamp(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  // ---------- 关键：先读时间，再读日期 ----------
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); 
  
  // 1. 定义并填充 struct tm（本地时间：2025年12月16日 10:30:00）
  struct tm current_time_val = {0};
  current_time_val.tm_sec  = sTime.Seconds;    // 秒
  current_time_val.tm_min  = sTime.Minutes;   // 分
  current_time_val.tm_hour = sTime.Hours;   // 时
  current_time_val.tm_mday = sDate.Date;   // 日
  current_time_val.tm_mon  = sDate.Month;   // 月（11=12月）
  current_time_val.tm_year = sDate.Year;  // 年（2025-1900=125）
  // tm_wday/tm_yday 无需设置，会被自动计算

  // 2. 转换为时间戳（本地时区）
  time_t timestamp = mktime(&current_time_val);
  if (timestamp == (time_t)-1) {
      DEBUGINFO("fail\n");
      return 0;
  }

  // 3. 输出结果
  DEBUGINFO("time:%lld\n", (uint32_t)timestamp);
  return (uint32_t)timestamp;  
}
