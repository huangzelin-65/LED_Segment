/*
 * Register.h
 *
 *  Created on: Jan 4th, 2026
 *      Author: xusz
 */

#ifndef INC_REGISTER_BOARD_H_
#define INC_REGISTER_BOARD_H_

#include "stdbool.h"
#include "stdint.h"
#include "stm32h5xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


#define EEP_ADD_PDTLINE_REGISTER_HEAD	      	0x1F00
#define EEP_ADD_FIELD_REGISTER_USER_NAME_LEN	0x1F00

#define MOST_FIELD_REG_ERR_TIMES	4

typedef struct
{
	uint32_t head; //0xAA55AA55
	uint8_t info[4]; //CRC32 each byte +1
	uint8_t counter; //register failed counter
}Stru_ProductLine_Register_Typedef;

typedef struct
{
	uint8_t nameLen;
	uint8_t name[30];
	uint8_t pwdLen;
	uint8_t pwd[30];
	uint8_t snLen;
	uint8_t sn[50];
	uint32_t crc;
}Stru_Field_Register_Typedef;

///////////////////////// 产线注册信息操作 ///////////////////////////
/*
 * bInitProdRegisterInfo, bReadProdRegisterInfo
 * uuid[in], uuid
 * len[in], uuid length
 * return, operation success/fail,
 *         the fail result contains RegisterInfo is in-correction.
 */
bool bInitProdRegisterInfo(const char *uuid, uint8_t len);//产线软件调用
bool bReadProdRegisterInfo(const char *uuid, uint8_t len);//正式软件初始化调用


///////////////////////// 现场注册信息操作 /////////////////////////
//读取与写入，内部会计算CRC
bool bWriteFieldRegisterInfo(const Stru_Field_Register_Typedef *info);//注册成功调用
bool bReadFieldRegisterInfo(Stru_Field_Register_Typedef *info);//正式软件初始化调用

//写入现场注册错误次数
bool bIncFieldRegisterErrTimes(void);//现场注册错误时调用

#endif /* INC_FLASH_H_ */
