#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "semphr.h"
#include "LogDebugInfo.h"
#include "Task_LED_Segment.h"

extern osSemaphoreId_t xSegBlinkSemHandle;

uint8_t num1_display = 0;
uint8_t num2_display = 0;
uint8_t num3_display = 0;


Digit_Seg_Pins_Typedef Digit1_Pins={
  .A_Port=GPIOB,.A_Pin=GPIO_PIN_0,
  .B_Port=GPIOB,.B_Pin=GPIO_PIN_1,
  .C_Port=GPIOB,.C_Pin=GPIO_PIN_2,
  .D_Port=GPIOB,.D_Pin=GPIO_PIN_10,
  .E_Port=GPIOB,.E_Pin=GPIO_PIN_11,
  .F_Port=GPIOA,.F_Pin=GPIO_PIN_6,
  .G_Port=GPIOA,.G_Pin=GPIO_PIN_5
};

Digit_Seg_Pins_Typedef Digit2_Pins={
  .A_Port=GPIOB,.A_Pin=GPIO_PIN_9,
  .B_Port=GPIOC,.B_Pin=GPIO_PIN_13,
  .C_Port=GPIOB,.C_Pin=GPIO_PIN_12,
  .D_Port=GPIOB,.D_Pin=GPIO_PIN_13,
  .E_Port=GPIOB,.E_Pin=GPIO_PIN_14,//
  .F_Port=GPIOB,.F_Pin=GPIO_PIN_7,
  .G_Port=GPIOB,.G_Pin=GPIO_PIN_6
};

Digit_Seg_Pins_Typedef Digit3_Pins={
  .A_Port=GPIOB,.A_Pin=GPIO_PIN_4,
  .B_Port=GPIOB,.B_Pin=GPIO_PIN_5,
  .C_Port=GPIOB,.C_Pin=GPIO_PIN_15,//
  .D_Port=GPIOA,.D_Pin=GPIO_PIN_8,
  .E_Port=GPIOA,.E_Pin=GPIO_PIN_11,
  .F_Port=GPIOA,.F_Pin=GPIO_PIN_15,
  .G_Port=GPIOA,.G_Pin=GPIO_PIN_12
};
const uint8_t SEGMENT_COM_CATHODE[11] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};

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
}

void LED_Segment_Init(void)
{
	for(uint8_t i = 0 ; i<10 ;i++)
	{
	num_display(&Digit1_Pins,i);
	num_display(&Digit2_Pins,i);
	num_display(&Digit3_Pins,i);
	HAL_Delay(500);
	}
	num_display(&Digit1_Pins,0);
	num_display(&Digit2_Pins,0);
	num_display(&Digit3_Pins,0);
}

void LED_Segment_DeInit(void)
{
	num_display(&Digit1_Pins,10);
	num_display(&Digit2_Pins,10);
	num_display(&Digit3_Pins,10);
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
			LED_Segment_DeInit();
			for(uint8_t i = 0 ;i<3; i++)
			{
			LED_Segment_ON(num1_display,num2_display,num3_display);
			vTaskDelay(pdMS_TO_TICKS(500));
			LED_Segment_DeInit();
		    vTaskDelay(pdMS_TO_TICKS(500));
			}
			LED_Segment_ON(num1_display,num2_display,num3_display);
		}
	}
}

//#define Blink_cnt 3
