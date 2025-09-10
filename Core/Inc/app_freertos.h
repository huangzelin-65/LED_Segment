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

/* USER CODE END EM */
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
extern osTimerId_t xSensorDebounceTimerHandle;
extern osTimerId_t xToggleSwitchTimerHandle;
extern osTimerId_t xResetButtonTimerHandle;
extern osMessageQueueId_t xPrint_QueueHandle;
extern osMessageQueueId_t xInterrupt_QueueHandle;
extern osMessageQueueId_t xMotion_QueueHandle;
extern osMessageQueueId_t xSensor_QueueHandle;
extern osMessageQueueId_t xWifi_Rx_QueueHandle;
extern osMessageQueueId_t xTest_Rx_QueueHandle;
extern osMessageQueueId_t xMotor_Rx_QueueHandle;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;
extern osSemaphoreId_t xMotorTxSemHandle;
extern osSemaphoreId_t xWifiTxSemHandle;
extern osSemaphoreId_t xWifiReadySemHandle;
extern osSemaphoreId_t xPrintSemHandle;

/* Exported function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

extern void vInitTask(void *argument);
extern void vIntProcessTask(void *argument);
extern void vMotionCtrlTask(void *argument);
extern void vMotorFeedbackTask(void *argument);
extern void vRfidTask(void *argument);
extern void vSensorTask(void *argument);
extern void vTestTask(void *argument);
extern void vWifiManagerTask(void *argument);
extern void vWifiReceiveTask(void *argument);
extern void vPrintTask(void *argument);
extern void vSensorDebounceCallback(void *argument);
extern void vToggleSwitchCallback(void *argument);
extern void vResetButtonCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

#ifdef __cplusplus
}
#endif
#endif /* __APP_FREERTOS_H */
