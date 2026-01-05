#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include "queue.h"
#include "LogDebugInfo.h"
#include "DwinHMI.h"
#include "adaptor_eeprom.h"
#include "adaptor_box.h"
#include "adaptor_HMI.h"
#include "Task_BoxRfid.h"
#include "Task_BoxCtrl.h"
#include "UserPasswd.h"
#include "UV_Clean.h"
#include "Calculate.h"
#include "SystemTime.h"




#define CORRECT_PERIOD_DAYS	6*30

#define OWN_UNICODE_DIRTY_CODE  (0x0001) //“污”字unicode自编码
#define OWN_UNICODE_CLEAN_CODE  (0x00A1) //“洁”字unicode自编码

extern osMessageQueueId_t xHmi_Send_QueueHandle;
extern osMessageQueueId_t xHmi_Recv_QueueHandle;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;

/***********************************loacal variable***************************************/
static uint8_t revDataLen = 0;
static uint8_t ucRevDataBuf[HMI_RX_BUF_SIZE] = {0};

secondAct actFlag = rtcOnlyRead;//0:nothing

/***********************************golbal variable***************************************/
uint8_t startFinishedFlag = 0;
uint8_t hmiVersion = 0;

eDwinPage currentPage = pgWelcome;
eDwinPage lastPage = pgWelcome;

uint8_t carNum[2] = {0,0};

//static uint8_t passwdBytes = 4;
//static uint8_t encyPasswd[4]={0,0,0,0};
//static uint8_t decryPasswd[4]={0,0,0,0};

uint8_t localRTCTime[6]={0,0,0,0,0,0};//save as dec
uint8_t setRTCTime[6]={0,0,0,0,0,0};	//save as bcd
uint8_t lastCorrectDate[6] = {0,0,0,0,0,0};//save as dec
uint8_t HmiRunTimeBCD[4] = {0,0,0,0};//save as bcd
uint8_t RunTimeHoursASCII[5] = {0};   // 4字节ASCII + 终止符
uint8_t RunTimeMinutesASCII[3] = {0}; // 2字节ASCII + 终止符
uint8_t RunTimeSecondsASCII[3] = {0}; // 2字节ASCII + 终止符

uint8_t lastUvSetTime = 0;
uint8_t defaultUvSetTime = 0;
uint8_t currentVirtualButtonEn = 0; // 使能虚拟按钮(勾选或取消勾选)
uint8_t hmiEnButton = 1;
// uint8_t currentInStationEn = 0; // 使能进站信号按钮(用于提醒PLC小车已进站，TK2.1可去掉)


/***********************************base function***************************************/
uint8_t HMI_Free_DwinMsg(DwinMsgSt *ptr)
{
	if(ptr->data) {
		vPortFree(ptr->data);
		ptr->data = NULL;
	}
	vPortFree(ptr);
	ptr= NULL;
	return 1;
}

DwinMsgSt *HMI_Malloc_DwinMsg(uint8_t dataLen)
{
	DwinMsgSt *dMsg;
	if(dataLen >0) {	
		dMsg = pvPortMalloc(sizeof(DwinMsgSt));
		if(dMsg == NULL) {
			return NULL;
		}

		dMsg->data = pvPortMalloc(dataLen);
		if(dMsg->data == NULL){
			HMI_Free_DwinMsg(dMsg);
			return NULL;
		}			
		memset(dMsg->data,0, dataLen);	
		return dMsg;
	}
	else 
		return NULL;
}

uint8_t HMI_Send_Msg_To_SendTask(DwinMsgSt * sendMsg)
{
	if(osMessageQueuePut(xHmi_Send_QueueHandle, &sendMsg, 0, 0)!= osOK) 
	{
		DEBUGINFO("hmi msg send failed!");
		return 0;
	}
	return 1;
}

static uint8_t HMI_Send_Msg_To_RecTask(uint8_t *buf,uint8_t len)
{
	DwinMsgSt *recMsg = HMI_Malloc_DwinMsg(len-6);
	recMsg->head[0]=buf[0];
	recMsg->head[1]=buf[1];
	recMsg->length=buf[2];
	recMsg->cmd=(eDwinCmdType)buf[3];
	recMsg->crc[0]= buf[len-2];
	recMsg->crc[1]= buf[len-1];
	memcpy(recMsg->data,buf+4,len-6);

	if(recMsg==NULL)
		return 0;

	if(osMessageQueuePut(xHmi_Recv_QueueHandle, &recMsg, 0, 0)!= osOK)
	{
		DEBUGINFO("hmi msg send failed!");
		return 0;
	}

	return 1;
}

uint8_t HMI_Is_Button_En(void)
{
	return hmiEnButton;
}

