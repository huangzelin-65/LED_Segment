#ifndef _TASK_PARSE_H
#define _TASK_PARSE_H

#define RS485_RX_BUF_LEN 256
#define LED_SEGMENT_NOT_BLINK 0X00
#define LED_SEGMENT_BLINK 0X01

typedef struct
{
	uint8_t rx_data_bit1;
	uint8_t rx_data_bit2;
	uint8_t rx_data_bit3;
}RS485_Recv_Data_Bit;

#endif
