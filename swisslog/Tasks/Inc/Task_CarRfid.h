#ifndef INC_TASK_CAR_RFID_H_
#define INC_TASK_CAR_RFID_H_


// 缓冲区大小
#define CAR_RFID_RX_BUF_SIZE   64

#ifdef ZHONGNENG_RFID
// 核心参数宏定义
#define CARD_NUM_LEN 9 // 9位卡号
#define CHAR_OFFSET 6 //卡号开始位置：跳过前面六个字节
#define POS_LEN 5 // 5位位置编号
#define SPEED_OFFSET 5 // 速度偏移量
#define POS_TYPE_OFFSET 7 // 位置类型偏移量
#define Rfid_Rx_Package_LEN  17 		//接收数据包的长度

char* pcGetRfidCardNum_ZHONGNENG(uint8_t *data, u32 RfidDataLen);

#else

#define CARD_NUM_LEN 9     // 9位卡号
#define CHAR_OFFSET 8      //卡号开始位置：跳过"$E000000"（8字节）
#define POS_LEN 5          // 5位位置编号
#define SPEED_OFFSET 5     // 速度偏移量
#define POS_TYPE_OFFSET 7  // 位置类型偏移量

char* pcGetRfidCardNum(char *data, u32 RfidDataLen);

#endif

void vGetCarPosition(char *data);
void vGetTagPosType(char *data);
void vGetTagSpeed(char *data);
void vRfidTask(void *argument);

#endif /* INC_TASK_RFID_H_ */
