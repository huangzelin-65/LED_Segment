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
uint8_t UserPswd_Add_Encry_Passwd(uint8_t byte){
	if(byte == DEFAULT_BACKSPACE){
		if(localEncryCount<=0)
			return 0;
		localEncryCount--;
		localEncryPasswd[localEncryCount] = DEFAULT_PASSWD_CHAR;
	}
	else{
		if(localEncryCount>=MAX_PASSWD_LEN)
			return 0;
		localEncryPasswd[localEncryCount] = byte;
		localEncryCount++;
	}

	return 1;	
}

uint8_t UserPswd_Get_Encry_Status(){
	return isEncryed;
}

void UserPswd_Set_Encry_Status(uint8_t flag){
	if(flag == 0){
		Car_Set_Station_Status(InStation);
	}
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
uint8_t UserPswd_Add_Decry_Passwd(uint8_t byte){
	if(byte == DEFAULT_BACKSPACE){
		if(localDecryCount<=0)
			return 0;
		localDecryCount--;
		localDecryPasswd[localDecryCount] = DEFAULT_PASSWD_CHAR;
	}
	else{
		if(localDecryCount>=MAX_PASSWD_LEN)
			return 0;
		localDecryPasswd[localDecryCount] = byte;
		localDecryCount++;
	}
	
	return 1;	
}

uint8_t UserPswd_Get_Decry_Len(){
	return localDecryCount;	
}
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
