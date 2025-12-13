#ifndef ADAPTOR_INC_ADAPTOR_CAN_H_
#define ADAPTOR_INC_ADAPTOR_CAN_H_


#define CAN_QUEUE_ITEM_SIZE (sizeof(FDCAN_RxHeaderTypeDef) + 8)


/* 全局变量：CAN接收结构体 + FreeRTOS队列 */
typedef struct {
  uint8_t can_id;   // 1=FDCAN1, 2=FDCAN2
  uint32_t std_id;  // 标准ID
  uint8_t data[8];  // 数据域
  uint8_t len;      // 数据长度
} CAN_Recv_Msg_t;

//----------CAN RX FIFO枚举----------
typedef enum
{
    CAN_RX_FIFO0 = 0,
    CAN_RX_FIFO1 = 1,
}eFifoType;

void CAN_Init(FDCAN_HandleTypeDef *hfdcan, 
                        uint16_t Rx_id, 
                        eFifoType xFifo);
                        
void CAN_FilterConfig(FDCAN_HandleTypeDef *hfdcan, 
                                uint16_t Rx_id, 
                                eFifoType xFifo);

void CAN_RxFifoNotify_Activate(FDCAN_HandleTypeDef *hfdcan, 
                                        eFifoType xFifo);

void CAN_Start(FDCAN_HandleTypeDef *hfdcan);


void CAN_AddMsgToTxFifo(FDCAN_HandleTypeDef *hfdcan, 
                                uint16_t CAN_Tx_ID, 
                                uint8_t *pucCAN_Tx_Data);




#endif /* ADAPTOR_INC_ADAPTOR_CAN_H_ */
