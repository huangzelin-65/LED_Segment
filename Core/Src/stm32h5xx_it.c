/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32h5xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32h5xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "queue.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include "adaptor_motor.h"
#include "adaptor_wifi.h"
#include "adaptor_rfid.h"
#include "adaptor_test.h"
#include "adaptor_HMI.h"
#include "LogDebugInfo.h"
#include "sensors.h"
#include <stdbool.h>
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
BaseType_t xHigherPriorityTaskWoken;
eInterruptType msg;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel6;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA2_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel5;
extern DMA_HandleTypeDef handle_GPDMA1_Channel4;
extern DMA_HandleTypeDef handle_GPDMA1_Channel3;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
extern DMA_HandleTypeDef handle_GPDMA2_Channel2;
extern DMA_HandleTypeDef handle_GPDMA2_Channel1;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart8;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart10;
extern UART_HandleTypeDef huart11;
extern TIM_HandleTypeDef htim2;

/* USER CODE BEGIN EV */
extern osMessageQueueId_t xInterrupt_QueueHandle;

extern osSemaphoreId_t xMotorTxSemHandle;
extern osSemaphoreId_t xWifiTxSemHandle;
extern osSemaphoreId_t xPrintSemHandle;
extern osSemaphoreId_t xCarRfidRxSemHandle;
extern osSemaphoreId_t xBoxRfidRxSemHandle;
extern osSemaphoreId_t xTestRxSemHandle;
extern osSemaphoreId_t xWifiRxSemHandle;
extern osSemaphoreId_t xMotorRxSemHandle;
extern osSemaphoreId_t xHmiRxSemHandle;
extern osSemaphoreId_t xHMITxSemHandle;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32H5xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32h5xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI Line0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */

  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(FP_L_Pin);
  /* USER CODE BEGIN EXTI0_IRQn 1 */
  msg = FrontProxSensor; // 前距离传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line1 interrupt.
  */
void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */

  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(TOGGLE_BACK_Pin);
  /* USER CODE BEGIN EXTI1_IRQn 1 */
  msg = ToggleSwitch; // 拨动开关
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI1_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */

  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(ELOCK1_STATUS_Pin);
  /* USER CODE BEGIN EXTI2_IRQn 1 */
  msg = BoxELock; // 车厢电子锁
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line3 interrupt.
  */
void EXTI3_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI3_IRQn 0 */

  /* USER CODE END EXTI3_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(RP_L_Pin);
  /* USER CODE BEGIN EXTI3_IRQn 1 */
  msg = RearProxSensor; // 后距离传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI3_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */

  /* USER CODE END EXTI4_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(TOGGLE_FRONT_Pin);
  /* USER CODE BEGIN EXTI4_IRQn 1 */
  msg = ToggleSwitch; // 拨动开关
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line5 interrupt.
  */
void EXTI5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI5_IRQn 0 */

  /* USER CODE END EXTI5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(FC_H_Pin);
  /* USER CODE BEGIN EXTI5_IRQn 1 */
  msg = FrontCrashSensor; // 前碰撞传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI5_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line6 interrupt.
  */
void EXTI6_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI6_IRQn 0 */

  /* USER CODE END EXTI6_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(ELOCK2_STATUS_Pin);
  /* USER CODE BEGIN EXTI6_IRQn 1 */
  msg = BoxELock; // 车厢电子锁
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI6_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line9 interrupt.
  */
void EXTI9_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_IRQn 0 */

  /* USER CODE END EXTI9_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(RC_L_Pin);
  /* USER CODE BEGIN EXTI9_IRQn 1 */
  msg = RearCrashSensor; // 后碰撞传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI9_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line10 interrupt.
  */
void EXTI10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI10_IRQn 0 */

  /* USER CODE END EXTI10_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(RC_H_Pin);
  /* USER CODE BEGIN EXTI10_IRQn 1 */
  msg = RearCrashSensor; // 后碰撞传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI10_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line11 interrupt.
  */
void EXTI11_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI11_IRQn 0 */

  /* USER CODE END EXTI11_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(FP_H_Pin);
  /* USER CODE BEGIN EXTI11_IRQn 1 */
  msg = FrontProxSensor; // 前距离传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI11_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line12 interrupt.
  */
void EXTI12_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI12_IRQn 0 */

  /* USER CODE END EXTI12_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(FC_L_Pin);
  /* USER CODE BEGIN EXTI12_IRQn 1 */
  msg = FrontCrashSensor; // 前碰撞传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI12_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line13 interrupt.
  */
