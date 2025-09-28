#ifndef ADAPTOR_INC_ADAPTOR_RFID_H_
#define ADAPTOR_INC_ADAPTOR_RFID_H_

// *************************小车底盘RFID****************************
// 缓冲区大小
#define CAR_RFID_RX_BUF_SIZE   64


// 函数声明
void vCarRfid_Start_DMA_Receive(uint8_t* ucCarRfid_Rx_Buffer);
uint32_t ulCarRfid_Get_DMA_Receive_Len(void);

// *************************小车车厢RFID****************************
#define BOX_RFID_RX_BUF_SIZE 64

void vSendToBoxRfid(uint8_t* ucBoxRfidSendBuffer, uint32_t ucLen);
void vBoxRfid_Start_DMA_Receive(uint8_t* ucBoxRfid_Rx_Buffer);
uint32_t ulBoxRfid_Get_DMA_Receive_Len(void);

#endif /* ADAPTOR_INC_ADAPTOR_RFID_H_ */
