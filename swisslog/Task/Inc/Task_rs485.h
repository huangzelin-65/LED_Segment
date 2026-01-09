#ifndef _TASK_RS485_H
#define _TASK_RS485_H

#define RS485_SET_RECEIVE()  HAL_GPIO_WritePin(RS485_Ctrl_GPIO_Port, RS485_Ctrl_Pin, GPIO_PIN_RESET)
#define RS485_SET_SEND()     HAL_GPIO_WritePin(RS485_Ctrl_GPIO_Port, RS485_Ctrl_Pin, GPIO_PIN_SET)

void RS485_Start_DMA_Receive(uint8_t* uRx_Buffer) ;
#define Rx_Buf_Size 256
#endif
