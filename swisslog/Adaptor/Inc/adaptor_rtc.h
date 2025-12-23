#ifndef ADAPTOR_INC_ADAPTOR_RTC_H_
#define ADAPTOR_INC_ADAPTOR_RTC_H_
#include <stdbool.h>

void Rtc_Init(void);
void Rtc_PowerEnable(void);
int Rtc_GetTime(RTC_TimeTypeDef *sTime);
int Rtc_GetDate(RTC_DateTypeDef *sDate);
int Rtc_SetDate(uint8_t Year,uint8_t Month,uint8_t Date,uint8_t WeekDay);
int Rtc_SetTime(uint8_t Hours,uint8_t Minutes,uint8_t Seconds);
char* Rtc_GetTimeStr(void);
uint32_t Rtc_GetTimeStamp(void);
void Rtc_SetDateTimeStamp(uint32_t time_stamp);
#endif 
