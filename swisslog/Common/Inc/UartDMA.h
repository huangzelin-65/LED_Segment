#ifndef COMMON_INC_UARTDMA_H_
#define COMMON_INC_UARTDMA_H_

#include "main.h"

// 启动GPDMA接收
void v_Swisslog_Start_DMA_Receive(UART_HandleTypeDef* huart, 
                                    uint8_t* puc_RxBuffer, 
                                    uint32_t u32_rxBuffSize);

// 获取当前DMA接收的数据长度
uint32_t u32_Swisslog_Get_DMA_Receive_Len(UART_HandleTypeDef* huart, 
                                            uint32_t* pulStartIdx, 
                                            uint32_t u32_rxBuffSize);


// 当缓冲区绕回时，将两段数据合并为一段
uint8_t* pu8_Swisslog_Rx_Buffer_Wrap_process(uint8_t* pu8_rxBuff,
                                        uint32_t u32_rxBuffSize,
										uint32_t startIdx,
										uint32_t dataLen,
										uint8_t* pTempBuf);

#endif /* COMMON_INC_UARTDMA_H_ */
