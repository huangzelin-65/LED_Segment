/*
 * adaptor_power.c
 *
 *  Created on: 2025年11月28日
 *      Author: Jiajie Li
 */

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adaptor_power.h"
#include "LogDebugInfo.h"

// 初始化板上电源初始化
void vPower_Init(void)
{
  GPIO_WRITE(BOX_5V,GPIO_PIN_SET);        // 车厢5V电源
  GPIO_WRITE(Wifi_Power,GPIO_PIN_SET);    // Wifi电源
  GPIO_WRITE(Rfid_Car_Power,GPIO_PIN_SET);// 底盘Rfid电源
  DEBUGINFO("Power Init");
}

// 关闭板上电源
void vPower_DeInit(void)
{
  GPIO_WRITE(BOX_5V,GPIO_PIN_RESET);        // 车厢5V电源
  GPIO_WRITE(Wifi_Power,GPIO_PIN_RESET);    // Wifi电源
  GPIO_WRITE(Rfid_Car_Power,GPIO_PIN_RESET);// 底盘Rfid电源
  DEBUGINFO("Power DeInit");
}