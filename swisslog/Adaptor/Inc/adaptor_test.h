#ifndef ADAPTOR_INC_ADAPTOR_TEST_H_
#define ADAPTOR_INC_ADAPTOR_TEST_H_

// 缓冲区大小
#define TEST_RX_BUF_SIZE   10

// 数据帧结构体（包含数据和长度）
typedef struct {
    uint8_t data[TEST_RX_BUF_SIZE];
    uint16_t len;
} Test_Rx_Frame_t;

void vPrint_start_Transmit(uint8_t *rxData, uint16_t Size);
void vTest_Start_DMA_Receive(uint8_t* ucTest_Rx_Buffer);
void vTest_RxEventCallback(uint16_t dataLength);
uint32_t ulTest_Get_DMA_Receive_Len(void);

#endif /* ADAPTOR_INC_ADAPTOR_TEST_H_ */
