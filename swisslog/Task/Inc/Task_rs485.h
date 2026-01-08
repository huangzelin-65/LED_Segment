#ifndef _TASK_RS485_H
#define _TASK_RS485_H

#define RS485_SET_RECEIVE()  HAL_GPIO_WritePin(RS485_Ctrl_GPIO_Port, RS485_Ctrl_Pin, GPIO_PIN_RESET)
#define RS485_SET_SEND()     HAL_GPIO_WritePin(RS485_Ctrl_GPIO_Port, RS485_Ctrl_Pin, GPIO_PIN_SET)

/********************* 接收缓冲区配置 *********************/
#define RS485_RX_BUF_LEN   64

// 声明全局变量
extern uint8_t RS485_RxBuf[RS485_RX_BUF_LEN];
extern uint8_t ucRxWritePtr;
extern uint8_t ucRxReadPtr;
extern uint8_t ucRxDataFlag;
extern uint8_t ucRxTempByte;

// 函数声明
void RS485_Interrupt_Receive_Init(void);
void rs485_communicate(void *argument);

#endif
