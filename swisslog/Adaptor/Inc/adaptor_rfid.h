#ifndef ADAPTOR_INC_ADAPTOR_RFID_H_
#define ADAPTOR_INC_ADAPTOR_RFID_H_

#ifdef ZHONGNENG_RFID
#include"Task_CarRfid.h"
#include "FreeRTOS.h"
#include "semphr.h"
#endif

// *************************小车底盘RFID****************************
// 缓冲区大小
#define CAR_RFID_RX_BUF_SIZE   64
// 下发指令后等待模组回复超时时间（单位：毫秒）
#define RFID_WAIT_REPLY_TIMEOUT   200

// 函数声明
void vCarRfid_Start_DMA_Receive(uint8_t* ucCarRfid_Rx_Buffer);
uint32_t ulCarRfid_Get_DMA_Receive_Len(uint32_t* pulStartIdx);

// *************************小车车厢RFID****************************
#define BOX_RFID_RX_BUF_SIZE 64

void vSendToBoxRfid(uint8_t* ucBoxRfidSendBuffer, uint32_t ucLen);
void vBoxRfid_Start_DMA_Receive(uint8_t* ucBoxRfid_Rx_Buffer);
uint32_t ulBoxRfid_Get_DMA_Receive_Len(void);


#endif /* ADAPTOR_INC_ADAPTOR_RFID_H_ */
