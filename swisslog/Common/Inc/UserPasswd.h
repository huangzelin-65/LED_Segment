#ifndef COMMON_INC_USERPASSWD_H_
#define COMMON_INC_USERPASSWD_H_

void UserPswd_Init(void);
uint8_t UserPswd_Add_Encry_Passwd(uint8_t byte);
uint8_t UserPswd_Get_Encry_Len(void);
uint8_t UserPswd_Get_Encry_Status(void);
void UserPswd_Set_Encry_Status(uint8_t flag);

void UserPswd_Clear_Encry(void);
uint8_t UserPswd_Save_EncryToFlash(void);
uint8_t UserPswd_Read_EncryFromFlash(void);


/************************************************************/
uint8_t UserPswd_Add_Decry_Passwd(uint8_t byte);
uint8_t UserPswd_Get_Decry_Len(void);
void UserPswd_Clear_Decry(void);

uint8_t UserPswd_Add_Sys_Passwd(uint8_t byte);
uint8_t UserPswd_Get_Sys_Len(void);
void UserPswd_Clear_Sys(void);

/**compare***/
uint8_t UserPswd_Check_Passwd(void);
uint8_t UserPswd_Check_SysPasswd(void);

#endif /* COMMON_INC_USERPASSWD_H_ */
