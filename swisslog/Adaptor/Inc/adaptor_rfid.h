#ifndef ADAPTOR_INC_ADAPTOR_RFID_H_
#define ADAPTOR_INC_ADAPTOR_RFID_H_

// 缓冲区大小
#define RFID_RX_BUF_SIZE   64


// 数据帧结构体（包含数据和长度）
typedef struct {
    uint8_t data[RFID_RX_BUF_SIZE];
    uint16_t len;
} Rfid_Rx_Frame_t;


// 函数声明
void vRfid_Start_DMA_Receive(uint8_t* ucRfid_Rx_Buffer);
void vRfid_RxEventCallback(uint16_t dataLength);

#endif /* ADAPTOR_INC_ADAPTOR_RFID_H_ */
