#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "adaptor_eeprom.h"
#include "SystemTime.h"

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
  //osThreadResume(WifiManagerTaskHandle);
  //osThreadResume(WifiReceiveTaskHandle);
  osThreadResume(BoxCtrlTaskHandle);
  osThreadResume(BoxRfidTaskHandle);
  osThreadResume(BoxRfidEventTaskHandle);
  osThreadResume(HmiEventTaskHandle);
  osThreadResume(HmiSendTaskHandle);
  osThreadResume(HmiRecvTaskHandle);
  osThreadResume(HmiWaitTaskHandle);
  osThreadResume(BoxLEDTaskHandle);


  DEBUGINFO("InitTask\r\n");

  vEeprom_Data_Init();
  time_tracker_init();
  //vEepromTest();

  osThreadExit();
}