/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adaptor_wifi.h"
#include "adaptor_test.h"
#include "adaptor_motor.h"
#include "adaptor_rfid.h"
#include "queue.h"
#include "sensors.h"
#include "LogDebugInfo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Definitions for xCarRfidRxSem */
osSemaphoreId_t xCarRfidRxSemHandle;
const osSemaphoreAttr_t xCarRfidRxSem_attributes = {
  .name = "xCarRfidRxSem"
};

/* Definitions for xBoxRfidRxSem */
osSemaphoreId_t xBoxRfidRxSemHandle;
const osSemaphoreAttr_t xBoxRfidRxSem_attributes = {
  .name = "xBoxRfidRxSem"
};


/* Definitions for xTestRxSem */
osSemaphoreId_t xTestRxSemHandle;
const osSemaphoreAttr_t xTestRxSem_attributes = {
  .name = "xTestRxSem"
};


/* Definitions for xWifiReadySem */
osSemaphoreId_t xWifiReadySemHandle;
const osSemaphoreAttr_t xWifiReadySem_attributes = {
  .name = "xWifiReadySem"
};

/* Definitions for xWifiRxSem */
osSemaphoreId_t xWifiRxSemHandle;
const osSemaphoreAttr_t xWifiRxSem_attributes = {
  .name = "xWifiRxSem"
};

/* Definitions for xMotorRxSem */
osSemaphoreId_t xMotorRxSemHandle;
const osSemaphoreAttr_t xMotorRxSem_attributes = {
  .name = "xMotorRxSem"
};

