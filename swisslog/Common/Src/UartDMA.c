#include "UartDMA.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "LogDebugInfo.h"


// 启动GPDMA接收
void v_Swisslog_Start_DMA_Receive(UART_HandleTypeDef* huart, 
                                    uint8_t* puc_RxBuffer, 
                                    uint32_t u32_rxBuffSize)
{
  // 1. 清除接收非空（RXNE）和溢出（ORE）标志
  __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);
  __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);

  // 2. 读取RDR寄存器（强制清空残留数据，即使无数据也不会阻塞）
  uint8_t temp;
  HAL_UART_Receive(huart, &temp, 1, 0); // 超时时间设为0，立即返回

  // 3. 启动DMA接收（此时无残留数据，接收正常）
  if(HAL_UARTEx_ReceiveToIdle_DMA(huart, puc_RxBuffer, u32_rxBuffSize)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 第一次启动DMA接收失败后，再次启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(huart, puc_RxBuffer, u32_rxBuffSize);
  }

  // 关闭半传输+全传输中断
  __HAL_DMA_DISABLE_IT((*huart).hdmarx, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT((*huart).hdmarx, DMA_IT_TC);
}


/**
 * @brief  当缓冲区绕回时，将两段数据合并为一段
 * @param  pu8_rxBuff: 原始缓冲区
 * @param  u32_rxBuffSize: 原始缓冲区大小
 * @param  startIdx:  数据起始下标
 * @param  dataLen:   数据长度
 * @param  pTempBuf:  临时缓冲区（用于拼接绕回数据存）
 * @retval 指向连续完整数据的指针：
 *         1. 未绕回：返回原缓冲区的起始位置（&pu8_rxBuff[startIdx]）
 *         2. 绕回：返回临时缓冲区（pTempBuf）
 *         3. 参数错误：返回NULL
 */
uint8_t* pu8_Swisslog_Rx_Buffer_Wrap_process(uint8_t* pu8_rxBuff,
                                        uint32_t u32_rxBuffSize,
										uint32_t startIdx,
										uint32_t dataLen,
										uint8_t* pTempBuf)
{
    memset(pTempBuf, 0, dataLen);

    //  判断是否绕回
    if (startIdx + dataLen <= u32_rxBuffSize) {
        memcpy(pTempBuf, &pu8_rxBuff[startIdx], dataLen);
        return pTempBuf;
    } else {
        // 绕回 - 数据分两段，拼接至临时缓冲区
        uint32_t len1 = u32_rxBuffSize - startIdx;  // 第一段长度（起始位置→缓冲区末尾）
        uint32_t len2 = dataLen - len1;        	    // 第二段长度（缓冲区开头→剩余数据）

        // 拼接第一段数据（起始位置→缓冲区末尾）
        memcpy(pTempBuf, &pu8_rxBuff[startIdx], len1);
        // 拼接第二段数据（缓冲区开头→剩余数据，接在第一段后）
        memcpy(&pTempBuf[len1], pu8_rxBuff, len2);
        return pTempBuf;
    }
}
