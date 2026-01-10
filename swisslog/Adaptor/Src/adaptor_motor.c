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
//extern DMA_HandleTypeDef hdma_usart7_rx;


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
void vMotor_Start_DMA_Receive(uint8_t* ucMotor_Rx_Buffer) {
  // 1. 清除接收非空（RXNE）和溢出（ORE）标志
  __HAL_UART_CLEAR_FLAG(&huart7, UART_FLAG_RXNE);
  __HAL_UART_CLEAR_FLAG(&huart7, UART_FLAG_ORE);

  // 2. 读取RDR寄存器（强制清空残留数据，即使无数据也不会阻塞）
  uint8_t temp;
  HAL_UART_Receive(&huart7, &temp, 1, 0); // 超时时间设为0，立即返回

  // 3. 启动DMA接收（此时无残留数据，接收正常）
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart7, ucMotor_Rx_Buffer, MOTOR_RX_BUF_SIZE)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 第一次启动DMA接收失败后，再次启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart7, ucMotor_Rx_Buffer, MOTOR_RX_BUF_SIZE);
  }

  // 关闭半传输+全传输中断
  __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT(huart7.hdmarx, DMA_IT_TC);
}

// 获取当前DMA接收的数据长度
// uint32_t ulMotor_Get_DMA_Receive_Len(void)
// {
//   uint32_t ulLen = MOTOR_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart7.hdmarx);
//   return ulLen;
// }

// 获取当前DMA接收的数据长度
uint32_t ulMotor_Get_DMA_Receive_Len(uint32_t* pulStartIdx)
{
	static uint32_t ulCurrentLen= 0;
	static uint32_t ulLastLen = 0 ;
	uint32_t ulLen = 0 ;
	if (pulStartIdx != NULL) {
		*pulStartIdx = ulLastLen % MOTOR_RX_BUF_SIZE;; // 起始位置 = 上一次的累计长度
	}

	// 计算当前DMA累计长度
	ulCurrentLen = MOTOR_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart7.hdmarx);

	// 计算单包长度（缓冲区绕回/没有绕回）
	if(ulCurrentLen > ulLastLen) {
		ulLen = ulCurrentLen - ulLastLen ;
	} else {
		ulLen = (MOTOR_RX_BUF_SIZE - ulLastLen) + ulCurrentLen;
	}

	ulLastLen = ulCurrentLen ;

	return ulLen;	// 返回单包长度
}

/**
 * @brief  当缓冲区绕回时，将两段数据合并为一段
 * @param  srcBufSize: 原始缓冲区大小（MOTOR_RX_BUF_SIZE）
 * @param  startIdx:  数据起始下标（package_start_idx）
 * @param  dataLen:   数据长度（ucReciveLen）
 * @param  pTempBuf:  临时缓冲区（用于拼接绕回数据存）
 * @param  tempBufSize: 临时缓冲区大小
 * @retval 指向连续完整数据的指针：
 *         1. 未绕回：返回原缓冲区的起始位置（&pSrcBuf[startIdx]）
 *         2. 绕回：返回临时缓冲区（pTempBuf）
 *         3. 参数错误：返回NULL
 */
uint8_t* ucMotor_Rx_Buffer_Wrap_process(uint8_t* pSrcBuf,
											uint32_t startIdx,
											uint32_t dataLen,
											uint8_t* pTempBuf)
{
    memset(pTempBuf, 0, dataLen);

    //  判断是否绕回
    if (startIdx + dataLen <= MOTOR_RX_BUF_SIZE) {
        memcpy(pTempBuf, &pSrcBuf[startIdx], dataLen);
        return pTempBuf;
    } else {
        // 绕回 - 数据分两段，拼接至临时缓冲区
        uint32_t len1 = MOTOR_RX_BUF_SIZE - startIdx;  // 第一段长度（起始位置→缓冲区末尾）
        uint32_t len2 = dataLen - len1;        		     // 第二段长度（缓冲区开头→剩余数据）

        // 拼接第一段数据（起始位置→缓冲区末尾）
        memcpy(pTempBuf, &pSrcBuf[startIdx], len1);
        // 拼接第二段数据（缓冲区开头→剩余数据，接在第一段后）
        memcpy(&pTempBuf[len1], pSrcBuf, len2);
        return pTempBuf;
    }
}