/* Definitions for xHmiRxSem */
osSemaphoreId_t xHmiRxSemHandle;
const osSemaphoreAttr_t xHmiRxSem_attributes = {
  .name = "xHmiRxSem"
};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for InitTask */
osThreadId_t InitTaskHandle;
const osThreadAttr_t InitTask_attributes = {
  .name = "InitTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 256 * 4
};
/* Definitions for IntProcessTask */
osThreadId_t IntProcessTaskHandle;
const osThreadAttr_t IntProcessTask_attributes = {
  .name = "IntProcessTask",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 256 * 4
};
/* Definitions for MotionCtrlTask */
osThreadId_t MotionCtrlTaskHandle;
const osThreadAttr_t MotionCtrlTask_attributes = {
  .name = "MotionCtrlTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for MotorFeedbackTask */
osThreadId_t MotorFeedbackTaskHandle;
const osThreadAttr_t MotorFeedbackTask_attributes = {
  .name = "MotorFeedbackTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for CarRfidTask */
osThreadId_t CarRfidTaskHandle;
const osThreadAttr_t CarRfidTask_attributes = {
  .name = "CarRfidTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for SensorTask */
osThreadId_t SensorTaskHandle;
const osThreadAttr_t SensorTask_attributes = {
  .name = "SensorTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for TestTask */
osThreadId_t TestTaskHandle;
const osThreadAttr_t TestTask_attributes = {
  .name = "TestTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for WifiManagerTask */
osThreadId_t WifiManagerTaskHandle;
const osThreadAttr_t WifiManagerTask_attributes = {
  .name = "WifiManagerTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for WifiReceiveTask */
osThreadId_t WifiReceiveTaskHandle;
const osThreadAttr_t WifiReceiveTask_attributes = {
  .name = "WifiReceiveTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024 * 4
};
/* Definitions for PrintTask */
osThreadId_t PrintTaskHandle;
const osThreadAttr_t PrintTask_attributes = {
  .name = "PrintTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 256 * 4
};
/* Definitions for BoxCtrlTask */
osThreadId_t BoxCtrlTaskHandle;
const osThreadAttr_t BoxCtrlTask_attributes = {
  .name = "BoxCtrlTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for BoxRfidTask */
osThreadId_t BoxRfidTaskHandle;
const osThreadAttr_t BoxRfidTask_attributes = {
  .name = "BoxRfidTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for BoxRfidEventTask */
osThreadId_t BoxRfidEventTaskHandle;
const osThreadAttr_t BoxRfidEventTask_attributes = {
  .name = "BoxRfidEventTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for HmiEventTask */
osThreadId_t HmiEventTaskHandle;
const osThreadAttr_t HmiEventTask_attributes = {
  .name = "HmiEventTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for HmiSendTask */
osThreadId_t HmiSendTaskHandle;
const osThreadAttr_t HmiSendTask_attributes = {
  .name = "HmiSendTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for HmiRecvTask */
osThreadId_t HmiRecvTaskHandle;
const osThreadAttr_t HmiRecvTask_attributes = {
  .name = "HmiRecvTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for HmiWaitTask */
osThreadId_t HmiWaitTaskHandle;
const osThreadAttr_t HmiWaitTask_attributes = {
  .name = "HmiWaitTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for BoxLEDTask */
osThreadId_t BoxLEDTaskHandle;
const osThreadAttr_t BoxLEDTask_attributes = {
  .name = "BoxLEDTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 128 * 4
};
/* Definitions for MqttManagerTask */
osThreadId_t MqttManagerTaskHandle;
const osThreadAttr_t MqttManagerTask_attributes = {
  .name = "MqttManagerTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for MqttReceiveTask */
osThreadId_t MqttReceiveTaskHandle;
const osThreadAttr_t MqttReceiveTask_attributes = {
  .name = "MqttReceiveTask",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for RobotManagerTask */
osThreadId_t RobotManagerTaskHandle;
const osThreadAttr_t RobotManagerTask_attributes = {
  .name = "RobotManagerTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for RobotReceiveTask */
osThreadId_t RobotReceiveTaskHandle;
const osThreadAttr_t RobotReceiveTask_attributes = {
  .name = "RobotReceiveTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for TcpManagerTask */
osThreadId_t TcpManagerTaskHandle;
const osThreadAttr_t TcpManagerTask_attributes = {
  .name = "TcpManagerTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 2048 * 4
};
/* Definitions for TcpReceiveTask */
osThreadId_t TcpReceiveTaskHandle;
const osThreadAttr_t TcpReceiveTask_attributes = {
  .name = "TcpReceiveTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for RobotHeartBeatTask */
osThreadId_t RobotHeartBeatTaskHandle;
const osThreadAttr_t RobotHeartBeatTask_attributes = {
  .name = "RobotHeartBeatTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for MqttNotifyTask */
osThreadId_t MqttNotifyTaskHandle;
const osThreadAttr_t MqttNotifyTask_attributes = {
  .name = "MqttNotifyTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for wifiUsartMutex */
osMutexId_t wifiUsartMutexHandle;
const osMutexAttr_t wifiUsartMutex_attributes = {
  .name = "wifiUsartMutex"
};
/* Definitions for mqttMutex */
osMutexId_t mqttMutexHandle;
const osMutexAttr_t mqttMutex_attributes = {
  .name = "mqttMutex"
};
/* Definitions for xResetButtonTimer */
osTimerId_t xResetButtonTimerHandle;
const osTimerAttr_t xResetButtonTimer_attributes = {
  .name = "xResetButtonTimer"
};
/* Definitions for xSensorDebounceTimer */
osTimerId_t xSensorDebounceTimerHandle;
const osTimerAttr_t xSensorDebounceTimer_attributes = {
  .name = "xSensorDebounceTimer"
};
/* Definitions for xToggleSwitchTimer */
osTimerId_t xToggleSwitchTimerHandle;
const osTimerAttr_t xToggleSwitchTimer_attributes = {
  .name = "xToggleSwitchTimer"
};
/* Definitions for xBoxRfidLoginTimer */
osTimerId_t xBoxRfidLoginTimerHandle;
const osTimerAttr_t xBoxRfidLoginTimer_attributes = {
  .name = "xBoxRfidLoginTimer"
};
/* Definitions for xUVTimer */
osTimerId_t xUVTimerHandle;
const osTimerAttr_t xUVTimer_attributes = {
  .name = "xUVTimer"
};
/* Definitions for xBoxELockDebounceTimer */
osTimerId_t xBoxELockDebounceTimerHandle;
const osTimerAttr_t xBoxELockDebounceTimer_attributes = {
  .name = "xBoxELockDebounceTimer"
};
/* Definitions for xServiceJoystickDebounceTimer */
osTimerId_t xServiceJoystickDebounceTimerHandle;
const osTimerAttr_t xServiceJoystickDebounceTimer_attributes = {
  .name = "xServiceJoystickDebounceTimer"
};
/* Definitions for xInterrupt_Queue */
osMessageQueueId_t xInterrupt_QueueHandle;
const osMessageQueueAttr_t xInterrupt_Queue_attributes = {
  .name = "xInterrupt_Queue"
};
/* Definitions for xSensor_Queue */
osMessageQueueId_t xSensor_QueueHandle;
const osMessageQueueAttr_t xSensor_Queue_attributes = {
  .name = "xSensor_Queue"
};
/* Definitions for xWifi_Rx_Queue */
osMessageQueueId_t xWifi_Rx_QueueHandle;
const osMessageQueueAttr_t xWifi_Rx_Queue_attributes = {
  .name = "xWifi_Rx_Queue"
};
/* Definitions for xTest_Rx_Queue */
osMessageQueueId_t xTest_Rx_QueueHandle;
const osMessageQueueAttr_t xTest_Rx_Queue_attributes = {
  .name = "xTest_Rx_Queue"
};
/* Definitions for xMotor_Rx_Queue */
osMessageQueueId_t xMotor_Rx_QueueHandle;
const osMessageQueueAttr_t xMotor_Rx_Queue_attributes = {
  .name = "xMotor_Rx_Queue"
};
/* Definitions for xRfid_Rx_Queue */
osMessageQueueId_t xRfid_Rx_QueueHandle;
const osMessageQueueAttr_t xRfid_Rx_Queue_attributes = {
  .name = "xRfid_Rx_Queue"
};
/* Definitions for xMotion_Queue */
osMessageQueueId_t xMotion_QueueHandle;
const osMessageQueueAttr_t xMotion_Queue_attributes = {
  .name = "xMotion_Queue"
};
/* Definitions for xPrint_Queue */
osMessageQueueId_t xPrint_QueueHandle;
const osMessageQueueAttr_t xPrint_Queue_attributes = {
  .name = "xPrint_Queue"
};
/* Definitions for xBox_Ctrl_Queue */
osMessageQueueId_t xBox_Ctrl_QueueHandle;
const osMessageQueueAttr_t xBox_Ctrl_Queue_attributes = {
  .name = "xBox_Ctrl_Queue"
};
/* Definitions for xUV_Queue */
osMessageQueueId_t xUV_QueueHandle;
const osMessageQueueAttr_t xUV_Queue_attributes = {
  .name = "xUV_Queue"
};
/* Definitions for xHmi_Send_Queue */
osMessageQueueId_t xHmi_Send_QueueHandle;
const osMessageQueueAttr_t xHmi_Send_Queue_attributes = {
  .name = "xHmi_Send_Queue"
};
/* Definitions for xHmi_Recv_Queue */
osMessageQueueId_t xHmi_Recv_QueueHandle;
const osMessageQueueAttr_t xHmi_Recv_Queue_attributes = {
  .name = "xHmi_Recv_Queue"
};
/* Definitions for xWifi_Parse_Queue */
osMessageQueueId_t xWifi_Parse_QueueHandle;
const osMessageQueueAttr_t xWifi_Parse_Queue_attributes = {
  .name = "xWifi_Parse_Queue"
};
/* Definitions for xMqttManagerQueue */
osMessageQueueId_t xMqttManagerQueueHandle;
const osMessageQueueAttr_t xMqttManagerQueue_attributes = {
  .name = "xMqttManagerQueue"
};
/* Definitions for xRobotQueue */
osMessageQueueId_t xRobotQueueHandle;
const osMessageQueueAttr_t xRobotQueue_attributes = {
  .name = "xRobotQueue"
};
/* Definitions for xTcpManageQueue */
osMessageQueueId_t xTcpManageQueueHandle;
const osMessageQueueAttr_t xTcpManageQueue_attributes = {
  .name = "xTcpManageQueue"
};
/* Definitions for xMotorTxSem */
osSemaphoreId_t xMotorTxSemHandle;
const osSemaphoreAttr_t xMotorTxSem_attributes = {
  .name = "xMotorTxSem"
};
/* Definitions for xWifiTxSem */
osSemaphoreId_t xWifiTxSemHandle;
const osSemaphoreAttr_t xWifiTxSem_attributes = {
  .name = "xWifiTxSem"
};
/* Definitions for xPrintSem */
osSemaphoreId_t xPrintSemHandle;
const osSemaphoreAttr_t xPrintSem_attributes = {
  .name = "xPrintSem"
};
/* Definitions for xHMITxSem */
osSemaphoreId_t xHMITxSemHandle;
const osSemaphoreAttr_t xHMITxSem_attributes = {
  .name = "xHMITxSem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* creation of wifiUsartMutex */
  wifiUsartMutexHandle = osMutexNew(&wifiUsartMutex_attributes);

  /* creation of mqttMutex */
  mqttMutexHandle = osMutexNew(&mqttMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of xMotorTxSem */
  xMotorTxSemHandle = osSemaphoreNew(1, 1, &xMotorTxSem_attributes);

  /* creation of xWifiTxSem */
  xWifiTxSemHandle = osSemaphoreNew(1, 1, &xWifiTxSem_attributes);

  /* creation of xPrintSem */
  xPrintSemHandle = osSemaphoreNew(1, 1, &xPrintSem_attributes);

  /* creation of xHMITxSem */
  xHMITxSemHandle = osSemaphoreNew(1, 1, &xHMITxSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* creation of xCarRfidRxSem */
  xCarRfidRxSemHandle = osSemaphoreNew(1, 0, &xCarRfidRxSem_attributes);

  /* creation of xBoxRfidRxSem */
  xBoxRfidRxSemHandle = osSemaphoreNew(1, 0, &xBoxRfidRxSem_attributes);
  
  /* creation of xTestRxSem */
  xTestRxSemHandle = osSemaphoreNew(1, 0, &xTestRxSem_attributes);

  /* creation of xWifiReadySem */
  xWifiReadySemHandle = osSemaphoreNew(1, 0, &xWifiReadySem_attributes);

  /* creation of xWifiReadySem */
  xWifiRxSemHandle = osSemaphoreNew(1, 0, &xWifiRxSem_attributes);

  /* creation of xWifiReadySem */
  xMotorRxSemHandle = osSemaphoreNew(1, 0, &xMotorRxSem_attributes);

  /* creation of xBoxRfidRxSem */
  xHmiRxSemHandle = osSemaphoreNew(1, 0, &xHmiRxSem_attributes);

  /* USER CODE END RTOS_SEMAPHORES */
  /* creation of xResetButtonTimer */
  xResetButtonTimerHandle = osTimerNew(vResetButtonCallback, osTimerOnce, NULL, &xResetButtonTimer_attributes);

  /* creation of xSensorDebounceTimer */
  xSensorDebounceTimerHandle = osTimerNew(vSensorDebounceCallback, osTimerOnce, NULL, &xSensorDebounceTimer_attributes);

  /* creation of xToggleSwitchTimer */
  xToggleSwitchTimerHandle = osTimerNew(vToggleSwitchCallback, osTimerOnce, NULL, &xToggleSwitchTimer_attributes);

  /* creation of xBoxRfidLoginTimer */
  xBoxRfidLoginTimerHandle = osTimerNew(vBoxRfidLoginTimerCallback, osTimerOnce, NULL, &xBoxRfidLoginTimer_attributes);

  /* creation of xUVTimer */
  xUVTimerHandle = osTimerNew(vUVTimerCallback, osTimerOnce, NULL, &xUVTimer_attributes);

  /* creation of xBoxELockDebounceTimer */
  xBoxELockDebounceTimerHandle = osTimerNew(vBoxELockDebounceCallback, osTimerOnce, NULL, &xBoxELockDebounceTimer_attributes);

  /* creation of xServiceJoystickDebounceTimer */
  xServiceJoystickDebounceTimerHandle = osTimerNew(vServiceJoystickCallback, osTimerOnce, NULL, &xServiceJoystickDebounceTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* creation of xInterrupt_Queue */
  xInterrupt_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xInterrupt_Queue_attributes);
  /* creation of xSensor_Queue */
  xSensor_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xSensor_Queue_attributes);
  /* creation of xWifi_Rx_Queue */
  xWifi_Rx_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xWifi_Rx_Queue_attributes);
  /* creation of xTest_Rx_Queue */
  xTest_Rx_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xTest_Rx_Queue_attributes);
  /* creation of xMotor_Rx_Queue */
  xMotor_Rx_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xMotor_Rx_Queue_attributes);
  /* creation of xRfid_Rx_Queue */
  xRfid_Rx_QueueHandle = osMessageQueueNew (2, sizeof(uint32_t), &xRfid_Rx_Queue_attributes);
  /* creation of xMotion_Queue */
  xMotion_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xMotion_Queue_attributes);
  /* creation of xPrint_Queue */
  xPrint_QueueHandle = osMessageQueueNew (128, sizeof(uint32_t), &xPrint_Queue_attributes);
  /* creation of xBox_Ctrl_Queue */
  xBox_Ctrl_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xBox_Ctrl_Queue_attributes);
  /* creation of xUV_Queue */
  xUV_QueueHandle = osMessageQueueNew (2, sizeof(uint8_t), &xUV_Queue_attributes);
  /* creation of xHmi_Send_Queue */
  xHmi_Send_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xHmi_Send_Queue_attributes);
  /* creation of xHmi_Recv_Queue */
  xHmi_Recv_QueueHandle = osMessageQueueNew (5, sizeof(uint32_t), &xHmi_Recv_Queue_attributes);
  /* creation of xWifi_Parse_Queue */
  xWifi_Parse_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xWifi_Parse_Queue_attributes);
  /* creation of xMqttManagerQueue */
  xMqttManagerQueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xMqttManagerQueue_attributes);
  /* creation of xRobotQueue */
  xRobotQueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xRobotQueue_attributes);
  /* creation of xTcpManageQueue */
  xTcpManageQueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xTcpManageQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  vQueueAddToRegistry(xInterrupt_QueueHandle, "Interrupt_Queue");
  vQueueAddToRegistry(xSensor_QueueHandle, "Sensor_Queue");
  vQueueAddToRegistry(xWifi_Rx_QueueHandle, "Wifi_Rx_Queue");
  vQueueAddToRegistry(xTest_Rx_QueueHandle, "Test_Rx_Queue");
  vQueueAddToRegistry(xMotor_Rx_QueueHandle, "Motor_Rx_Queue");
  vQueueAddToRegistry(xRfid_Rx_QueueHandle, "Rfid_Rx_Queue");
  vQueueAddToRegistry(xMotion_QueueHandle, "Motion_Queue");
  vQueueAddToRegistry(xPrint_QueueHandle, "Print_Queue");
  vQueueAddToRegistry(xWifi_Parse_QueueHandle, "Wifi_Parse_Queue");
  vQueueAddToRegistry(xMqttManagerQueueHandle, "MqttManagerQueue");
  vQueueAddToRegistry(xRobotQueueHandle, "RobotQueue");
  vQueueAddToRegistry(xTcpManageQueueHandle, "TcpManageQueue");
  /* USER CODE END RTOS_QUEUES */
  /* creation of InitTask */
  InitTaskHandle = osThreadNew(vInitTask, NULL, &InitTask_attributes);

  /* creation of IntProcessTask */
  IntProcessTaskHandle = osThreadNew(vIntProcessTask, NULL, &IntProcessTask_attributes);

  /* creation of MotionCtrlTask */
  MotionCtrlTaskHandle = osThreadNew(vMotionCtrlTask, NULL, &MotionCtrlTask_attributes);

  /* creation of MotorFeedbackTask */
  MotorFeedbackTaskHandle = osThreadNew(vMotorFeedbackTask, NULL, &MotorFeedbackTask_attributes);

  /* creation of CarRfidTask */
  CarRfidTaskHandle = osThreadNew(vCarRfidTask, NULL, &CarRfidTask_attributes);

  /* creation of SensorTask */
  SensorTaskHandle = osThreadNew(vSensorTask, NULL, &SensorTask_attributes);

  /* creation of TestTask */
  TestTaskHandle = osThreadNew(vTestTask, NULL, &TestTask_attributes);

  /* creation of WifiManagerTask */
  WifiManagerTaskHandle = osThreadNew(vWifiManagerTask, NULL, &WifiManagerTask_attributes);

  /* creation of WifiReceiveTask */
  WifiReceiveTaskHandle = osThreadNew(vWifiReceiveTask, NULL, &WifiReceiveTask_attributes);

  /* creation of PrintTask */
  PrintTaskHandle = osThreadNew(vPrintTask, NULL, &PrintTask_attributes);

  /* creation of BoxCtrlTask */
  BoxCtrlTaskHandle = osThreadNew(vBoxCtrlTask, NULL, &BoxCtrlTask_attributes);

  /* creation of BoxRfidTask */
  BoxRfidTaskHandle = osThreadNew(vBoxRfidTask, NULL, &BoxRfidTask_attributes);

  /* creation of BoxRfidEventTask */
  BoxRfidEventTaskHandle = osThreadNew(vBoxRfidEventTask, NULL, &BoxRfidEventTask_attributes);

  /* creation of HmiEventTask */
  HmiEventTaskHandle = osThreadNew(vHmiEventTask, NULL, &HmiEventTask_attributes);

  /* creation of HmiSendTask */
  HmiSendTaskHandle = osThreadNew(vHmiSendTask, NULL, &HmiSendTask_attributes);

  /* creation of HmiRecvTask */
  HmiRecvTaskHandle = osThreadNew(vHmiRecvTask, NULL, &HmiRecvTask_attributes);

  /* creation of HmiWaitTask */
  HmiWaitTaskHandle = osThreadNew(vHmiWaitTask, NULL, &HmiWaitTask_attributes);

  /* creation of BoxLEDTask */
  BoxLEDTaskHandle = osThreadNew(vBoxLEDTask, NULL, &BoxLEDTask_attributes);

  /* creation of MqttManagerTask */
  MqttManagerTaskHandle = osThreadNew(vMqttManagerTask, NULL, &MqttManagerTask_attributes);

  /* creation of MqttReceiveTask */
  MqttReceiveTaskHandle = osThreadNew(vMqttReceiveTask, NULL, &MqttReceiveTask_attributes);

  /* creation of RobotManagerTask */
  RobotManagerTaskHandle = osThreadNew(vRobotManagerTask, NULL, &RobotManagerTask_attributes);

  /* creation of RobotReceiveTask */
  RobotReceiveTaskHandle = osThreadNew(vRobotReceiveTask, NULL, &RobotReceiveTask_attributes);

  /* creation of TcpManagerTask */
  TcpManagerTaskHandle = osThreadNew(vTcpManagerTask, NULL, &TcpManagerTask_attributes);

  /* creation of TcpReceiveTask */
  TcpReceiveTaskHandle = osThreadNew(vTcpReceiveTask, NULL, &TcpReceiveTask_attributes);

  /* creation of RobotHeartBeatTask */
  RobotHeartBeatTaskHandle = osThreadNew(vRobotHeartBeatTask, NULL, &RobotHeartBeatTask_attributes);

  /* creation of MqttNotifyTask */
  MqttNotifyTaskHandle = osThreadNew(vMqttNotifyTask, NULL, &MqttNotifyTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  //先让InitTask之外的其他task休眠
  osThreadSuspend(IntProcessTaskHandle);
  osThreadSuspend(MotionCtrlTaskHandle);
  osThreadSuspend(MotorFeedbackTaskHandle);
  osThreadSuspend(CarRfidTaskHandle);
  osThreadSuspend(SensorTaskHandle);
  osThreadSuspend(TestTaskHandle);
  osThreadSuspend(WifiManagerTaskHandle);
  osThreadSuspend(WifiReceiveTaskHandle);
  osThreadSuspend(PrintTaskHandle);
  osThreadSuspend(BoxCtrlTaskHandle);
  osThreadSuspend(BoxRfidTaskHandle);
  osThreadSuspend(BoxRfidEventTaskHandle);
  osThreadSuspend(HmiEventTaskHandle);
  osThreadSuspend(HmiSendTaskHandle);
  osThreadSuspend(HmiRecvTaskHandle);
  osThreadSuspend(HmiWaitTaskHandle);
  osThreadSuspend(BoxLEDTaskHandle);
  osThreadSuspend(MqttManagerTaskHandle);
  osThreadSuspend(MqttReceiveTaskHandle);  
  osThreadSuspend(RobotManagerTaskHandle); 
  osThreadSuspend(RobotReceiveTaskHandle);
  osThreadSuspend(TcpManagerTaskHandle); 
  osThreadSuspend(TcpReceiveTaskHandle); 
  osThreadSuspend(RobotHeartBeatTaskHandle);
  osThreadSuspend(MqttNotifyTaskHandle); 
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */

  /* USER CODE END RTOS_EVENTS */

}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

