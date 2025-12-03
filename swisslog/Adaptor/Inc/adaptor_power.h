/*
 * adaptor_power.h
 *
 *  Created on: 2025年11月28日
 *      Author: Jiajie Li
 */

#ifndef ADAPTOR_INC_ADAPTOR_POWER_H_
#define ADAPTOR_INC_ADAPTOR_POWER_H_

void vBox_5V_Power_Init(void);
void vBox_5V_Power_DeInit(void);
void vWifi_Power_Init(void);
void vWifi_Power_DeInit(void);
void vRfid_Car_Power_Init(void);
void vRfid_Car_Power_DeInit(void);

// 初始化板上电源初始化
void vPower_Init(void);
// 关闭板上电源
void vPower_DeInit(void);

#endif /* ADAPTOR_INC_ADAPTOR_POWER_H_ */
