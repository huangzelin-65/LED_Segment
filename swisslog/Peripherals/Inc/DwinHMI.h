#ifndef PERIPHERALS_INC_DWINHMI_H_
#define PERIPHERALS_INC_DWINHMI_H_


#define HEADONE  0x5A
#define HEADTWO  0xA5

//page define 2bytes
typedef enum {
	pgWelcome = 00,
	pgHome = 01, // home页面
	pgEncrypt = 03, // 加密页面（输入密码页面）
	pgRfCard = 05, // “请扫描RFID卡或点击左侧按钮（已设置密码）”页面
	pgDecrypt = 06, // 解密页面（输入密码页面）
	pgUvStart = 8, // 消毒开始页面
	pgUvWorking = 10, // 消毒中页面
	pgUvFinished = 12, // 消毒完成页面
	pgSetting = 14, // 设置页面
	pgKeyBoard = 16,
	pgErrorPasswd = 18, // 密码错误页面
	pgWarningDoorIsOpenPd = 20,
	pgWarningUvCleanCanceled = 22, // “消毒已取消”页面
	pgWarningDoorIsOpenStartUv = 24, // "请关门后再消毒"页面
	pgRfCardWithOutPasswd = 26, // “请扫描RFID卡或点击左侧按钮”页面
	pgConfirmUvCancel = 27,
	pgSysPassWd = 30, // 系统密码页面
}eDwinPage;

//button define 
typedef enum {
	btToEncryptPage = 0x0001, // 加密
	btToUVPage = 0x0002, // 消毒
	btToSettingPage = 0x0003,
	btToLastPage = 0x0004,		
	btEncryptReq = 0x0005, // 加密页面数字键盘确认键
	btDecryptReq = 0x0006, // 解密页面数字键盘确认键
	btStartUv = 0x0007, // 消毒页面确定键
	btStopUv = 0x0008, // 确定取消消毒键
	btConfirmUv = 0x0009, // 消毒已完成确定键
	btChangeSetting = 0x000A, // 设置页面确认键
	btBackToHome = 0x000b,	// 返回主页面键（设置页面、消毒页面、加密页面都有使用）
	btBackToRfPg = 0x000c,	// 解密页面返回键
	btWrongPswdConfirm = 0x000d,	// “密码错误，请重新输入”页面确定键
	btDoorIsOpenPdConfirm = 0x000e,	// “密码设置前请先关门”页面确定键
	btDoorIsOpenUvConfirm = 0x000f, // “请关门后再消毒”页面确定键
//	btUvCancelConfirm = 0x0010,
//	btUvCancelBack = 0x0011,
	btVirtualRfCard	= 0x0012, // “请扫描RFID卡或点击左侧按钮（已设置密码）”页面解锁键
	btVirtualUnlock	= 0x0013, // 虚拟解锁按键
	btVirtualBtChangeSetting	= 0x0014, // 虚拟按键使能开关
	btInStationChangeSetting	= 0x0015, // 进站检测开关（TK2.1取消掉）
	btTPCorrectReq	= 0x0016, // 请求校准TP按键
	btToSettingPageWithPasswd	= 0x0017, // 带系统密码的设置键
	btSysPasswdConfirm	= 0x0018, // 系统密码确认键
}eDwinButtonDef;
	

