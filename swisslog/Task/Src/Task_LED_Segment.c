#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"
#include "LogDebugInfo.h"
#include "Task_LED_Segment.h"
#include "Task_Parse.h"
#include "Task_rs485.h"

extern osSemaphoreId_t xSegBlinkSemHandle;
extern RS485_Recv_Data_Bit rs485_recv;
extern uint8_t Package_Data[Rx_Buf_Size];
extern uint8_t Display_Flag;
extern uint8_t Blink_Flag;
uint8_t blink_interval_cnt = 2;
uint8_t blink_cnt = 0xFF ;

Digit_Seg_Pins_Typedef Digit1_Pins={
  .A_Port=GPIOB,.A_Pin=GPIO_PIN_0,
  .B_Port=GPIOB,.B_Pin=GPIO_PIN_1,
  .C_Port=GPIOB,.C_Pin=GPIO_PIN_2,
  .D_Port=GPIOB,.D_Pin=GPIO_PIN_10,
  .E_Port=GPIOB,.E_Pin=GPIO_PIN_11,
  .F_Port=GPIOA,.F_Pin=GPIO_PIN_6,
  .G_Port=GPIOA,.G_Pin=GPIO_PIN_5,
  .DP_Port=GPIOA,.DP_Pin=GPIO_PIN_7
};

Digit_Seg_Pins_Typedef Digit2_Pins={
  .A_Port=GPIOB,.A_Pin=GPIO_PIN_9,
  .B_Port=GPIOC,.B_Pin=GPIO_PIN_13,
  .C_Port=GPIOB,.C_Pin=GPIO_PIN_12,
  .D_Port=GPIOB,.D_Pin=GPIO_PIN_13,
  .E_Port=GPIOB,.E_Pin=GPIO_PIN_14,//
  .F_Port=GPIOB,.F_Pin=GPIO_PIN_7,
  .G_Port=GPIOB,.G_Pin=GPIO_PIN_6,
  .DP_Port=GPIOB,.DP_Pin=GPIO_PIN_8
};

Digit_Seg_Pins_Typedef Digit3_Pins={
  .A_Port=GPIOB,.A_Pin=GPIO_PIN_4,
  .B_Port=GPIOB,.B_Pin=GPIO_PIN_5,
  .C_Port=GPIOB,.C_Pin=GPIO_PIN_15,//
  .D_Port=GPIOA,.D_Pin=GPIO_PIN_8,
  .E_Port=GPIOA,.E_Pin=GPIO_PIN_11,
  .F_Port=GPIOA,.F_Pin=GPIO_PIN_15,
  .G_Port=GPIOA,.G_Pin=GPIO_PIN_12,
  .DP_Port=GPIOB,.DP_Pin=GPIO_PIN_3
};
//const uint8_t SEGMENT_COM_CATHODE[11] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
// 规则：下标 0~9  → 无小数点 0~9
// 规则：下标 10~19 → 带小数点 0~9 (对应 0. 1. 2. ... 9.)
// 共阴极数码管
const uint8_t SEGMENT_COM_CATHODE[21] = {
0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,  // 下标0-9  → 0 1 2 3 4 5 6 7 8 9 【小数点关】
0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,   // 下标10-19→ 0.1.2.3.4.5.6.7.8.9.【小数点开】
0x00													//数码管熄灭
};
void num_display(Digit_Seg_Pins_Typedef* digit_pins,uint8_t shownum)
{
	uint8_t seg_code = SEGMENT_COM_CATHODE[shownum];
	HAL_GPIO_WritePin(digit_pins->A_Port, digit_pins->A_Pin, (seg_code & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->B_Port, digit_pins->B_Pin, (seg_code & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->C_Port, digit_pins->C_Pin, (seg_code & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->D_Port, digit_pins->D_Pin, (seg_code & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->E_Port, digit_pins->E_Pin, (seg_code & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->F_Port, digit_pins->F_Pin, (seg_code & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->G_Port, digit_pins->G_Pin, (seg_code & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(digit_pins->DP_Port, digit_pins->DP_Pin, (seg_code & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LED_Segment_Init(void)
{
	for(uint8_t i = 10 ; i<=20 ;i++)
	{
	LED_Segment_ON(i,i,i);
	HAL_Delay(500);
	}
	LED_Segment_ON(0,0,0);
}

void LED_Segment_DeInit(void)
{
	num_display(&Digit1_Pins,20);
	num_display(&Digit2_Pins,20);
	num_display(&Digit3_Pins,20);
}

void LED_Segment_ON(uint8_t num1, uint8_t num2, uint8_t num3)
{
	num_display(&Digit1_Pins,num3);
	num_display(&Digit2_Pins,num2);
	num_display(&Digit3_Pins,num1);
}

void xSegment_Blink(void *argument)
{
	while(1)
	{
		if(xSemaphoreTake(xSegBlinkSemHandle, portMAX_DELAY) == pdTRUE)
		{
			RS485_Recv_Data_Bit* pRs485_Recv =&rs485_recv;
			LED_Segment_DeInit();
			if(Display_Flag ==0)						//如果是设置显示模式下调用，则需要设置闪烁间隔跟闪烁次数
			{
			blink_interval_cnt = Package_Data[3];
			blink_cnt = Package_Data[4];
			}
			if(blink_interval_cnt == 0x00)				//判断是否设置闪烁间隔,未设置时设为2
			{
				blink_interval_cnt = 2;
			}
			if(blink_cnt == 0x00)				//判断是否设置闪烁次数,未设置时无限闪烁
			{
				blink_cnt = 0xFF ;
			}
			if(blink_cnt == 0xFF)	//闪烁无限次
			{
				while(Blink_Flag)
				{
				LED_Segment_ON(pRs485_Recv->rx_data_bit1,pRs485_Recv->rx_data_bit2,pRs485_Recv->rx_data_bit3);
				vTaskDelay(pdMS_TO_TICKS(blink_interval_cnt*BLINK_INTERVAL_UNIT));
				LED_Segment_DeInit();
			    vTaskDelay(pdMS_TO_TICKS(blink_interval_cnt*BLINK_INTERVAL_UNIT));
				}
				LED_Segment_ON(pRs485_Recv->rx_data_bit1,pRs485_Recv->rx_data_bit2,pRs485_Recv->rx_data_bit3);
			}
			else
			{
				for(uint8_t i = 0 ;i<blink_cnt; i++)
				{
				LED_Segment_ON(pRs485_Recv->rx_data_bit1,pRs485_Recv->rx_data_bit2,pRs485_Recv->rx_data_bit3);
				vTaskDelay(pdMS_TO_TICKS(blink_interval_cnt*BLINK_INTERVAL_UNIT));
				LED_Segment_DeInit();
				vTaskDelay(pdMS_TO_TICKS(blink_interval_cnt*BLINK_INTERVAL_UNIT));
				}
				LED_Segment_ON(pRs485_Recv->rx_data_bit1,pRs485_Recv->rx_data_bit2,pRs485_Recv->rx_data_bit3);
				}
		}
	}
}

