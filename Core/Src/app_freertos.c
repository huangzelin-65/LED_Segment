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
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};
/* Definitions for IntProcessTask */
osThreadId_t IntProcessTaskHandle;
const osThreadAttr_t IntProcessTask_attributes = {
  .name = "IntProcessTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
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
/* Definitions for RfidTask */
osThreadId_t RfidTaskHandle;
const osThreadAttr_t RfidTask_attributes = {
  .name = "RfidTask",
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
  .stack_size = 128 * 4
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
  .stack_size = 128 * 4
};
/* Definitions for PrintTask */
osThreadId_t PrintTaskHandle;
const osThreadAttr_t PrintTask_attributes = {
  .name = "PrintTask",
  .priority = (osPriority_t) osPriorityBelowNormal7,
  .stack_size = 256 * 4
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
/* Definitions for xWifiReadySem */
osSemaphoreId_t xWifiReadySemHandle;
const osSemaphoreAttr_t xWifiReadySem_attributes = {
  .name = "xWifiReadySem"
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

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of xMotorTxSem */
  xMotorTxSemHandle = osSemaphoreNew(1, 1, &xMotorTxSem_attributes);

  /* creation of xWifiTxSem */
  xWifiTxSemHandle = osSemaphoreNew(1, 1, &xWifiTxSem_attributes);

  /* creation of xWifiReadySem */
  xWifiReadySemHandle = osSemaphoreNew(1, 1, &xWifiReadySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */
  /* creation of xResetButtonTimer */
  xResetButtonTimerHandle = osTimerNew(vResetButtonCallback, osTimerOnce, NULL, &xResetButtonTimer_attributes);

  /* creation of xSensorDebounceTimer */
  xSensorDebounceTimerHandle = osTimerNew(vSensorDebounceCallback, osTimerOnce, NULL, &xSensorDebounceTimer_attributes);

  /* creation of xToggleSwitchTimer */
  xToggleSwitchTimerHandle = osTimerNew(vToggleSwitchCallback, osTimerOnce, NULL, &xToggleSwitchTimer_attributes);

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
  xRfid_Rx_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xRfid_Rx_Queue_attributes);
  /* creation of xMotion_Queue */
  xMotion_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xMotion_Queue_attributes);
  /* creation of xPrint_Queue */
  xPrint_QueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &xPrint_Queue_attributes);

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


  /* USER CODE END RTOS_QUEUES */
  /* creation of InitTask */
  InitTaskHandle = osThreadNew(vInitTask, NULL, &InitTask_attributes);

  /* creation of IntProcessTask */
  IntProcessTaskHandle = osThreadNew(vIntProcessTask, NULL, &IntProcessTask_attributes);

  /* creation of MotionCtrlTask */
  MotionCtrlTaskHandle = osThreadNew(vMotionCtrlTask, NULL, &MotionCtrlTask_attributes);

  /* creation of MotorFeedbackTask */
  MotorFeedbackTaskHandle = osThreadNew(vMotorFeedbackTask, NULL, &MotorFeedbackTask_attributes);

  /* creation of RfidTask */
  RfidTaskHandle = osThreadNew(vRfidTask, NULL, &RfidTask_attributes);

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

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  
  //先让InitTask之外的其他task休眠
  osThreadSuspend(IntProcessTaskHandle);
  osThreadSuspend(MotionCtrlTaskHandle);
  osThreadSuspend(MotorFeedbackTaskHandle);
  osThreadSuspend(RfidTaskHandle);
  osThreadSuspend(SensorTaskHandle);
  osThreadSuspend(TestTaskHandle);
  osThreadSuspend(WifiManagerTaskHandle);
  osThreadSuspend(WifiReceiveTaskHandle);
  osThreadSuspend(PrintTaskHandle);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */

  /* USER CODE END RTOS_EVENTS */

}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

