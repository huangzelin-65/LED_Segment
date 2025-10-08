#ifndef PERIPHERALS_INC_DWINHMI_H_
#define PERIPHERALS_INC_DWINHMI_H_


#define HEADONE  0x5A
#define HEADTWO  0xA5

//page define 2bytes
typedef enum {
	pgWelcome = 00,
	pgHome = 01,
	pgEncrypt = 03,
	pgRfCard = 05,		
	pgDecrypt = 06,
	pgUvStart = 8,
	pgUvWorking = 10,
	pgUvFinished = 12,
	pgSetting = 14,
	pgKeyBoard = 16,
	pgErrorPasswd = 18,
	pgWarningDoorIsOpenPd = 20,
	pgWarningUvCleanCanceled = 22,
	pgWarningDoorIsOpenStartUv = 24,
	pgRfCardWithOutPasswd = 26,
	pgConfirmUvCancel = 27,
	pgSysPassWd = 30,
}eDwinPage;

//button define 
typedef enum {
	btToEncryptPage = 0x0001,
	btToUVPage = 0x0002,
	btToSettingPage = 0x0003,
	btToLastPage = 0x0004,		
	btEncryptReq = 0x0005,
	btDecryptReq = 0x0006,
	btStartUv = 0x0007,
	btStopUv = 0x0008,
	btConfirmUv = 0x0009,
	btChangeSetting = 0x000A,
	btBackToHome = 0x000b,	
	btBackToRfPg = 0x000c,	
	btWrongPswdConfirm = 0x000d,	
	btDoorIsOpenPdConfirm = 0x000e,	
	btDoorIsOpenUvConfirm = 0x000f,
//	btUvCancelConfirm = 0x0010,
//	btUvCancelBack = 0x0011,
	btVirtualRfCard	= 0x0012,
	btVirtualUnlock	= 0x0013,
	btVirtualBtChangeSetting	= 0x0014,
	btInStationChangeSetting	= 0x0015,
	btTPCorrectReq	= 0x0016,
	btToSettingPageWithPasswd	= 0x0017,
	btSysPasswdConfirm	= 0x0018,
}eDwinButtonDef;
	

//address define 
typedef enum {
	addButton = 0x0001,
	addCarNum = 0x0002,
	addPasswdEy = 0x0003,
	addPasswdDy = 0x0005,		
	addUvWorkTime = 0x0007,
	addSetUvDefaultWorkTime = 0x0008,
	addSetDataYY = 0x0009,
	addSetDataMM = 0x0000A,
	addSetDataDD = 0x0000B,
	addSetDataHH = 0x000C,
	addSetDataMIN = 0x000D,
	addSetDataSS = 0x000E,
	addLastUvTime = 0x000F,//7*2words
	addPasswdSys = 0x0010,
	addLastUvDuration = 0x0015,
	addLockStatus = 0x0020,
	addCorrectStatus = 0x0022,
	addVirtualBtEnableDisplay = 0x0024,
	addInStationEnableDisplay = 0x0026,
	addSrcStationNum = 0x0029,
	addDestStationNum = 0x002B,
	addWifiSignalBars = 0x002C,
	addDirtyStatus = 0x002D,
	addSetCarNum = 0x002E,
	addCurLocationId = 0x0900,
	addTextRunTime = 0x910,
	addTextEyWord = 0x1000,
	addTextDyWord = 0x1100,
	addTextLastUvRecoed = 0x1200,
	addTextStm32Version = 0x1300,
	addTextHmiVersion = 0x1350,
	addTextSysWord = 0x1400,
}eDwinValueAdd;

//special register define 
typedef enum {
	addRegVersion = 0x00,
	addRegPic_Id = 0x04,
	addRegRTC = 0x20,
	addRegKey = 0x4F,
	addRegCorrect = 0xEA,
}eDwinRegAdd;
	
//special Key Value define 
typedef enum {
	keyCtrlEnBoard = 0xF1,
	keyCtrlCloseBoard = 0xF2,
	keyCtrlConfirmBoard = 0xF3,
}eDwinKeyCtrlValue;
		

//cmd define
typedef enum {
	DwinWriteReg = 0x80,//address by byte
	DwinReadReg,
	DwinWriteValue,		//address by word
	DwinReadValue,
}eDwinCmdType;

//msg strcut
typedef struct {
	uint8_t head[2];
	uint8_t length;//the len after this
	eDwinCmdType cmd;
	uint8_t *data;
	uint8_t crc[2];//calculate by cmd and data,	CRC-16
}DwinMsgSt;


typedef enum {
	rtcOnlyRead = 0,
	rtcForUv = 1,
	rtcForSetting = 2,
	rtcForCorrect = 3,
	rtcForCheckCorrect = 4,
}secondAct;


uint8_t HMI_Free_DwinMsg(DwinMsgSt *ptr);
DwinMsgSt *HMI_Malloc_DwinMsg(uint8_t dataLen);
uint8_t HMI_Send_Msg_To_SendTask(DwinMsgSt * sendMsg);
uint8_t HMI_Is_Button_En(void);

void HMI_Change_Page(eDwinPage page);
void HMI_Show_Rf_Page();
void HMI_Set_RFCardPage(void);
void HMI_Force_Home_Page(void);

void HMI_Get_Rtc();
uint8_t HMI_Get_Instation_Setting(void);

void HMI_Display_Text_Stm32Version();
void HMI_Display_Text_HmiVersion();
void HMI_Display_Text_EncrtPasswd();
void HMI_Display_Text_DecryPasswd();
void HMI_Display_Text_SysPasswd();

void HMI_Update_DefaultUVTime_Req(uint8_t time);
void _HMI_Update_CarNum_Req(uint8_t add, uint8_t carNum[2]);
void HMI_Update_CarNum_Req(uint8_t carNum[2]);
void HMI_Update_LockStatus_Req(uint8_t status);
void HMI_Update_CorrectStatus_Req(uint8_t status);
void HMI_Update_VirtualBtSetting_Req(uint8_t status);
void HMI_Update_InStationSetting_Req(uint8_t status);
void HMI_Update_Default_Setting_Page_RtcTime_Req(uint8_t date[6]);

void HMI_Check_Uv_Clean(uint8_t time);
void HMI_CheckRFCard(uint8_t en);
uint8_t HMI_Check_Correct_Status(uint8_t currentTime[6]);

void HMI_Save_Last_Correct_Date(uint8_t temp[6]);
void HMI_Deal_HmiButtonCmd(eDwinButtonDef button);
void HMI_Usart_GetDataHandler(uint8_t* ucReciveData, uint32_t ulReciveLen);




#endif /* PERIPHERALS_INC_DWINHMI_H_ */
