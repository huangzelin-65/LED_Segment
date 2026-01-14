#include "main.h"

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
	crc=(crc_temp1<<8)+(crc_temp2>>8);
	return  (crc);
}