/*****************************local function***************************************/
// HMI屏幕上解锁按键被按下
void HMI_Unlock_Button_Press()
{
	eBoxCtrlType box_msg;
	DEBUGINFO("box_msg = BoxElockOps");
	box_msg = BoxElockOps;
	if(osMessageQueuePut(xBox_Ctrl_QueueHandle, &box_msg, 0, 0)!= osOK) {
		DEBUGINFO("Uv clean msg send failed!");
	}
}

void HMI_KeyBoard_Ctrl(eDwinKeyCtrlValue key){
	DEBUGINFO("cmd:DwinWriteReg data[0]:addRegKey data[1]:%d\r\n",key);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(2);
	sendSt->cmd = DwinWriteReg;
	sendSt->length = 3;
	sendSt->data[0]=addRegKey;
	sendSt->data[1] = key;
	HMI_Send_Msg_To_SendTask(sendSt);
}

//touch panel correct trigger
void HMI_TPCal_Triger(void){
	DEBUGINFO("cmd:DwinWriteReg data[0]:addRegCorrect data[1]:0x5a\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(2);
	sendSt->cmd = DwinWriteReg;
	sendSt->length = 3;
	sendSt->data[0]=addRegCorrect;
	sendSt->data[1] = 0x5a;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Change_Page(eDwinPage page){
	DEBUGINFO("cmd:DwinWriteReg data[0]:addRegPic_Id data[1]:%d\r\n",page);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(2);

	sendSt->cmd = DwinWriteReg;
	sendSt->length = 3;
	sendSt->data[0]=addRegPic_Id;
	sendSt->data[1] = page;

	HMI_Send_Msg_To_SendTask(sendSt);
	lastPage = currentPage;
	currentPage = page;
}

void HMI_Show_Rf_Page(){
	if(UserPswd_Get_Encry_Status()&&(UserPswd_Get_Encry_Len()>0)){
		//encryed
		HMI_Change_Page(pgRfCard);
	}else{
		HMI_Change_Page(pgRfCardWithOutPasswd);
	}
}

//open for uv clean task
void HMI_Set_RFCardPage(){
	if(currentPage !=pgRfCard && currentPage != pgRfCardWithOutPasswd ){
		//change page to rfCard
		HMI_Show_Rf_Page();
		RFID_Scan_Enable(1);
		hmiEnButton = 0;	
	}
}


/*
 * triggle the in station sensor
 */
void HMI_Force_Home_Page(void){
	DEBUGINFO("origin Page:%d\r\n",currentPage);
	if(currentPage == pgRfCard || currentPage == pgRfCardWithOutPasswd ||currentPage==pgDecrypt){
		//no paswd
		RFID_Scan_Enable(0);
		UserPswd_Clear_Encry();
		UserPswd_Clear_Decry();
		UserPswd_Save_EncryToFlash();
		UserPswd_Set_Encry_Status(0);
		if(UvClean_IsRunning()){
			HMI_Change_Page(pgUvWorking);
		}else{
			HMI_Change_Page(pgHome);
		}
		hmiEnButton = 1;
	}
}


void HMI_Get_Page_Req(){
	DEBUGINFO("cmd:DwinReadReg data[0]:addRegPic_Id data[1]:1\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(2);
	sendSt->cmd = DwinReadReg;
	sendSt->length = 3;
	sendSt->data[0]=addRegPic_Id;
	sendSt->data[1]=1;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Get_Rtc(){
	DEBUGINFO("cmd:DwinReadReg data[0]:addRegRTC data[1]:7\r\n");
// 5A A5 03 81 20 07 CRC  
//7BYTES,yy,mm,dd,ww,hh,mm,ss
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(2);
	sendSt->cmd = DwinReadReg;
	sendSt->length = 3;
	sendSt->data[0]=addRegRTC;
	sendSt->data[1]=7;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Get_Runtime(){
	DEBUGINFO("cmd:DwinReadReg data[0]:addRegRunTime data[1]:4\r\n");
// 5A A5 03 81 0C 04 CRC  
//4BYTES,hh hh,mm,ss
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(2);
	sendSt->cmd = DwinReadReg;
	sendSt->length = 3;
	sendSt->data[0]=addRegRunTime;
	sendSt->data[1]=4;
	HMI_Send_Msg_To_SendTask(sendSt);
}

// // 获取是否勾选使能进站信号按钮
// uint8_t HMI_Get_Instation_Setting(void){
// 	return currentInStationEn;
// }


void HMI_Display_Text_Stm32Version(){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(18);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 19;
	sendSt->data[0]=(addTextStm32Version & 0xff00) >> 8;
	sendSt->data[1]=addTextStm32Version & 0xff;

	sendSt->data[2]=0;
	sendSt->data[3]='V';
	sendSt->data[4]=0;
	sendSt->data[5]='e';
	sendSt->data[6]=0;
	sendSt->data[7]='r';
	sendSt->data[8]=0;
	sendSt->data[9]=0x20;
	sendSt->data[10]=0;
	sendSt->data[11]='1';
	sendSt->data[12]=0;
	sendSt->data[13]='.';
	sendSt->data[14]=0;
	sendSt->data[15]='2';
	sendSt->data[16]=0;
	sendSt->data[17]='7';
	HMI_Send_Msg_To_SendTask(sendSt);
}

/*****************************local function***************************************/
void HMI_Display_Text_HmiVersion(){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(20);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 21;
	sendSt->data[0]=(addTextHmiVersion & 0xff00) >> 8;
	sendSt->data[1]=addTextHmiVersion & 0xff;

	sendSt->data[2]=0;
	sendSt->data[3]='H';
	sendSt->data[4]=0;
	sendSt->data[5]='m';
	sendSt->data[6]=0;
	sendSt->data[7]='i';
	sendSt->data[8]=0;
	sendSt->data[9]=0x20;
	sendSt->data[10]=0;
	sendSt->data[11]='2';
	sendSt->data[12]=0;
	sendSt->data[13]='.';
	sendSt->data[14]=0;
	sendSt->data[15]='1';
	sendSt->data[16]=0;
	sendSt->data[17]='.';
	sendSt->data[18]=0;
	sendSt->data[19]='0';
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Display_Text_EncrtPasswd(){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(14);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 15;
	sendSt->data[0]=(addTextEyWord & 0xff00) >> 8;
	sendSt->data[1]=addTextEyWord & 0xff;
	uint32_t passwdTemp = UserPswd_Get_Encry_Len();

	sendSt->data[2]=0;
	sendSt->data[3]=(passwdTemp>5)?0x2a:0x20;
	sendSt->data[4]=0;
	sendSt->data[5]=(passwdTemp>4)?0x2a:0x20;
	sendSt->data[6]=0;
	sendSt->data[7]=(passwdTemp>3)?0x2a:0x20;
	sendSt->data[8]=0;
	sendSt->data[9]=(passwdTemp>2)?0x2a:0x20;
	sendSt->data[10]=0;
	sendSt->data[11]=(passwdTemp>1)?0x2a:0x20;
	sendSt->data[12]=0;
	sendSt->data[13]=(passwdTemp>0)?0x2a:0x20;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Display_Text_DecryPasswd(){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(14);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 15;
	sendSt->data[0]=(addTextDyWord & 0xff00) >> 8;
	sendSt->data[1]=addTextDyWord & 0xff;
	uint32_t passwdTemp = UserPswd_Get_Decry_Len();

	sendSt->data[2]=0;
	sendSt->data[3]=(passwdTemp>5)?0x2a:0x20;
	sendSt->data[4]=0;
	sendSt->data[5]=(passwdTemp>4)?0x2a:0x20;
	sendSt->data[6]=0;
	sendSt->data[7]=(passwdTemp>3)?0x2a:0x20;
	sendSt->data[8]=0;
	sendSt->data[9]=(passwdTemp>2)?0x2a:0x20;
	sendSt->data[10]=0;
	sendSt->data[11]=(passwdTemp>1)?0x2a:0x20;
	sendSt->data[12]=0;
	sendSt->data[13]=(passwdTemp>0)?0x2a:0x20;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Display_Text_SysPasswd(){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(14);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 15;
	sendSt->data[0]=(addTextSysWord & 0xff00) >> 8;
	sendSt->data[1]=addTextSysWord & 0xff;
	uint32_t passwdTemp = UserPswd_Get_Sys_Len();

	sendSt->data[2]=0;
	sendSt->data[3]=(passwdTemp>5)?0x2a:0x20;
	sendSt->data[4]=0;
	sendSt->data[5]=(passwdTemp>4)?0x2a:0x20;
	sendSt->data[6]=0;
	sendSt->data[7]=(passwdTemp>3)?0x2a:0x20;
	sendSt->data[8]=0;
	sendSt->data[9]=(passwdTemp>2)?0x2a:0x20;
	sendSt->data[10]=0;
	sendSt->data[11]=(passwdTemp>1)?0x2a:0x20;
	sendSt->data[12]=0;
	sendSt->data[13]=(passwdTemp>0)?0x2a:0x20;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Display_Text_LastUvRecord(uint8_t time[7]){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(38);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 39;
	sendSt->data[0]=(addTextLastUvRecoed & 0xff00) >> 8;
	sendSt->data[1]= addTextLastUvRecoed & 0xff;

//20**
	sendSt->data[2]=0;
	sendSt->data[3]=0x32;
	sendSt->data[4]=0;
	sendSt->data[5]=0x30;
	sendSt->data[6]=0;
	sendSt->data[7]=0x30+time[0]/10;
	sendSt->data[8]=0;
	sendSt->data[9]=0x30+time[0]%10;
	sendSt->data[10]=0;
	sendSt->data[11]=0x2d;

//month and day	
	sendSt->data[12]=0;
	sendSt->data[13]=0x30+time[1]/10;
	sendSt->data[14]=0;
	sendSt->data[15]=0x30+time[1]%10;
	sendSt->data[16]=0;
	sendSt->data[17]=0x2d;
	sendSt->data[18]=0;
	sendSt->data[19]=0x30+time[2]/10;
	sendSt->data[20]=0;
	sendSt->data[21]=0x30+time[2]%10;

//hour min sec
	sendSt->data[22]=0;
	sendSt->data[23]=0x30+time[3]/10;
	sendSt->data[24]=0;
	sendSt->data[25]=0x30+time[3]%10;
	sendSt->data[26]=0;
	sendSt->data[27]=0x3a;
	sendSt->data[28]=0;
	sendSt->data[29]=0x30+time[4]/10;
	sendSt->data[30]=0;
	sendSt->data[31]=0x30+time[4]%10;
	sendSt->data[32]=0;
	sendSt->data[33]=0x3a;
	sendSt->data[34]=0;
	sendSt->data[35]=0x30+time[5]/10;
	sendSt->data[36]=0;
	sendSt->data[37]=0x30+time[5]%10;

	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Display_Text_RunTime(){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(38);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 23;
	sendSt->data[0]=(addTextRunTime & 0xff00) >> 8;
	sendSt->data[1]=addTextRunTime & 0xff;

	sendSt->data[2]=0;
	sendSt->data[3]=RunTimeHoursASCII[0];
	sendSt->data[4]=0;
	sendSt->data[5]=RunTimeHoursASCII[1];
	sendSt->data[6]=0;
	sendSt->data[7]=RunTimeHoursASCII[2];
	sendSt->data[8]=0;
	sendSt->data[9]=RunTimeHoursASCII[3];
	sendSt->data[10]=0;
	sendSt->data[11]=0x3A; // ':'
	sendSt->data[12]=0;
	sendSt->data[13]=RunTimeMinutesASCII[0];
	sendSt->data[14]=0;
	sendSt->data[15]=RunTimeMinutesASCII[1];
	sendSt->data[16]=0;
	sendSt->data[17]=0x3A; // ':'
	sendSt->data[18]=0;
	sendSt->data[19]=RunTimeSecondsASCII[0];
	sendSt->data[20]=0;
	sendSt->data[21]=RunTimeSecondsASCII[1];

	HMI_Send_Msg_To_SendTask(sendSt);
}

/*
void HMI_Clear_Encry_Password(void){
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(6);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 7;
	sendSt->data[0]=0;
	sendSt->data[1]=addPasswdEy ;
	memcpy(sendSt->data+2,encyPasswd,4);
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Clear_Decry_Password(void){
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(6);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 7;
	sendSt->data[0]=0;
	sendSt->data[1]=addPasswdDy ;
	memcpy(sendSt->data+2,decryPasswd,4);
	HMI_Send_Msg_To_SendTask(sendSt);
}
*/

void HMI_Update_SrcStation_Req(uint16_t StationNum){
	DEBUGINFO("StationNum = %d\r\n",StationNum);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addSrcStationNum;
	sendSt->data[2] = (StationNum & 0xff00) >> 8;
	sendSt->data[3] = StationNum & 0xff;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_DestStation_Req(uint16_t StationNum){
	DEBUGINFO("StationNum = %d\r\n",StationNum);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addDestStationNum;
	sendSt->data[2] = (StationNum & 0xff00) >> 8;
	sendSt->data[3] = StationNum & 0xff;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_WifiSignalBars_Req(uint8_t WifiSignalBars){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addWifiSignalBars;
	sendSt->data[2] = 0;
	sendSt->data[3] = WifiSignalBars;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_DirtyStatus_Req(uint8_t isDirty){
	DEBUGINFO("\r\n");
	uint16_t gbk_code = isDirty ? OWN_UNICODE_DIRTY_CODE : OWN_UNICODE_CLEAN_CODE;
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length  = 5;
	sendSt->data[0] = 0;
	sendSt->data[1] = addDirtyStatus;
	sendSt->data[2] = (gbk_code & 0xff00) >> 8;
	sendSt->data[3] = gbk_code & 0xff;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_CurLocationId_Req(uint32_t CurLocationId){
	DEBUGINFO("CurLocationId = %d\r\n",CurLocationId);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(6);
	sendSt->cmd = DwinWriteValue;
	sendSt->length  = 7;
	sendSt->data[0] = (addCurLocationId & 0xff00) >> 8;
	sendSt->data[1] = addCurLocationId & 0xff;
	sendSt->data[2] = (CurLocationId & 0xff000000) >> 24;
	sendSt->data[3] = (CurLocationId & 0x00ff0000) >> 16;
	sendSt->data[4] = (CurLocationId & 0x0000ff00) >> 8;
	sendSt->data[5] = CurLocationId & 0xff;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_UVTime_Req(uint8_t time){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addUvWorkTime ;
	sendSt->data[2] = 0;
	sendSt->data[3] = time;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_DefaultUVTime_Req(uint8_t time){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addSetUvDefaultWorkTime;
	sendSt->data[2] = 0;
	sendSt->data[3] = time;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_LastUvRecord_Req(uint8_t time[7]){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(16);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 17;
	sendSt->data[0]=0;
	sendSt->data[1]=addLastUvTime;
	int i = 0;
	for(i=0;i<7;i++){
		sendSt->data[2+i*2] = 0;
		sendSt->data[3+i*2] = time[i];
	}		
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_LastUvDuration_Req(uint8_t LastUvDuration){
	DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length  = 5;
	sendSt->data[0] = 0;
	sendSt->data[1] = addLastUvDuration;
	sendSt->data[2] = 0;
	sendSt->data[3] = LastUvDuration;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void _HMI_Update_CarNum_Req(uint8_t add, uint8_t carNum[2]){
	// DEBUGINFO("\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length  = 5;
	sendSt->data[0] = 0;
	sendSt->data[1] = add;
	sendSt->data[2] = carNum[0];
	sendSt->data[3] = carNum[1];
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_CarNum_Req(uint8_t carNum[2]){
	DEBUGINFO("carNum[0]:%x, carNum[1]%x\r\n",carNum[0],carNum[1]);
	_HMI_Update_CarNum_Req(addCarNum, carNum);
	_HMI_Update_CarNum_Req(addSetCarNum, carNum);
}

void HMI_Update_LockStatus_Req(uint8_t status){
	DEBUGINFO("cmd:DwinWriteValue data[1]=addLockStatus status:%d\r\n",status);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addLockStatus;
	sendSt->data[2] = 0;
	sendSt->data[3] = status;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_CorrectStatus_Req(uint8_t status){
	DEBUGINFO("cmd:DwinWriteValue data[1]=addCorrectStatus status:0\r\n");
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addCorrectStatus;
	sendSt->data[2] = 0;
	sendSt->data[3] = 0;//status;
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_VirtualBtSetting_Req(uint8_t status){
	DEBUGINFO("cmd:DwinWriteValue data[1]=addVirtualBtEnableDisplay status:%d\r\n",status);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 5;
	sendSt->data[0]=0;
	sendSt->data[1]=addVirtualBtEnableDisplay;
	sendSt->data[2] = 0;
	sendSt->data[3] = status>0?1:0;
	HMI_Send_Msg_To_SendTask(sendSt);
}

// //使能进站信号按钮
// void HMI_Update_InStationSetting_Req(uint8_t status){
// 	DEBUGINFO("cmd:DwinWriteValue data[1]=addInStationEnableDisplay status:%d\r\n",status);
// 	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(4);
// 	sendSt->cmd = DwinWriteValue;
// 	sendSt->length = 5;
// 	sendSt->data[0]=0;
// 	sendSt->data[1]=addInStationEnableDisplay;
// 	sendSt->data[2] = 0;
// 	sendSt->data[3] = status>0?1:0;;
// 	HMI_Send_Msg_To_SendTask(sendSt);
// }


//yy为16进制，一般是hmi申请调整自己的时间
void HMI_Update_Rtc(uint8_t date[6]){
	DEBUGINFO("cmd:DwinWriteReg data[1]=addRegRTC date:%d,%d,%d,%d,%d,%d\r\n",date[0],date[1],date[2],date[3],date[4],date[5]);
	// 5A A5 0A 80 1F 51 ,13 11 08 00 18 56 00
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(9);
	sendSt->cmd = DwinWriteReg;
	sendSt->length = 0x0a;
	sendSt->data[0]=addRegRTC-1;
	sendSt->data[1]=0x5A;
	sendSt->data[2]=date[0];
	sendSt->data[3]=date[1];
	sendSt->data[4]=date[2];
	sendSt->data[5]=0;
	sendSt->data[6]=date[3];
	sendSt->data[7]=date[4];
	sendSt->data[8]=date[5];
	HMI_Send_Msg_To_SendTask(sendSt);
}

void HMI_Update_Default_Setting_Page_RtcTime_Req(uint8_t date[6]){
	DEBUGINFO("cmd:DwinWriteValue data[1]=addSetDataYY date:%d,%d,%d,%d,%d,%d\r\n",date[0],date[1],date[2],date[3],date[4],date[5]);
	DwinMsgSt *sendSt = HMI_Malloc_DwinMsg(14);
	sendSt->cmd = DwinWriteValue;
	sendSt->length = 15;
	sendSt->data[0]=0;
	sendSt->data[1]=addSetDataYY;
	int i = 0;
	for(i=0;i<6;i++){
		sendSt->data[2+i*2] = 0;
		sendSt->data[3+i*2] = date[i];
	}		
	HMI_Send_Msg_To_SendTask(sendSt);
}


uint8_t HMI_Check_Correct_Status(uint8_t currentTime[6]){
	DEBUGINFO("\r\n");
	if(currentTime[0]<lastCorrectDate[0]){
		return 1;
	}

	int16_t temp = (currentTime[0]-lastCorrectDate[0])*360+(currentTime[1]-
		lastCorrectDate[1])*30+currentTime[2]-lastCorrectDate[2];
	if((temp>CORRECT_PERIOD_DAYS) || (temp<0)){
		return 1;
	}
	return 0;
}


//open for uv clean task
void HMI_Check_Uv_Clean(uint8_t time){
	DEBUGINFO("HMI_Check_Uv_Clean time:%d\r\n",time);
	//uint8_t temp[7];
	HMI_Update_UVTime_Req(time);
	
	if(time == 0&&(currentPage != pgRfCard && currentPage != pgRfCardWithOutPasswd)){
		HMI_Change_Page(pgUvFinished);
	}
}

//open for rfid clean task
void HMI_CheckRFCard(uint8_t en){
	DEBUGINFO("HMI_CheckRFCard en:%d\r\n",en);
	if(en>0	){
		// 密码状态检查（未加密/空密码/管理员权限）
		if(UserPswd_Get_Encry_Status() == 0||(UserPswd_Get_Encry_Len() == 0 )|| en == 0x35){	
				
			if(currentPage == pgRfCard || currentPage == pgRfCardWithOutPasswd ||currentPage==pgDecrypt){
				//no paswd
				RFID_Scan_Enable(0);
				UserPswd_Clear_Encry();
				UserPswd_Clear_Decry();
				UserPswd_Save_EncryToFlash();
				UserPswd_Set_Encry_Status(0);
				if(UvClean_IsRunning()){
					HMI_Change_Page(pgUvWorking);
				}else{
					HMI_Change_Page(pgHome);
				}
				hmiEnButton = 1;
				HMI_Unlock_Button_Press();	
			}else{
				if(!UvClean_IsRunning()){
					HMI_Unlock_Button_Press();
				}
			}

			
		}
		// 密码状态检查（已加密 && 非空密码 && 非管理员权限）
		else{
			if(currentPage == pgRfCard || currentPage == pgRfCardWithOutPasswd){
				UserPswd_Clear_Decry(); 
				HMI_Display_Text_DecryPasswd();
				HMI_Change_Page(pgDecrypt);
			}
		}	
	}
	// en=0 时的处理
	else {
		if(currentPage == pgDecrypt){
			HMI_KeyBoard_Ctrl(keyCtrlCloseBoard);
			HMI_Show_Rf_Page();
		}
	}
}



//touch panel correct trigger
void HMI_Save_Last_Correct_Date(uint8_t temp[6]){
	DEBUGINFO("HMI_Save_Last_Correct_Date\r\n");
	memcpy(lastCorrectDate,temp,6);
	bEeprom_Check_Conn();
	bEeprom_Write_Buf(EEP_ADD_LAST_CORRECT_DATE,temp,6);
}

//deal  the button presss from hmi
void HMI_Deal_HmiButtonCmd(eDwinButtonDef button)
{
	uint8_t temp[7];
	//DwinMsgSt *sendSt = NULL;

	switch(button){
		case btToEncryptPage:
			DEBUGINFO("btToEncryptPage\r\n");
			//clear old password
			UserPswd_Clear_Encry();
			UserPswd_Clear_Decry();	
			HMI_Display_Text_EncrtPasswd();
			HMI_Display_Text_DecryPasswd();
			HMI_Change_Page(pgEncrypt);	
			break;
		
		case btToUVPage:
			DEBUGINFO("btToUVPage\r\n");
			lastUvSetTime = defaultUvSetTime;
			HMI_Update_UVTime_Req(defaultUvSetTime);
			UvClean_Get_Record(temp);
			HMI_Update_LastUvRecord_Req(temp);
			HMI_Update_LastUvDuration_Req(temp[6]);
			HMI_Display_Text_LastUvRecord(temp);
			HMI_Change_Page(pgUvStart);
			break;
		
		case btToSettingPage:
		DEBUGINFO("btToSettingPage\r\n");
			vSystem_Get_RunTime_Ascii(RunTimeHoursASCII,RunTimeMinutesASCII,RunTimeSecondsASCII);
			HMI_Display_Text_RunTime();
			//osDelay(pdMS_TO_TICKS(50));
			HMI_Get_Rtc();
			actFlag = rtcForSetting;
			break;

		case btToSettingPageWithPasswd:
			DEBUGINFO("btToSettingPageWithPasswd\r\n");
			UserPswd_Clear_Sys();
			HMI_Display_Text_DecryPasswd();
			HMI_Change_Page(pgSysPassWd);
			break;

		case btSysPasswdConfirm:
			DEBUGINFO("btSysPasswdConfirm\r\n");
			if(UserPswd_Check_SysPasswd() > 0)
			{
				vSystem_Get_RunTime_Ascii(RunTimeHoursASCII,RunTimeMinutesASCII,RunTimeSecondsASCII);
				HMI_Display_Text_RunTime();
				//osDelay(pdMS_TO_TICKS(50));
				HMI_Get_Rtc();
				actFlag = rtcForSetting;
			}
			else
			{
				HMI_Change_Page(pgErrorPasswd);
			}
			break;
		//case btToLastPage:
		//	break;	
		
		case btEncryptReq:
			DEBUGINFO("btEncryptReq\r\n");
			if((!ELOCK1_LEVEL)&&(!ELOCK2_LEVEL)){
				HMI_Change_Page(pgWarningDoorIsOpenPd);
			}else{
				UserPswd_Save_EncryToFlash();
				UserPswd_Set_Encry_Status(1);
				HMI_Show_Rf_Page();
				RFID_Scan_Enable(1);
				hmiEnButton = 0;
			}			
			break;
			
		case btDecryptReq:
			DEBUGINFO("btDecryptReq\r\n");
			//check the password is correct?
			if(UserPswd_Check_Passwd()>0){			
				RFID_Scan_Enable(0);
				UserPswd_Clear_Encry();
				UserPswd_Clear_Decry();
				UserPswd_Save_EncryToFlash();
				UserPswd_Set_Encry_Status(0);
				if(UvClean_IsRunning()){
					HMI_Change_Page(pgUvWorking);
				}else{
					HMI_Change_Page(pgHome);			
				}
				hmiEnButton = 1;
				HMI_Unlock_Button_Press();
			}
			else{
				HMI_Change_Page(pgErrorPasswd);
			}
			break;
			
		case btStartUv:
			DEBUGINFO("btStartUv\r\n");
			if(lastUvSetTime == 0){
				break;
			}
			//check if the door is closed?
			if((!ELOCK1_LEVEL)&&(!ELOCK2_LEVEL)){
				HMI_Change_Page(pgWarningDoorIsOpenStartUv);
			}else{
				HMI_Change_Page(pgUvWorking);
				HMI_Get_Rtc();
				actFlag = rtcForUv;
			}		
			break;
			
		case btStopUv:
			DEBUGINFO("btStopUv\r\n");
			//stop uv clean
			UvClean_Stop();
			UvClean_Save_Record();
			HMI_Change_Page(pgWarningUvCleanCanceled);
			break;
			
		case btConfirmUv:
			DEBUGINFO("btConfirmUv\r\n");
			HMI_Change_Page(pgHome);
			break;
		
		case btChangeSetting:
			DEBUGINFO("btChangeSetting\r\n");
			//save the setting to eeprom and update local
			bEeprom_Check_Conn();
			bEeprom_Write_Byte(EEP_ADD_EN_VIRTUAL_BUTTON,currentVirtualButtonEn);
			HMI_Update_VirtualBtSetting_Req(currentVirtualButtonEn);
				
			// bEeprom_Write_Byte(EEP_ADD_EN_IN_STATION_SENSOR,currentInStationEn);
			// HMI_Update_InStationSetting_Req(currentInStationEn);
				
			bEeprom_Write_Byte(EEP_ADD_UVCLEAN_TIME_MINUTES,defaultUvSetTime);
			HMI_Update_DefaultUVTime_Req(defaultUvSetTime);

			setRTCTime[0] = h10ToBCD(setRTCTime[0]);
			setRTCTime[1] = h10ToBCD(setRTCTime[1]);
			setRTCTime[2] = h10ToBCD(setRTCTime[2]);
			setRTCTime[3] = h10ToBCD(setRTCTime[3]);
			setRTCTime[4] = h10ToBCD(setRTCTime[4]);
			setRTCTime[5] = h10ToBCD(setRTCTime[5]);
			HMI_Update_Rtc(setRTCTime);
			HMI_Update_CarNum_Req(carNum);
			HMI_Change_Page(pgHome);
			break;
			
		case btBackToHome:
			DEBUGINFO("btBackToHome\r\n");
			if(currentPage == pgSetting){
				bEeprom_Read_Byte(EEP_ADD_EN_VIRTUAL_BUTTON,&currentVirtualButtonEn);
				HMI_Update_VirtualBtSetting_Req(currentVirtualButtonEn);
				
				// bEeprom_Read_Byte(EEP_ADD_EN_IN_STATION_SENSOR,&currentInStationEn);
				// HMI_Update_InStationSetting_Req(currentInStationEn);

				bEeprom_Read_Byte(EEP_ADD_UVCLEAN_TIME_MINUTES,&defaultUvSetTime);
				HMI_Update_DefaultUVTime_Req(defaultUvSetTime);//twice when first commu
			}
			HMI_Update_CarNum_Req(carNum);
			HMI_Change_Page(pgHome);
			HMI_Get_Rtc();
			actFlag = rtcForCheckCorrect;
			break;
		
		case btBackToRfPg:
			DEBUGINFO("btBackToRfPg\r\n");
			HMI_Show_Rf_Page();
			break;		
		case btWrongPswdConfirm:
			DEBUGINFO("btWrongPswdConfirm\r\n");
			HMI_Change_Page(lastPage);
			break;	
		
		case btDoorIsOpenPdConfirm:
			DEBUGINFO("btDoorIsOpenPdConfirm\r\n");
			HMI_Change_Page(pgEncrypt);
			break;
		
		case btDoorIsOpenUvConfirm:
			DEBUGINFO("btDoorIsOpenUvConfirm\r\n");
			HMI_Change_Page(pgUvStart);
			break;
		
		//the same function like rfcard scan(user level)
		case btVirtualRfCard:
			DEBUGINFO("btVirtualRfCard\r\n");
			if(currentVirtualButtonEn >0){
				HMI_CheckRFCard(1);
			}
			break;
		case btVirtualUnlock:
			DEBUGINFO("btVirtualUnlock\r\n");
			if(currentVirtualButtonEn >0){
				HMI_Unlock_Button_Press();
			}
			break;
		case btVirtualBtChangeSetting:
			//使能虚拟按钮(勾选或取消勾选)
			DEBUGINFO("btVirtualBtChangeSetting\r\n");
			if(currentVirtualButtonEn >0){
				currentVirtualButtonEn = 0;
			}else{
				currentVirtualButtonEn = 1;
			}
			HMI_Update_VirtualBtSetting_Req(currentVirtualButtonEn);
			break;	
		// case btInStationChangeSetting:
		// 	//使能进站信号按钮(勾选或取消勾选)
		// 	DEBUGINFO("btInStationChangeSetting\r\n");
		// 	if(currentInStationEn >0){
		// 		currentInStationEn = 0;
		// 		DEBUGINFO("currentInStationEn = 0\r\n");
		// 	}else{
		// 		currentInStationEn = 1;
		// 		DEBUGINFO("currentInStationEn = 1\r\n");
		// 	}
		// 	HMI_Update_InStationSetting_Req(currentInStationEn);
		// 	break;

		case btTPCorrectReq:
			DEBUGINFO("btTPCorrectReq\r\n");
			HMI_TPCal_Triger();
			HMI_Get_Rtc();
			actFlag = rtcForCorrect;
			break;
		default:
			break;

		}

}

static void HMI_Deal_Uart_Data(uint8_t ucData)
{
	uint8_t dataLen;
	uint16_t CalCRC16 = 0;
	uint16_t RevCRC16 = 0;
	ucRevDataBuf[revDataLen++] = ucData;
	if((revDataLen == 1)&&(ucRevDataBuf[0] != HEADONE)){  
		revDataLen = 0;
	}
	else if	((revDataLen == 2)&&(ucRevDataBuf[1] != HEADTWO)){
		revDataLen = 0;	  
	}
	else if(revDataLen > 3) {
		dataLen	= ucRevDataBuf[2];
		if((dataLen+5) <= revDataLen) {
			RevCRC16 = (ucRevDataBuf[dataLen+3] << 8)+ ucRevDataBuf[dataLen+4];
			CalCRC16 = CRC16(ucRevDataBuf+3,dataLen);
			//DEBUGINFO("HMI Get:");
			for(int i=0; i<revDataLen; i++) {
				//DEBUGINFO("%2X ",ucRevDataBuf[i]);
			}
			//DEBUGINFO("\n");

			if(RevCRC16 == CalCRC16) {
				//DEBUGINFO("reciver screen dat crc ok\n");
				//send data to hmi task
				HMI_Send_Msg_To_RecTask(ucRevDataBuf,revDataLen);
			}
			else if(ucRevDataBuf[dataLen+3]==0x2A && ucRevDataBuf[dataLen+4]==0x23) {
				//send data to hmi task
				HMI_Send_Msg_To_RecTask(ucRevDataBuf,revDataLen);
			}

			revDataLen = 0;
		}
	}
}

void HMI_Usart_GetDataHandler(uint8_t* ucReciveData, uint32_t ulReciveLen)
{
	for(int i=0; i<ulReciveLen; i++)
	{
		HMI_Deal_Uart_Data(ucReciveData[i]);
	}
}


