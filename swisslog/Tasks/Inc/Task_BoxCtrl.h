#ifndef TASKS_INC_TASK_BOXCTRL_H_
#define TASKS_INC_TASK_BOXCTRL_H_

typedef enum {
   	Lock=0,
   	Open,
   	Idle
}ELockStatus;

ELockStatus Car_Get_Elock_Status(void);
CarStationStatus Car_Get_Station_Status(void);
void Car_Set_Station_Status(CarStationStatus value);
uint8_t Button_Gpio_Press_Status(void);
void Button_Gpio_Press_Set(uint8_t Val);
void vBoxCtrlTask(void *argument);

#endif /* TASKS_INC_TASK_BOXCTRL_H_ */