void EXTI13_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI13_IRQn 0 */

  /* USER CODE END EXTI13_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(RP_H_Pin);
  /* USER CODE BEGIN EXTI13_IRQn 1 */
  msg = RearProxSensor; // 后距离传感器
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI13_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line14 interrupt.
  */
void EXTI14_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI14_IRQn 0 */

  /* USER CODE END EXTI14_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(LOW_VOLTAGE_DET_Pin);
  /* USER CODE BEGIN EXTI14_IRQn 1 */
  msg = LowVoltageDetect; // 低电压检测
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI14_IRQn 1 */
}

/**
  * @brief This function handles EXTI Line15 interrupt.
  */
void EXTI15_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_IRQn 0 */

  /* USER CODE END EXTI15_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(RESET_Pin);
  /* USER CODE BEGIN EXTI15_IRQn 1 */
  msg = ResetButton; // RESET按键
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
  /* USER CODE END EXTI15_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 0 global interrupt.
  */
void GPDMA1_Channel0_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel0_IRQn 0 */

  /* USER CODE END GPDMA1_Channel0_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel0);
  /* USER CODE BEGIN GPDMA1_Channel0_IRQn 1 */

  /* USER CODE END GPDMA1_Channel0_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 1 global interrupt.
  */
void GPDMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel1_IRQn 0 */

  /* USER CODE END GPDMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);
  /* USER CODE BEGIN GPDMA1_Channel1_IRQn 1 */

  /* USER CODE END GPDMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 2 global interrupt.
  */
void GPDMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel2_IRQn 0 */

  /* USER CODE END GPDMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel2);
  /* USER CODE BEGIN GPDMA1_Channel2_IRQn 1 */

  /* USER CODE END GPDMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 3 global interrupt.
  */
void GPDMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel3_IRQn 0 */

  /* USER CODE END GPDMA1_Channel3_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel3);
  /* USER CODE BEGIN GPDMA1_Channel3_IRQn 1 */

  /* USER CODE END GPDMA1_Channel3_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 4 global interrupt.
  */
void GPDMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel4_IRQn 0 */

  /* USER CODE END GPDMA1_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel4);
  /* USER CODE BEGIN GPDMA1_Channel4_IRQn 1 */

  /* USER CODE END GPDMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 5 global interrupt.
  */
void GPDMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel5_IRQn 0 */

  /* USER CODE END GPDMA1_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel5);
  /* USER CODE BEGIN GPDMA1_Channel5_IRQn 1 */

  /* USER CODE END GPDMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles GPDMA1 Channel 6 global interrupt.
  */
void GPDMA1_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA1_Channel6_IRQn 0 */

  /* USER CODE END GPDMA1_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel6);
  /* USER CODE BEGIN GPDMA1_Channel6_IRQn 1 */

  /* USER CODE END GPDMA1_Channel6_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles I2C1 Event interrupt.
  */
void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */

  /* USER CODE END I2C1_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C1 Error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */

  /* USER CODE END I2C1_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_ER_IRQn 1 */

  /* USER CODE END I2C1_ER_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles UART5 global interrupt.
  */
void UART5_IRQHandler(void)
{
  /* USER CODE BEGIN UART5_IRQn 0 */

  /* USER CODE END UART5_IRQn 0 */
  HAL_UART_IRQHandler(&huart5);
  /* USER CODE BEGIN UART5_IRQn 1 */

  /* USER CODE END UART5_IRQn 1 */
}

/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */

  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */

  /* USER CODE END USART6_IRQn 1 */
}

/**
  * @brief This function handles USART10 global interrupt.
  */
void USART10_IRQHandler(void)
{
  /* USER CODE BEGIN USART10_IRQn 0 */

  /* USER CODE END USART10_IRQn 0 */
  HAL_UART_IRQHandler(&huart10);
  /* USER CODE BEGIN USART10_IRQn 1 */

  /* USER CODE END USART10_IRQn 1 */
}

/**
  * @brief This function handles USART11 global interrupt.
  */
void USART11_IRQHandler(void)
{
  /* USER CODE BEGIN USART11_IRQn 0 */

  /* USER CODE END USART11_IRQn 0 */
  HAL_UART_IRQHandler(&huart11);
  /* USER CODE BEGIN USART11_IRQn 1 */

  /* USER CODE END USART11_IRQn 1 */
}

/**
  * @brief This function handles GPDMA2 Channel 0 global interrupt.
  */
void GPDMA2_Channel0_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA2_Channel0_IRQn 0 */

  /* USER CODE END GPDMA2_Channel0_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA2_Channel0);
  /* USER CODE BEGIN GPDMA2_Channel0_IRQn 1 */

  /* USER CODE END GPDMA2_Channel0_IRQn 1 */
}