//HMI屏幕上的变量地址
typedef enum {
	addButton = 0x0001,
	addCarNum = 0x0002, // 车辆号
	addPasswdEy = 0x0003, // 显示加密页面密码显示区域
	addPasswdDy = 0x0005, // 显示解密页面密码显示区域
	addUvWorkTime = 0x0007, // 设置本次消毒分钟数（剩余消毒分钟数共用此变量）
	addSetUvDefaultWorkTime = 0x0008, // 默认消毒分钟数
	addSetDataYY = 0x0009, // 年
	addSetDataMM = 0x0000A, // 月
	addSetDataDD = 0x0000B, // 日
	addSetDataHH = 0x000C, // 时
	addSetDataMIN = 0x000D, // 分
	addSetDataSS = 0x000E, // 秒
	addLastUvTime = 0x000F,//7*2words
	addPasswdSys = 0x0010, // 显示系统密码页面密码显示区域
	addLastUvDuration = 0x0015, // 上次消毒持续分钟数
	addLockStatus = 0x0020, // 上锁状态图标 (0:解锁图标 1:上锁图标)
	addCorrectStatus = 0x0022,
	addVirtualBtEnableDisplay = 0x0024, // 解锁虚拟按键图标（0:不显示虚拟按键 1:显示虚拟按键）
	addInStationEnableDisplay = 0x0026, // 进站检测开关图标（TK2.1取消掉）
	addSrcStationNum = 0x0029, // 起始站点号（s16）
	addDestStationNum = 0x002B, // 目标站点号 （s16）
	addWifiSignalBars = 0x002C, // wifi信号图标 [0~4 0：0格信号 4：4格(满格)信号]
	addDirtyStatus = 0x002D, // 污/洁
	addSetCarNum = 0x002E, // 显示小车号
	addCurLocationId = 0x0900, // 当前位置标签号（u32）
	addTextRunTime = 0x0910, // 运行时间
	addTextEyWord = 0x1000, // 加密页面密码显示区域
	addTextDyWord = 0x1100, // 解密页面密码显示区域
	addTextLastUvRecoed = 0x1200, // 上次消毒时间
	addTextStm32Version = 0x1300, // 显示Stm32Version(没使用)
	addTextHmiVersion = 0x1350, // 显示HmiVersion
	addTextSysWord = 0x1400, // 输入系统密码页面，1.0无用，2.0点设置时跳转到此页面，密码固定为123456
}eDwinValueAdd;

//special register define 
typedef enum {
	addRegVersion = 0x00,
	addRegPic_Id = 0x04,
	addRegRunTime = 0x0C,
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
// void HMI_Set_RFCardPage(void);
void HMI_Force_Home_Page(void);

void HMI_Get_Rtc();

bool b_HMI_UV_OnOff(FunctionalState x_onOff); // 联动HMI的UV消毒开关

void HMI_Display_Text_Stm32Version();
void HMI_Display_Text_HmiVersion();
void HMI_Display_Text_EncrtPasswd();
void HMI_Display_Text_DecryPasswd();
void HMI_Display_Text_SysPasswd();
void HMI_Display_Text_RunTime(); // 设置屏幕运行时间

void HMI_Update_DefaultUVTime_Req(uint8_t time);
void _HMI_Update_CarNum_Req(uint8_t add, uint8_t carNum[2]);
void HMI_Update_CarNum_Req(uint8_t carNum[2]); // 设置屏幕上的车辆号
void HMI_Update_LockStatus_Req(uint8_t status); // 设置车厢锁状态，0~1
void HMI_Update_CorrectStatus_Req(uint8_t status);
void HMI_Update_VirtualBtSetting_Req(uint8_t status); // 设置虚拟解锁按钮，0~1
// void HMI_Update_InStationSetting_Req(uint8_t status); // 使能进站信号按钮，0~1
void HMI_Update_Default_Setting_Page_RtcTime_Req(uint8_t date[6]);
void HMI_Update_SrcStation_Req(uint16_t StationNum); // 设置起始站点，000~999
void HMI_Update_DestStation_Req(uint16_t StationNum); // 设置目标站点，000~999
void HMI_Update_WifiSignalBars_Req(uint8_t WifiSignalBars); //设置Wifi信号强度
void HMI_Update_DirtyStatus_Req(uint8_t isDirty); // 设置污车、洁车，0~1
void HMI_Update_CurLocationId_Req(uint32_t CurLocationId); // 设置当前位置，00000~99999
void HMI_Update_Rtc(uint8_t date[6]); // 更新屏幕rtc时间

void HMI_Check_Uv_Clean(uint8_t time);
void HMI_CheckRFCard(uint8_t en);
uint8_t HMI_Check_Correct_Status(uint8_t currentTime[6]);

void HMI_Save_Last_Correct_Date(uint8_t temp[6]);
void HMI_Deal_HmiButtonCmd(eDwinButtonDef button);
void HMI_Usart_GetDataHandler(uint8_t* ucReciveData, uint32_t ulReciveLen);




#endif /* PERIPHERALS_INC_DWINHMI_H_ */
