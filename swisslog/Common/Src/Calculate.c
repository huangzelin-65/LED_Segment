#include <Common.h>
#include "main.h"
#include "Calculate.h"
#include <string.h>

/**************计算CRC效验*************************/

uint16_t CRC16( uint8_t *arr_buff, uint16_t len)  
{
	uint16_t  crc=0xFFFF,crc_temp1,crc_temp2;
	uint16_t i, j;
	for ( j=0;j<len;j++)
	{
		crc=crc^*(arr_buff+j);
		for ( i=0; i<8; i++)
		{
			if ((crc&0x0001)>0)
			{
				crc>>=1;
				crc^=0xa001;
			}
			else
				crc>>=1;			
		}
	}
	crc_temp1=crc;
	crc_temp2=crc;
	crc=(crc_temp1<<8)+(crc_temp2>>8);                  //
	return  (crc);
}

extern CRC_HandleTypeDef hcrc; // App侧独立CRC句柄
// CRC32计算（按字节长度，自动对齐4字节）
uint32_t crc32_calc(uint32_t start_addr, uint32_t data_len) {
    if (data_len == 0) {
        return 0;
    }

    // 计算4字节对齐后的长度
    //uint32_t word_len = (data_len + 3) / 4;
    uint32_t crc_val = HAL_CRC_Calculate(&hcrc, (uint32_t*)start_addr, data_len);
    return crc_val;
}

/***********计算CRL效验***************************/

uint8_t LRC(uint8_t * buff,uint16_t len)
{
    uint8_t uchLRC = 0;
    uint16_t i;
    for(i = 0; i < len;i ++)
    {
        uchLRC += *buff;
        buff++;
    }
  return ((unsigned char )(-((char) (uchLRC))));
}

/***********把ASCII字符转换为16进制数***************/

uint8_t char2hex(uint8_t bHex)
{
    if((bHex <= 9))
    {
        bHex += 0x30;
    }
    else  if((bHex >= 10) &&(bHex <= 15))
    {
        bHex += 0x37;
    }
    else bHex = 0xFF;
    return bHex;
}

/*****************把16进制数转ASCII字符******************/
uint8_t Hex2char(uint8_t bHex)
{
    if((bHex >= 0x30)&&(bHex <= 0x39))
    {
        bHex -= 0x30;
    }
    else if((bHex >= 0x41)&&(bHex <= 0x46))
    {
        bHex -= 0x37;
    }
    else bHex = 0xFF;
    return bHex ;
}

void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen)
{
	char    ddl,ddh;
	int i;

	for (i=0; i<nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i*2] = ddh;
		pbDest[i*2+1] = ddl;
	}

	//pbDest[nLen*2] = '\0';
}
//比较两个数组是否相等,不相等返回1，相等返回0
uint8_t compareArray(uint8_t* arr1,uint8_t* arr2,int len)
{
	for(int i=0;i<len;i++)
	{
		if(arr1[i]!=arr2[i])
			return 1;
	}
	return 0;
}
/***********
把10进制转16进制

19（0x13）转0x19
for dwin rtc

***************/

uint8_t h10ToBCD(uint8_t bDec)
{
   uint8_t temp = (bDec/10)*6; 
   return bDec+temp;
}
uint8_t BCDToh10(uint8_t bHex)
{
   uint8_t temp = (bHex/16)*6; 
   return bHex-temp;
}



/**
 * 将字符串中指定开始位置和长度的子串转换为无符号整数
 */
uint32_t substring_to_uint(char* str, uint16_t start, uint16_t length) 
{
  uint32_t value = 0;
  uint32_t digit = 0;
  // 检查参数合法性
  if (str == NULL) {
      return CONVERT_NULL_PTR;
  }
  
  size_t str_len = strlen(str);
  if (start >= str_len) {
      return CONVERT_INVALID_START;
  }
  
  if (length == 0 || start + length > str_len) {
      return CONVERT_INVALID_LENGTH;
  }
  
  // 检查所有字符是否都是数字
  for (size_t i = 0; i < length; i++) {
      if (str[start + i] < '0' || str[start + i] > '9') {
          return CONVERT_NON_DIGIT;
      }
  }
  
  // 转换为无符号整数
  for (size_t i = 0; i < length; i++) {
      // 检查溢出
      if (value > UINT32_MAX / 10) {
          return CONVERT_OVERFLOW;
      }
      value *= 10;
      
      digit = str[start + i] - '0';
      if (value > UINT32_MAX - digit) {
          return CONVERT_OVERFLOW;
      }
      value += digit;
  }
  
  return value;
}

bool waitforperiod(int *counter,int period)
{
    *counter = *counter + 1;
    if(*counter >= period)
    {
        *counter = 0;
        return true;
    }
    return false;
}

