#ifndef _ZHONGNENGRFIDREADER_H
#define _ZHONGNENGRFIDREADER_H

#ifdef ZHONGNENG_RFID
#define MAX_INIT_READER_CNT  2			//最大初始化次数

#ifdef ZN_RFID_ENCRYPT
//密码为16个0x36
#define PASSWORD_16BYTE  \
0x36, 0x36, 0x36, 0x36, \
0x36, 0x36, 0x36, 0x36, \
0x36, 0x36, 0x36, 0x36, \
0x36, 0x36, 0x36, 0x36
#else
//无密码
#define PASSWORD_16BYTE  \
		0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00, \
		0x00, 0x00, 0x00, 0x00
#endif /* ZN_RFID_ENCRYPT */

//这个宏是LED与蜂鸣器的开关，0x01表示LED_ON,0x00表示BUZZER_OFF
#define LED_and_BUZZER		0X01,0X00	//0x01表示LED_ON,0x00表示BUZZER_OFF
//#define LED_and_BUZZER	0X00,0X01		//LED_OFF,BUZZER_ON

//这个宏是上报间隔,0xFF,0xFF,0xFF,0xFF表示永不重复上报
#define REPORT_INTERVAL_TIME 0xFF,0xFF,0xFF,0xFF
//#define REPORT_INTERVAL_TIME 0x00,0x00,0x07,0xD0	//上报间隔两秒
// 直接在头文件完整定义枚举（声明+定义一体化，无任何冗余，彻底消除类型冲突）
typedef enum {
    RFID_STATUS_OK = 0,
    RFID_STATUS_READING_WAY_FAIL,    // 读卡器读取方式设置失败
    RFID_STATUS_LED_BUZZER_FAIL,     // LED蜂鸣器设置失败
    RFID_STATUS_REPORT_INTERVAL_FAIL // 上报间隔设置失败
} RFID_StatusTypeDef;

char* RFID_Uart_SendRecvCheck(UART_HandleTypeDef *huart,
                                     u8 *send_buf, uint16_t send_len,
                                     u8* expect_buf,uint16_t expect_len,
                                     u32 timeout);
uint8_t Calc_XOR_CheckCode(uint8_t* tset_command, uint8_t tset_command_Len);
void bin_id_to_num_str(uint8_t *src, uint32_t src_len, char *dst);
RFID_StatusTypeDef Set_Reading_Way(void);
RFID_StatusTypeDef Set_LED_BUZZER(void) ;
RFID_StatusTypeDef Set_Report_Interval(void);
char* vCarRfidInit(void);
void Sem_Init_ZHONGNENG(void);
void CarRfid_Init_With_Retry(void);
#endif

#endif // _ZHONGNENGRFIDREADER_H
