#ifndef TASKS_INC_TASK_BOXCTRL_H_
#define TASKS_INC_TASK_BOXCTRL_H_


CarStationStatus Car_Get_Station_Status(void);
void Car_Set_Station_Status(CarStationStatus value);
void vBoxCtrlTask(void *argument);

#endif /* TASKS_INC_TASK_BOXCTRL_H_ */
