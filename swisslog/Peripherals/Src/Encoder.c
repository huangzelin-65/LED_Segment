/*
 * Encoder.c
 *
 *  Created on: 2025年11月28日
 *      Author: e3lijia25d
 */
#include "main.h"
#include "Encoder.h"
#include "LogDebugInfo.h"

uint16_t usEncoder_Read_Number()
{
    uint8_t ucHundred = 0;
    uint8_t ucTen = 0;
    uint8_t ucUnit = 0;
    uint16_t ucTotal = 0;

    uint8_t ucHundred_1 = GPIO_READ(Encoder_S1_1);
    uint8_t ucHundred_2 = GPIO_READ(Encoder_S1_2);
    DEBUGINFO("ucHundred_1 = %d, ucHundred_2 = %d",ucHundred_1,ucHundred_2);

    uint8_t ucTen_1 = GPIO_READ(Encoder_S2_1);
    uint8_t ucTen_2 = GPIO_READ(Encoder_S2_2);
    uint8_t ucTen_4 = GPIO_READ(Encoder_S2_4);
    uint8_t ucTen_8 = GPIO_READ(Encoder_S2_8);
    DEBUGINFO("ucTen_1 = %d, ucTen_2 = %d, ucTen_4 = %d, ucTen_8 = %d",ucTen_1,ucTen_2,ucTen_4,ucTen_8);

    uint8_t ucUnit_1 = GPIO_READ(Encoder_S3_1);
    uint8_t ucUnit_2 = GPIO_READ(Encoder_S3_2);
    uint8_t ucUnit_4 = GPIO_READ(Encoder_S3_4);
    uint8_t ucUnit_8 = GPIO_READ(Encoder_S3_8);
    DEBUGINFO("ucUnit_1 = %d, ucUnit_2 = %d, ucUnit_4 = %d, ucUnit_8 = %d",ucUnit_1,ucUnit_2,ucUnit_4,ucUnit_8);


    // 计算百位
    if(ucHundred_1 == GPIO_PIN_RESET) ucHundred += 1;
    if(ucHundred_2 == GPIO_PIN_RESET) ucHundred += 2;

    // 计算十位
    if(ucTen_1 == GPIO_PIN_RESET) ucTen += 1;
    if(ucTen_2 == GPIO_PIN_RESET) ucTen += 2;
    if(ucTen_4 == GPIO_PIN_RESET) ucTen += 4;
    if(ucTen_8 == GPIO_PIN_RESET) ucTen += 8;
    if(ucTen > 9) ucTen = 0;

    // 计算个位
    if(ucUnit_1 == GPIO_PIN_RESET) ucUnit += 1;
    if(ucUnit_2 == GPIO_PIN_RESET) ucUnit += 2;
    if(ucUnit_4 == GPIO_PIN_RESET) ucUnit += 4;
    if(ucUnit_8 == GPIO_PIN_RESET) ucUnit += 8;
    if(ucUnit > 9) ucUnit = 0;

    // 计算总数值
    ucTotal = ucHundred * 100 + ucTen * 10 + ucUnit;

    return ucTotal;
}