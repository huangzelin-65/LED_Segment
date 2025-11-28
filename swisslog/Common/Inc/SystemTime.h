#ifndef COMMON_INC_SYSTEMTIME_H_
#define COMMON_INC_SYSTEMTIME_H_

void vTime_Tracker_Init(void);
void vSystem_Get_RunTime(uint16_t hours, uint8_t minutes, uint8_t seconds);
void vSystem_Get_RunTime_Ascii(uint8_t *hours_ascii, uint8_t *minutes_ascii, uint8_t *seconds_ascii);
#endif /* COMMON_INC_SYSTEMTIME_H_ */
