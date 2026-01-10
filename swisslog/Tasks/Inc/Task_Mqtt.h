#ifndef TASKS_INC_TASK_MQTT_H_
#define TASKS_INC_TASK_MQTT_H_



void vMqttManagerTask(void *argument);
void vMqttReceiveTask(void *argument);
void vMqttNotifyTask(void *argument);
void vMqttErrorHandleTask(void *argument);
#endif