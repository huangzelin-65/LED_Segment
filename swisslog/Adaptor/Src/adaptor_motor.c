#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_motor.h"
#include "LogDebugInfo.h"


// 双缓冲区（防止处理期间数据被覆盖）
// uint8_t ucMotor_Rx_Buffer[2][MOTOR_RX_BUF_SIZE];
// uint8_t ucMotor_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern UART_HandleTypeDef huart7;
extern osMessageQueueId_t xMotor_Rx_Queue;
extern osSemaphoreId_t xMotorTxSemHandle;


void vSendToMotor(uint8_t * CmdDataArr,uint8_t len)
{
    // DEBUGINFO("send len = %d ,data:",len);
    // vPrint_Array(CmdDataArr,len);

    // 获取RS485发送锁
    if (osSemaphoreAcquire(xMotorTxSemHandle, osWaitForever) == osOK) 
    {
        Motor_RS485_TX_MODE(); // 使能发送模式(在发送完成回调中切回RX模式)

        taskENTER_CRITICAL(); // 进入临界区
        HAL_UART_Transmit_DMA(&huart7, CmdDataArr, len);    // 启动DMA发送
        //HAL_UART_Transmit_IT(&huart7, CmdDataArr, len); // 启动中断发送
        taskEXIT_CRITICAL(); // 退出临界区
    }
}


// 启动Motor的GPDMA接收
void vMotor_Start_DMA_Receive(uint8_t* ucMotor_Rx_Buffer) 
{
    v_Swisslog_Start_DMA_Receive(&huart7, ucMotor_Rx_Buffer, MOTOR_RX_BUF_SIZE);
}


// 获取当前DMA接收的数据长度
uint32_t u32_Motor_Get_DMA_Receive_Len(uint32_t* pulStartIdx)
{
    uint32_t u32_len = u32_Swisslog_Get_DMA_Receive_Len(&huart7, pulStartIdx, MOTOR_RX_BUF_SIZE);
    return u32_len;
}

