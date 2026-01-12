#ifndef PERIPHERALS_INC_UV_CLEAN_H_
#define PERIPHERALS_INC_UV_CLEAN_H_

uint8_t u8_UvClean_IsRunning(void);
void v_UvClean_Start(uint8_t u8_timeMin,uint8_t startTime[6]);
void v_UvClean_Stop(void);
void v_UvClean_Save_Record(void);
void v_UvClean_Get_Record(uint8_t time[7]);
bool b_UvClean_Set_Time(uint8_t time);
bool b_UvClean_Set_Duration(uint8_t u8_Duration); // 设置消毒时长(更新eeprom里的消毒时长设置 和 CarStatus里的消毒时长设置)
uint8_t u8_UvClean_Get_Duration(void); // 获取默认消毒时长

#endif /* PERIPHERALS_INC_UV_CLEAN_H_ */
