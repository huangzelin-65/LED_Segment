#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "adaptor_rfid.h"
#include "LogDebugInfo.h"

// *************************小车底盘RFID****************************
// 外部声明
extern UART_HandleTypeDef huart5;


void vCarRfid_Start_DMA_Receive(uint8_t* ucCarRfid_Rx_Buffer) {
  // 1. 清除接收非空（RXNE）和溢出（ORE）标志
  __HAL_UART_CLEAR_FLAG(&huart5, UART_FLAG_RXNE);
  __HAL_UART_CLEAR_FLAG(&huart5, UART_FLAG_ORE);

  // 2. 读取RDR寄存器（强制清空残留数据，即使无数据也不会阻塞）
  uint8_t temp;
  HAL_UART_Receive(&huart5, &temp, 1, 0); // 超时时间设为0，立即返回

  // 3. 启动DMA接收（此时无残留数据，接收正常）
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart5, ucCarRfid_Rx_Buffer, CAR_RFID_RX_BUF_SIZE)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 第一次启动DMA接收失败后，再次启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, ucCarRfid_Rx_Buffer, CAR_RFID_RX_BUF_SIZE);
  }
  // 关闭半传输+全传输中断
  __HAL_DMA_DISABLE_IT(huart5.hdmarx, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT(huart5.hdmarx, DMA_IT_TC);
}


uint32_t ulCarRfid_Get_DMA_Receive_Len(u32* pulStartIdx)
{
	static u32 ulCurrentLen= 0;
	static u32 ulLastLen = 0 ;
	u32 ulLen = 0 ;
	if (pulStartIdx != NULL) {
		*pulStartIdx = ulLastLen % CAR_RFID_RX_BUF_SIZE;; // 起始位置 = 上一次的累计长度
	}

	// 计算当前DMA累计长度
	ulCurrentLen = CAR_RFID_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart5.hdmarx);

	// 计算单包长度（缓冲区绕回/没有绕回）
	if(ulCurrentLen > ulLastLen) {
		ulLen = ulCurrentLen - ulLastLen ;
	} else {
		ulLen = (CAR_RFID_RX_BUF_SIZE - ulLastLen) + ulCurrentLen;
	}

	ulLastLen = ulCurrentLen ;

	return ulLen;	// 返回单包长度
}

// *************************小车车厢RFID****************************
// 外部声明
extern UART_HandleTypeDef huart8;

void vSendToBoxRfid(uint8_t* ucBoxRfidSendBuffer, uint32_t ucLen)
{
  taskENTER_CRITICAL(); // 进入临界区
  HAL_UART_Transmit_IT(&huart8, ucBoxRfidSendBuffer, ucLen); // 启动中断发送
  taskEXIT_CRITICAL(); // 退出临界区
  DEBUGINFO("ucBoxRfidSendBuffer:");
  vPrint_Array(ucBoxRfidSendBuffer, ucLen);
}

void vBoxRfid_Start_DMA_Receive(uint8_t* ucBoxRfid_Rx_Buffer) {
  // 1. 清除接收非空（RXNE）和溢出（ORE）标志
  __HAL_UART_CLEAR_FLAG(&huart8, UART_FLAG_RXNE);
  __HAL_UART_CLEAR_FLAG(&huart8, UART_FLAG_ORE);

  // 2. 读取RDR寄存器（强制清空残留数据，即使无数据也不会阻塞）
  uint8_t temp;
  HAL_UART_Receive(&huart8, &temp, 1, 0); // 超时时间设为0，立即返回

  // 3. 启动DMA接收（此时无残留数据，接收正常）
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart8, ucBoxRfid_Rx_Buffer, BOX_RFID_RX_BUF_SIZE)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 第一次启动DMA接收失败后，再次启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart8, ucBoxRfid_Rx_Buffer, BOX_RFID_RX_BUF_SIZE);
  }
  __HAL_DMA_DISABLE_IT(huart8.hdmarx, DMA_IT_HT);
}

uint32_t ulBoxRfid_Get_DMA_Receive_Len(void)
{
  uint32_t ulLen = BOX_RFID_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart8.hdmarx);
  return ulLen;
}


/**
 * @brief  当缓冲区绕回时，将两段数据合并为一段
 * @param  srcBufSize: 原始缓冲区大小（CAR_RFID_RX_BUF_SIZE）
 * @param  startIdx:  数据起始下标（package_start_idx）
 * @param  dataLen:   数据长度（ucReciveLen）
 * @param  pTempBuf:  临时缓冲区（用于拼接绕回数据存）
 * @param  tempBufSize: 临时缓冲区大小
 * @retval 指向连续完整数据的指针：
 *         1. 未绕回：返回原缓冲区的起始位置（&pSrcBuf[startIdx]）
 *         2. 绕回：返回临时缓冲区（pTempBuf）
 *         3. 参数错误：返回NULL
 */
u8* ucCarRfid_Rx_Buffer_Wrap_process(u8* pSrcBuf,
											u32 startIdx,
											uint32_t dataLen,
											uint8_t* pTempBuf)
{
    memset(pTempBuf, 0, dataLen);

    //  判断是否绕回
    if (startIdx + dataLen <= CAR_RFID_RX_BUF_SIZE) {
        memcpy(pTempBuf, &pSrcBuf[startIdx], dataLen);
        return pTempBuf;
    } else {
        // 绕回 - 数据分两段，拼接至临时缓冲区
        u32 len1 = CAR_RFID_RX_BUF_SIZE - startIdx;  // 第一段长度（起始位置→缓冲区末尾）
        u32 len2 = dataLen - len1;        		     // 第二段长度（缓冲区开头→剩余数据）

        // 拼接第一段数据（起始位置→缓冲区末尾）
        memcpy(pTempBuf, &pSrcBuf[startIdx], len1);
        // 拼接第二段数据（缓冲区开头→剩余数据，接在第一段后）
        memcpy(&pTempBuf[len1], pSrcBuf, len2);
        return pTempBuf;
    }
}

