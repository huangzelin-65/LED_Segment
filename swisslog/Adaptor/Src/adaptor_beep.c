#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adaptor_beep.h"
#include "LogDebugInfo.h"
#include "adaptor_beep.h"

// 蜂鸣器PWM配置参数
#define BUZZER_TIM &htim4  // TIM4
#define BUZZER_CHANNEL TIM_CHANNEL_2  // PB7对应TIM4_CH2
#define BUZZER_FREQ 4000  // 目标频率：4000Hz

extern TIM_HandleTypeDef htim4;
extern uint8_t enable;

void vBeep_Control(uint8_t enable)
{
  if (enable) {
    // 启动PWM输出
    HAL_TIM_PWM_Start(BUZZER_TIM, BUZZER_CHANNEL);
    DEBUGINFO("Beep enable\r\n");
  } else {
    // 停止PWM输出
    HAL_TIM_PWM_Stop(BUZZER_TIM, BUZZER_CHANNEL);
    GPIO_WRITE(BEEP_PWM, GPIO_PIN_RESET); // 将蜂鸣器引脚置为低电平
    DEBUGINFO("Beep disable\r\n");

  }
}
