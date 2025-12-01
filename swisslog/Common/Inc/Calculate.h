#ifndef COMMON_INC_CALCULATE_H_
#define COMMON_INC_CALCULATE_H_

#include <stdint.h>
#include <stdbool.h>

uint16_t CRC16( uint8_t *arr_buff, uint16_t len);
uint8_t LRC(uint8_t * buff,uint16_t len);
uint8_t Hex2char(uint8_t bHex);
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen);
uint8_t compareArray(uint8_t* arr1,uint8_t* arr2,int len);
uint8_t h10Toh16(uint8_t bHex);
uint8_t BCDToh10(uint8_t bHex);
uint8_t h10ToBCD(uint8_t bDec);
uint32_t substring_to_uint(char* str, u16 start, u16 length);
bool waitforperiod(int *counter,int period);
#endif /* COMMON_INC_CALCULATE_H_ */
