#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "adaptor_eeprom.h"
#include "SystemTime.h"
#include "Task_BoxCtrl.h"


extern osThreadId_t InitTaskHandle;
extern osThreadId_t IntProcessTaskHandle;
extern osThreadId_t MotionCtrlTaskHandle;
extern osThreadId_t MotorFeedbackTaskHandle;
extern osThreadId_t CarRfidTaskHandle;
extern osThreadId_t SensorTaskHandle;
extern osThreadId_t TestTaskHandle;
extern osThreadId_t WifiManagerTaskHandle;
extern osThreadId_t WifiReceiveTaskHandle;
extern osThreadId_t PrintTaskHandle;
extern osThreadId_t BoxCtrlTaskHandle;
extern osThreadId_t BoxRfidTaskHandle;
extern osThreadId_t BoxRfidEventTaskHandle;
extern osThreadId_t HmiEventTaskHandle;
extern osThreadId_t HmiSendTaskHandle;
extern osThreadId_t HmiRecvTaskHandle;
extern osThreadId_t HmiWaitTaskHandle;
extern osThreadId_t BoxLEDTaskHandle;
extern osThreadId_t MqttManagerTaskHandle;
extern osThreadId_t MqttReceiveTaskHandle;

extern CarStatus_t CarStatus;


void vInitTask(void *argument)
{
  CarStatus.xSetDirection = Forward; //小车预设运行方向为前进

  osThreadResume(PrintTaskHandle);
  osThreadResume(IntProcessTaskHandle); 
  osThreadResume(MotionCtrlTaskHandle); 
  osThreadResume(MotorFeedbackTaskHandle); 
  osThreadResume(CarRfidTaskHandle); 
  osThreadResume(SensorTaskHandle);
  osThreadResume(TestTaskHandle);
  // osThreadResume(WifiManagerTaskHandle);
  // osThreadResume(WifiReceiveTaskHandle);
  osThreadResume(BoxCtrlTaskHandle);
  osThreadResume(BoxRfidTaskHandle);
  osThreadResume(BoxRfidEventTaskHandle);
  osThreadResume(HmiEventTaskHandle);
  osThreadResume(HmiSendTaskHandle);
  osThreadResume(HmiRecvTaskHandle);
  osThreadResume(HmiWaitTaskHandle);
  osThreadResume(BoxLEDTaskHandle);
  // osThreadResume(MqttManagerTaskHandle);
  // osThreadResume(MqttReceiveTaskHandle); 

  DEBUGINFO("InitTask\r\n");

  vEeprom_Data_Init();
  time_tracker_init();

  //Car_Set_Station_Status(InStation);// 设置小车状态为InStation
  
  //vEepromTest();

  // uint32_t uid0, uid1, uid2;
    
  // // 读取芯片UID
  // uid0 = HAL_GetUIDw0();
  // uid1 = HAL_GetUIDw1();
  // uid2 = HAL_GetUIDw2();

  // // 处理或输出UID（例如通过串口打印）
  // DEBUGINFO("STM32H563 Chip ID:\r\n");
  // DEBUGINFO("UID0: 0x%08X\r\n", uid0);
  // DEBUGINFO("UID1: 0x%08X\r\n", uid1);
  // DEBUGINFO("UID2: 0x%08X\r\n", uid2);

  osThreadExit();
}