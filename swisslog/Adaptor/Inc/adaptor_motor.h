#ifndef ADAPTOR_INC_ADAPTOR_MOTOR_H_
#define ADAPTOR_INC_ADAPTOR_MOTOR_H_

#include "main.h"

// 缓冲区大小
#define MOTOR_TX_BUF_SIZE   16
#define MOTOR_RX_BUF_SIZE   16

// 电机RS485模式切换宏
#define Motor_RS485_TX_MODE() GPIO_WRITE(MOTOR_RS485_CTRL,GPIO_PIN_SET) // 发送模式 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)  
#define Motor_RS485_RX_MODE() GPIO_WRITE(MOTOR_RS485_CTRL,GPIO_PIN_RESET) // 接收模式 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET) 

/*
// 数据帧结构体（包含数据和长度）
typedef struct {
    uint8_t data[MOTOR_TX_BUF_SIZE];
    uint16_t len;
} Motor_Tx_Frame_t;

typedef struct {
    uint8_t data[MOTOR_RX_BUF_SIZE];
    uint16_t len;
} Motor_Rx_Frame_t;
*/

void vSendToMotor(uint8_t * CmdDataArr,uint8_t len); //发送数据到motor
void vMotor_Start_GPDMA_Receive(void);

#endif /* ADAPTOR_INC_ADAPTOR_MOTOR_H_ */
