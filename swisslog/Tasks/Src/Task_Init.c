#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "Project_Config.h"
#include "app_freertos.h"
#include "adaptor_eeprom.h"
#include "adaptor_power.h"
#include "SystemTime.h"
#include "Task_BoxCtrl.h"
#include "Encoder.h"


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
extern osThreadId_t RobotManagerTaskHandle;
extern osThreadId_t RobotReceiveTaskHandle;
extern osThreadId_t TcpManagerTaskHandle;
extern osThreadId_t TcpReceiveTaskHandle;
extern osThreadId_t RobotHeartBeatTaskHandle;
extern osThreadId_t MqttNotifyTaskHandle;
extern CarStatus_t CarStatus;

void vGetChipID(void)
{
 uint32_t uid0, uid1, uid2;
    
  /* 1. 禁用 ICACHE */
  HAL_ICACHE_Disable();

  // 读取芯片UID
  uid0 = HAL_GetUIDw0();
  uid1 = HAL_GetUIDw1();
  uid2 = HAL_GetUIDw2();

  /* 2. 重新启用 ICACHE */
  HAL_ICACHE_Enable();

  // 处理或输出UID（例如通过串口打印）
  DEBUGINFO("STM32H563 Chip ID: UID0: 0x%08lX, UID1: 0x%08lX, UID2: 0x%08lX\r\n", uid0, uid1, uid2);
}


void vInitTask(void *argument)
{
  DEBUGINFO("InitTask\r\n");

  vPower_Init();
  v_Eeprom_Data_Init();
  // vEepromTest();
  vTime_Tracker_Init();
  vGetChipID();

  CarStatus.usCarID = usEncoder_Read_Number(); // 读取编码器编号
  DEBUGINFO("CarID: %d", CarStatus.usCarID);

  // Car_Set_Station_Status(InStation);// 设置小车状态为InStation
  CarStatus.xSetDirection = Forward; //小车预设运行方向为前进
  CarStatus.xMotorStopReason = NoStopReason; //小车停止原因为无

  osDelay(pdMS_TO_TICKS(100));

  osThreadResume(PrintTaskHandle);
  osThreadResume(MotionCtrlTaskHandle); 
  osThreadResume(MotorFeedbackTaskHandle); 
  osThreadResume(CarRfidTaskHandle); 
  osThreadResume(SensorTaskHandle);
  
#ifdef UART1_TEST
  osThreadResume(TestTaskHandle);
#endif

#ifdef  BOX_CTRL
  osThreadResume(BoxCtrlTaskHandle);
  osThreadResume(BoxRfidTaskHandle);
  osThreadResume(BoxRfidEventTaskHandle);
  osThreadResume(HmiEventTaskHandle);
  osThreadResume(HmiSendTaskHandle);
  osThreadResume(HmiRecvTaskHandle);
  osThreadResume(HmiWaitTaskHandle);
  osThreadResume(BoxLEDTaskHandle);
#endif

#ifdef  USE_WIFI
  osThreadResume(WifiManagerTaskHandle);
  osThreadResume(WifiReceiveTaskHandle);
#endif

#ifdef MQTT_ENABLE
  osThreadResume(MqttManagerTaskHandle);
  osThreadResume(MqttReceiveTaskHandle); 
  osThreadResume(RobotManagerTaskHandle);
  osThreadResume(RobotReceiveTaskHandle); 
  osThreadResume(TcpManagerTaskHandle);
  osThreadResume(TcpReceiveTaskHandle);
  osThreadResume(RobotHeartBeatTaskHandle);     
  osThreadResume(MqttNotifyTaskHandle);
  osThreadResume(NtpManagerTaskHandle);
  osThreadResume(JsonGenerateTaskHandle);
  osThreadResume(JsonParseTaskHandle);  
  osThreadResume(MqttErrorHandleTaskHandle);
  osThreadResume(MqttHeartBeatTaskHandle);   
#endif

#ifdef  USE_CAN
  osThreadResume(CANManagerTaskHandle);
#endif

  osThreadResume(IntProcessTaskHandle); 

  osThreadExit();
}