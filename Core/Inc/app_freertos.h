/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.h
  * Description        : FreeRTOS applicative header file
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_FREERTOS_H
#define __APP_FREERTOS_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported macro -------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern osSemaphoreId_t xCarRfidRxSemHandle;
extern osSemaphoreId_t xBoxRfidRxSemHandle;
extern osSemaphoreId_t xTestRxSemHandle;
extern osSemaphoreId_t xWifiReadySemHandle;
extern osSemaphoreId_t xWifiRxSemHandle;
extern osSemaphoreId_t xMotorRxSemHandle;
extern osSemaphoreId_t xHmiRxSemHandle;
/* USER CODE END EM */
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
extern osMutexId_t wifiUsartMutexHandle;
extern osMutexId_t mqttMutexHandle;
extern osTimerId_t xResetButtonTimerHandle;
extern osTimerId_t xSensorDebounceTimerHandle;
extern osTimerId_t xToggleSwitchTimerHandle;
extern osTimerId_t xBoxRfidLoginTimerHandle;
extern osTimerId_t xUVTimerHandle;
extern osTimerId_t xBoxELockDebounceTimerHandle;
extern osTimerId_t xServiceJoystickDebounceTimerHandle;
extern osMessageQueueId_t xInterrupt_QueueHandle;
extern osMessageQueueId_t xSensor_QueueHandle;
extern osMessageQueueId_t xWifi_Rx_QueueHandle;
extern osMessageQueueId_t xTest_Rx_QueueHandle;
extern osMessageQueueId_t xMotor_Rx_QueueHandle;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;
extern osMessageQueueId_t xMotion_QueueHandle;
extern osMessageQueueId_t xPrint_QueueHandle;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;
extern osMessageQueueId_t xUV_QueueHandle;
extern osMessageQueueId_t xHmi_Send_QueueHandle;
extern osMessageQueueId_t xHmi_Recv_QueueHandle;
extern osMessageQueueId_t xWifi_Parse_QueueHandle;
extern osMessageQueueId_t xMqttManagerQueueHandle;
extern osMessageQueueId_t xRobotQueueHandle;
extern osMessageQueueId_t xTcpManageQueueHandle;
extern osSemaphoreId_t xMotorTxSemHandle;
extern osSemaphoreId_t xWifiTxSemHandle;
extern osSemaphoreId_t xPrintSemHandle;
extern osSemaphoreId_t xHMITxSemHandle;

/* Exported function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

extern void vInitTask(void *argument);
extern void vIntProcessTask(void *argument);
extern void vMotionCtrlTask(void *argument);
extern void vMotorFeedbackTask(void *argument);
extern void vCarRfidTask(void *argument);
extern void vSensorTask(void *argument);
extern void vTestTask(void *argument);
extern void vWifiManagerTask(void *argument);
extern void vWifiReceiveTask(void *argument);
extern void vPrintTask(void *argument);
extern void vBoxCtrlTask(void *argument);
extern void vBoxRfidTask(void *argument);
extern void vBoxRfidEventTask(void *argument);
extern void vHmiEventTask(void *argument);
extern void vHmiSendTask(void *argument);
extern void vHmiRecvTask(void *argument);
extern void vHmiWaitTask(void *argument);
extern void vBoxLEDTask(void *argument);
extern void vMqttManagerTask(void *argument);
extern void vMqttReceiveTask(void *argument);
extern void vRobotManagerTask(void *argument);
extern void vRobotReceiveTask(void *argument);
extern void vTcpManagerTask(void *argument);
extern void vTcpReceiveTask(void *argument);
extern void vRobotHeartBeatTask(void *argument);
extern void vResetButtonCallback(void *argument);
extern void vSensorDebounceCallback(void *argument);
extern void vToggleSwitchCallback(void *argument);
extern void vBoxRfidLoginTimerCallback(void *argument);
extern void vUVTimerCallback(void *argument);
extern void vBoxELockDebounceCallback(void *argument);
extern void vServiceJoystickCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FREERTOS_H */
