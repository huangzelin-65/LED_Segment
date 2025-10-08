#ifndef PERIPHERALS_INC_UV_CLEAN_H_
#define PERIPHERALS_INC_UV_CLEAN_H_

uint8_t UvClean_IsRunning(void);
void UvClean_Start(uint8_t timeMin,uint8_t startTime[6]);
void UvClean_Stop(void);
void UvClean_Save_Record(void);
void UvClean_Get_Record(uint8_t time[7]);

#endif /* PERIPHERALS_INC_UV_CLEAN_H_ */
