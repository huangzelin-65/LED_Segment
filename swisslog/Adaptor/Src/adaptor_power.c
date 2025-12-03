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

//********************************* 车厢5V电源 ******************************************/
void vBox_5V_Power_Init(void)
{
  GPIO_WRITE(BOX_5V,GPIO_PIN_SET);   
}

void vBox_5V_Power_DeInit(void)
{
  GPIO_WRITE(BOX_5V,GPIO_PIN_RESET);        
}

//********************************* Wifi电源 ******************************************/
void vWifi_Power_Init(void)
{
  GPIO_WRITE(Wifi_Power,GPIO_PIN_SET);    
}

void vWifi_Power_DeInit(void)
{
  GPIO_WRITE(Wifi_Power,GPIO_PIN_RESET);  
}

//********************************* 底盘Rfid电源 ******************************************/
void vRfid_Car_Power_Init(void)
{
  GPIO_WRITE(Rfid_Car_Power,GPIO_PIN_SET);
}

void vRfid_Car_Power_DeInit(void)
{
  GPIO_WRITE(Rfid_Car_Power,GPIO_PIN_RESET);
}


//********************************* 电源总控 ******************************************/
// 初始化板上电源初始化
void vPower_Init(void)
{
  
  vBox_5V_Power_Init(); // 车厢5V电源
  vWifi_Power_Init(); // Wifi电源
  vRfid_Car_Power_Init(); // 底盘Rfid电源
  DEBUGINFO("Power Init");
}

// 关闭板上电源
void vPower_DeInit(void)
{
  vBox_5V_Power_DeInit();                   // 车厢5V电源
  vWifi_Power_DeInit();    // Wifi电源
  vRfid_Car_Power_DeInit();// 底盘Rfid电源
  DEBUGINFO("Power DeInit");
}