#ifndef _Task_LED_Segment_h
#define _Task_LED_Segment_h

#define BLINK_INTERVAL_UNIT 100  //上班间隔单位/ms
typedef struct
{
    GPIO_TypeDef* A_Port;
    uint16_t A_Pin;
    GPIO_TypeDef* B_Port;
    uint16_t B_Pin;
    GPIO_TypeDef* C_Port;
    uint16_t C_Pin;
    GPIO_TypeDef* D_Port;
    uint16_t D_Pin;
    GPIO_TypeDef* E_Port;
    uint16_t E_Pin;
    GPIO_TypeDef* F_Port;
    uint16_t F_Pin;
    GPIO_TypeDef* G_Port;
    uint16_t G_Pin;
    GPIO_TypeDef* DP_Port;
    uint16_t DP_Pin;			//小数点
} Digit_Seg_Pins_Typedef;

extern Digit_Seg_Pins_Typedef Digit1_Pins ;
extern Digit_Seg_Pins_Typedef Digit2_Pins ;
extern Digit_Seg_Pins_Typedef Digit3_Pins ;
void num_display(Digit_Seg_Pins_Typedef* digit_pins,uint8_t shownum);
void LED_Segment_Init(void);
void LED_Segment_DeInit(void);
void LED_Segment_ON(uint8_t num1, uint8_t num2, uint8_t num3);
void xSegment_Blink(void *argument);
#endif


