#include <string.h>
#include "main.h"
#include "adaptor_eeprom.h"
#include "UserPasswd.h"
#include "Task_BoxCtrl.h"
#include "LogDebugInfo.h"


#define MAX_PASSWD_LEN 6
#define DEFAULT_PASSWD_CHAR			0x00
#define DEFAULT_BACKSPACE			0xF2

static uint8_t localEncryPasswd[MAX_PASSWD_LEN];
static uint8_t localDecryPasswd[MAX_PASSWD_LEN];
static uint8_t localSysPasswd[MAX_PASSWD_LEN];
static uint8_t defSysPasswd[MAX_PASSWD_LEN] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36};

static uint8_t localEncryCount = 0;
static uint8_t localDecryCount = 0;
static uint8_t localSysCount = 0;

static uint8_t isEncryed =0;		//记录当前是否有加密

/************************************************************/
// 向密码缓冲区添加或删除字符
uint8_t UserPswd_Add_Encry_Passwd(uint8_t byte)
{
	// 处理退格键
	if (byte == DEFAULT_BACKSPACE)
	{
		// 如果密码长度为0，不做处理
		if (localEncryCount <= 0)
			return 0;											 // 返回失败
		localEncryCount--;										 // 密码长度减1
		localEncryPasswd[localEncryCount] = DEFAULT_PASSWD_CHAR; // 用默认字符替换被删除的字符
	}
	// 处理普通字符输入
	else
	{
		// 如果密码已达最大长度，不做处理
		if (localEncryCount >= MAX_PASSWD_LEN)
			return 0;							  // 返回失败
		localEncryPasswd[localEncryCount] = byte; // 存储输入的字符
		localEncryCount++;						  // 密码长度加1
	}

	return 1; // 返回成功
}

uint8_t UserPswd_Get_Encry_Status()
{
	return isEncryed;
}

void UserPswd_Set_Encry_Status(uint8_t flag){
	if(isEncryed == flag)
		return;
	isEncryed = flag;	
	bEeprom_Check_Conn();	
	bEeprom_Write_Byte(EEP_ADD_IS_ENCRYED,isEncryed); 
	
}

uint8_t UserPswd_Get_Encry_Len(){
	return localEncryCount;	
}
void UserPswd_Clear_Encry(){
	memset(localEncryPasswd,DEFAULT_PASSWD_CHAR,MAX_PASSWD_LEN);
	localEncryCount = 0;
}

uint8_t UserPswd_Save_EncryToFlash(){
	bEeprom_Check_Conn();
	bEeprom_Write_Buf(EEP_ADD_SEND_PASSWORD,localEncryPasswd,MAX_PASSWD_LEN); 
	return 1;	
}
uint8_t UserPswd_Read_EncryFromFlash(){
	bEeprom_Check_Conn();	
	bEeprom_Read_Buf(EEP_ADD_SEND_PASSWORD,localEncryPasswd,MAX_PASSWD_LEN); 
	localEncryCount = 0;
	int i = 0;
	for(i=0;i<MAX_PASSWD_LEN;i++){
		DEBUGINFO("localEncryPasswd[%d]=%d \r\n", i, localEncryPasswd[i]);
		if(localEncryPasswd[i]!=DEFAULT_PASSWD_CHAR){
			localEncryCount++;
		}else{
			break;
		}
	}
	bEeprom_Check_Conn();	
	bEeprom_Read_Byte(EEP_ADD_IS_ENCRYED,&isEncryed); 
	DEBUGINFO("isEncryed=%d \r\n", isEncryed);
	return 1;	
}

/************************************************************/
// 函数功能：向解密密码缓冲区添加/删除字符
// 参数：byte - 要处理的字符
// 返回值：1-操作成功，0-操作失败
uint8_t UserPswd_Add_Decry_Passwd(uint8_t byte)
{
	// 处理退格键
	if (byte == DEFAULT_BACKSPACE)
	{
		// 如果缓冲区已空，拒绝操作
		if (localDecryCount <= 0)
			return 0;
		// 回退计数器并用默认字符填充当前位置
		localDecryCount--;
		localDecryPasswd[localDecryCount] = DEFAULT_PASSWD_CHAR;
	}
	// 处理普通字符
	else
	{
		// 如果缓冲区已满，拒绝操作
		if (localDecryCount >= MAX_PASSWD_LEN)
			return 0;
		// 存储字符并前进计数器
		localDecryPasswd[localDecryCount] = byte;
		localDecryCount++;
	}

	return 1; // 操作成功
}

uint8_t UserPswd_Get_Decry_Len()
{
	return localDecryCount;
}
// 清空临时解密密码
void UserPswd_Clear_Decry(){
	memset(localDecryPasswd,DEFAULT_PASSWD_CHAR,MAX_PASSWD_LEN);
	localDecryCount = 0;
}



/**compare***/
uint8_t UserPswd_Check_Passwd(){
	if(localDecryCount!=localEncryCount)
		return 0;

	int i =0 ;
	for(i=0;i<MAX_PASSWD_LEN;i++){
		if(localDecryPasswd[i]!=localEncryPasswd[i])
			return 0;
	}
	return 1;
}

uint8_t UserPswd_Add_Sys_Passwd(uint8_t byte){
	if(byte == DEFAULT_BACKSPACE){
		if(localSysCount<=0)
			return 0;
		localSysCount--;
		localSysPasswd[localSysCount] = DEFAULT_PASSWD_CHAR;
	}
	else{
		if(localSysCount>=MAX_PASSWD_LEN)
			return 0;
		localSysPasswd[localSysCount] = byte;
		localSysCount++;
	}
	
	return 1;	
}

uint8_t UserPswd_Get_Sys_Len(){
	return localSysCount;	
}

void UserPswd_Clear_Sys(){
	memset(localSysPasswd,DEFAULT_PASSWD_CHAR,MAX_PASSWD_LEN);
	localSysCount = 0;
}

/**compare***/
uint8_t UserPswd_Check_SysPasswd(){
	int i =0 ;
	for(i=0;i<MAX_PASSWD_LEN;i++){
		if(localSysPasswd[i]!=defSysPasswd[i])
			return 0;
	}
	return 1;
}

void UserPswd_Init(){
	DEBUGINFO("\r\n");
	memset(localEncryPasswd,DEFAULT_PASSWD_CHAR,MAX_PASSWD_LEN);
	memset(localDecryPasswd,DEFAULT_PASSWD_CHAR,MAX_PASSWD_LEN);
	memset(localSysPasswd,DEFAULT_PASSWD_CHAR,MAX_PASSWD_LEN);
	localEncryCount = 0;
	localDecryCount = 0;
	localSysCount = 0;

	//read from eeprom
	UserPswd_Read_EncryFromFlash();
}
