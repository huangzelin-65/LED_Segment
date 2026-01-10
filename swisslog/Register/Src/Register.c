/*
 * Register.C
 *
 *  Created on: Jan 4th, 2026
 *      Author: xusz
 */

#include "Register.h"
#include "adaptor_eeprom.h"
#include "Calculate.h"
#include "string.h"

bool bInitProdRegisterInfo(const char *uuid, uint8_t len)
{
	if(uuid == NULL) return false;

	Stru_ProductLine_Register_Typedef plRegInfo;
	plRegInfo.head = 0xAA55AA55;
	plRegInfo.counter = 0;

	uint32_t crc = crc32_calc((uint32_t)uuid, len);
	plRegInfo.info[0] = (crc&0xFF)+1;
	plRegInfo.info[1] = ((crc&0xFF00)>>8) +1;
	plRegInfo.info[2] = ((crc&0xFF0000)>>16) +1;
	plRegInfo.info[3] = ((crc&0xFF000000)>>24) +1;

	b_Eeprom_Check_Conn();
	if(b_Eeprom_Write_Buf(EEP_ADD_PDTLINE_REGISTER_HEAD, (uint8_t*)&plRegInfo, sizeof(Stru_ProductLine_Register_Typedef)))
		return true;
	else
		return false;
}

bool bReadProdRegisterInfo(const char *uuid, uint8_t len)
{
	if(uuid == NULL) return false;

	uint32_t crc = crc32_calc((uint32_t)uuid, len);
	Stru_ProductLine_Register_Typedef plRegInfo;
	b_Eeprom_Check_Conn();
	if(!b_Eeprom_Read_Buf(EEP_ADD_PDTLINE_REGISTER_HEAD, (uint8_t*)&plRegInfo, sizeof(Stru_ProductLine_Register_Typedef)))
	{
		return false;
	}

	//compare head
	if(plRegInfo.head != 0xAA55AA55) return false;

	//compare crc
	uint32_t readcrc = (plRegInfo.info[3]-1)<<24 | (plRegInfo.info[2]-1)<<16 | (plRegInfo.info[1]-1)<<8 | (plRegInfo.info[0]-1);
	if(readcrc != crc) return false;

	return true;
}

bool bIncFieldRegisterErrTimes(void)
{
	Stru_ProductLine_Register_Typedef plRegInfo;

	//read out
	if(!b_Eeprom_Read_Buf(EEP_ADD_PDTLINE_REGISTER_HEAD, (uint8_t*)&plRegInfo, sizeof(Stru_ProductLine_Register_Typedef)))
	{
		return false;
	}

	//compare head
	if(plRegInfo.head != 0xAA55AA55) return false;

	plRegInfo.counter++;
	//
	if(MOST_FIELD_REG_ERR_TIMES <= plRegInfo.counter )//over MOST_FIELD_REG_ERR_TIMES
	{
		//clear data
		memset(&plRegInfo, 0xFF, sizeof(Stru_ProductLine_Register_Typedef));
	}

	//write back to eeprom
	b_Eeprom_Check_Conn();
	if(b_Eeprom_Write_Buf(EEP_ADD_PDTLINE_REGISTER_HEAD, (uint8_t*)&plRegInfo, sizeof(Stru_ProductLine_Register_Typedef)))
		return true;
	else
		return false;
}

///////////////////////// 现场注册信息操作 /////////////////////////
//读取与写入，内部会计算CRC
bool bWriteFieldRegisterInfo(const Stru_Field_Register_Typedef *info)
{
	Stru_Field_Register_Typedef temp;
	//copy input param to temp
	memcpy(&temp, info, sizeof(Stru_Field_Register_Typedef));

	//calculate crc32
	temp.crc = crc32_calc((uint32_t)info, sizeof(Stru_Field_Register_Typedef)-4);

	//write
	b_Eeprom_Check_Conn();
	if(b_Eeprom_Write_Buf(EEP_ADD_FIELD_REGISTER_USER_NAME_LEN, (uint8_t*)&temp, sizeof(Stru_Field_Register_Typedef)))
		return true;
	else
		return false;
}

bool bReadFieldRegisterInfo(Stru_Field_Register_Typedef *info)
{
	//read out field register info
	b_Eeprom_Check_Conn();
	if(!b_Eeprom_Read_Buf(EEP_ADD_FIELD_REGISTER_USER_NAME_LEN, (uint8_t*)info, sizeof(Stru_Field_Register_Typedef)))
	{
		return false;
	}

	//calculate and compare crc
	uint32_t crc = crc32_calc((uint32_t)info, sizeof(Stru_Field_Register_Typedef)-4);
	if(crc == info->crc)
	{
		return true;
	}
	return false;
}

bool bEraseRegisterArea(void)
{
	Stru_Field_Register_Typedef temp;
	memset(&temp, 0xFF, sizeof(Stru_Field_Register_Typedef));

	//clear eeprom area
	b_Eeprom_Check_Conn();
	if(b_Eeprom_Write_Buf(EEP_ADD_FIELD_REGISTER_USER_NAME_LEN, (uint8_t*)&temp, sizeof(Stru_Field_Register_Typedef)))
		return true;
	else
		return false;
}
