#include "main.h"
#include "Task_rs485.h"
#include "usart.h"
#include "gpio.h"   // GPIO操作头文件
#include "FreeRTOS.h"
#include "task.h"   // 补充：FreeRTOS任务/临界区/延时函数的核心头文件
#include <stdio.h>

// 全局变量定义（中断和任务共享）
uint8_t RS485_RxBuf[RS485_RX_BUF_LEN] = {0};  // 需确保Task_rs485.h中已定义该宏
uint8_t ucRxWritePtr = 0;                     // 缓冲区写指针（中断用）
uint8_t ucRxReadPtr = 0;                      // 缓冲区读指针（任务用）
uint8_t ucRxDataFlag = 0;                     // 有数据待处理标志
uint8_t ucRxTempByte = 0;                     // 中断临时接收字节

extern UART_HandleTypeDef huart1;

/**
 * @brief  485接收初始化（补全逻辑，修正变量）
 */
void RS485_Interrupt_Receive_Init(void)
{
    // 1. 初始化485为接收模式（需确保Task_rs485.h中已定义该宏）
    RS485_SET_RECEIVE();

    // 2. 开启串口2字节接收中断（使用全局临时字节变量）
    HAL_UART_Receive_IT(&huart1, &ucRxTempByte, 1);
}

/**
 * @brief  RS485接收任务（保持原有逻辑不变）
 */
void rs485_communicate(void *argument)
{
    uint8_t ucRecvData = 0;

    for (;;)
    {
        if (ucRxDataFlag == 1)
        {
            // 替换：HAL临界区保护
            __disable_irq();

            ucRecvData = RS485_RxBuf[ucRxReadPtr];
            ucRxReadPtr = (ucRxReadPtr + 1) % RS485_RX_BUF_LEN;

            if (ucRxReadPtr == ucRxWritePtr)
            {
                ucRxDataFlag = 0;
            }

            __enable_irq();

            // 业务逻辑：打印接收数据
            printf("RS485接收：0x%02X | ASCII：%c\r\n", ucRecvData, ucRecvData);
        }

        vTaskDelay(1);
    }
}