/**
  * @brief This function handles GPDMA2 Channel 1 global interrupt.
  */
void GPDMA2_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA2_Channel1_IRQn 0 */

  /* USER CODE END GPDMA2_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA2_Channel1);
  /* USER CODE BEGIN GPDMA2_Channel1_IRQn 1 */

  /* USER CODE END GPDMA2_Channel1_IRQn 1 */
}

/**
  * @brief This function handles GPDMA2 Channel 2 global interrupt.
  */
void GPDMA2_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN GPDMA2_Channel2_IRQn 0 */

  /* USER CODE END GPDMA2_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&handle_GPDMA2_Channel2);
  /* USER CODE BEGIN GPDMA2_Channel2_IRQn 1 */

  /* USER CODE END GPDMA2_Channel2_IRQn 1 */
}

/**
  * @brief This function handles UART7 global interrupt.
  */
void UART7_IRQHandler(void)
{
  /* USER CODE BEGIN UART7_IRQn 0 */

  /* USER CODE END UART7_IRQn 0 */
  HAL_UART_IRQHandler(&huart7);
  /* USER CODE BEGIN UART7_IRQn 1 */

  /* USER CODE END UART7_IRQn 1 */
}

/**
  * @brief This function handles UART8 global interrupt.
  */
void UART8_IRQHandler(void)
{
  /* USER CODE BEGIN UART8_IRQn 0 */

  /* USER CODE END UART8_IRQn 0 */
  HAL_UART_IRQHandler(&huart8);
  /* USER CODE BEGIN UART8_IRQn 1 */

  /* USER CODE END UART8_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if (huart->Instance == USART1)
  {
    //Test串口接收处理
    uint16_t dataLength = ulTest_Get_DMA_Receive_Len();

    if (dataLength > 0) 
    {
      osSemaphoreRelease(xTestRxSemHandle);  // 释放信号量,允许读取Test命令数据
    }
  }
  else if (huart->Instance == UART5) {
    //Car Rfid串口接收处理
    uint16_t dataLength = ulCarRfid_Get_DMA_Receive_Len();

    if (dataLength > 0) 
    {
      osSemaphoreRelease(xCarRfidRxSemHandle);  // 释放信号量,允许读取RFID数据
    }

  }
  else if (huart->Instance == USART6) {
    //Wifi串口接收处理
    uint16_t dataLength = ulWifi_Get_DMA_Receive_Len();

    if (dataLength > 0) 
    {
      osSemaphoreRelease(xWifiRxSemHandle);  // 释放信号量,允许读取wifi数据
    }
  }
  else if (huart->Instance == UART7)
  {
    //motor串口接收处理
    uint16_t dataLength = ulMotor_Get_DMA_Receive_Len();
    if (dataLength > 0) 
    {
      osSemaphoreRelease(xMotorRxSemHandle);  // 释放信号量,允许读取motor数据
    }
  }
  else if (huart->Instance == UART8) {
    //Box Rfid串口接收处理
    uint32_t dataLength = ulBoxRfid_Get_DMA_Receive_Len();

    if (dataLength > 0) 
    {
      osSemaphoreRelease(xBoxRfidRxSemHandle);  // 释放信号量,允许读取RFID数据
    }
  }
  else if (huart->Instance == USART10) {
    //HMI串口接收处理
    uint32_t dataLength = ulHMI_Get_DMA_Receive_Len();

    if (dataLength > 0) 
    {
      osSemaphoreRelease(xHmiRxSemHandle);  // 释放信号量,允许读取HMI数据
    }
  }

}


/**
 * @brief UART发送完成回调函数
 *
 * 当UART发送完成时调用此函数，切换到接收模式并释放信号量，允许下一次485发送。
 *
 * @param huart UART句柄
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    char* tx_data = (char*)huart->pTxBuffPtr;
    vPortFree(tx_data);
    osSemaphoreRelease(xPrintSemHandle);  // 释放信号量,允许下一次打印
  }
  else if (huart->Instance == USART6)
  {
    osSemaphoreRelease(xWifiTxSemHandle);  // 释放信号量,允许下一次wifi发送
  }
  else if (huart->Instance == UART7)
  {
    Motor_RS485_RX_MODE(); // 发送完成切接收模式
    
    osSemaphoreRelease(xMotorTxSemHandle);  // 释放信号量,允许下一次485电机发送
  }
  else if (huart->Instance == USART10)
  {
    osSemaphoreRelease(xHMITxSemHandle);  // 释放信号量,允许下一次HMI发送
  }
}


/* USER CODE END 1 */
