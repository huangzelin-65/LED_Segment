#include "main.h"
#include "Task_rs485.h"
#include "usart.h"
#include "LogDebugInfo.h"

uint8_t Rx_Buffer[Rx_Buf_Size] = {0};
uint16_t Rx_Len = 0 ;
void RS485_Start_DMA_Receive(uint8_t* uRx_Buffer) 		//初始化之后调用这个函数开始DMA接受
{
  // 1. 清除接收非空（RXNE）和溢出（ORE）标志
  __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
  __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_ORE);

  // 2. 读取RDR寄存器（强制清空残留数据，即使无数据也不会阻塞）
  uint8_t temp;
  HAL_UART_Receive(&huart1, &temp, 1, 0); // 超时时间设为0，立即返回

  // 3. 启动DMA接收（此时无残留数据，接收正常）
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uRx_Buffer, Rx_Buf_Size)!=HAL_OK)
  {
    DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA() retry\r\n");
    // 第一次启动DMA接收失败后，再次启动DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uRx_Buffer, Rx_Buf_Size);
  }
  // 关闭半传输+全传输中断
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_TC);
}

void Check_Data_Valid(uint8_t* uPackage_Data,uint8_t uRx_Len)
{

}
void RS485_Init(void)
{
	RS485_Start_DMA_Receive(Rx_Buffer);
}
