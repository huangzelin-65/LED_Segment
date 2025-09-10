#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

extern osThreadId_t InitTaskHandle;
extern osThreadId_t IntProcessTaskHandle;
extern osThreadId_t MotionCtrlTaskHandle;
extern osThreadId_t MotorFeedbackTaskHandle;
extern osThreadId_t RfidTaskHandle;
extern osThreadId_t SensorTaskHandle;
extern osThreadId_t TestTaskHandle;
extern osThreadId_t WifiManagerTaskHandle;
extern osThreadId_t WifiReceiveTaskHandle;
extern osThreadId_t PrintTaskHandle;

void vInitTask(void *argument)
{
  osThreadResume(PrintTaskHandle);
  osThreadResume(IntProcessTaskHandle); 
  osThreadResume(MotionCtrlTaskHandle); 
  osThreadResume(MotorFeedbackTaskHandle); 
  osThreadResume(RfidTaskHandle); 
  osThreadResume(SensorTaskHandle);
  osThreadResume(TestTaskHandle);
  osThreadResume(WifiManagerTaskHandle);
  osThreadResume(WifiReceiveTaskHandle);

  osThreadExit();
}