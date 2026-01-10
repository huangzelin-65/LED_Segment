#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include "queue.h"
#include "LogDebugInfo.h"
#include "DwinHMI.h"
#include "NumDisplay.h"
#include "adaptor_eeprom.h"
#include "adaptor_box.h"
#include "adaptor_rfid.h"
#include "adaptor_HMI.h"
#include "Task_BoxCtrl.h"
#include "Task_BoxRfid.h"
#include "Calculate.h"
#include "UserPasswd.h"
#include "UV_Clean.h"

uint8_t ucHMI_Rx_Buffer[2][HMI_RX_BUF_SIZE]; // 接收缓冲区
uint8_t ucHMI_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern eDwinPage currentPage;
extern eDwinPage lastPage;

extern secondAct actFlag;
extern uint8_t startFinishedFlag;
extern uint8_t hmiVersion;

extern uint8_t carNum[2];

extern uint8_t localRTCTime[6];//save as dec
extern uint8_t setRTCTime[6];	//save as bcd
extern uint8_t lastCorrectDate[6];//save as dec
extern uint8_t HmiRunTimeBCD[4];//save as bcd
extern uint8_t HmiRunTimeASCII[8];//save as ascii

extern uint8_t lastUvSetTime;
extern uint8_t u8_defaultUvDuration;
extern uint8_t currentVirtualButtonEn;
extern uint8_t hmiEnButton;
// extern uint8_t currentInStationEn;

extern osSemaphoreId_t xHmiRxSemHandle;
extern osMessageQueueId_t xHmi_Send_QueueHandle;
extern osMessageQueueId_t xHmi_Recv_QueueHandle;

// 接收来自HMI的串口数据
void vHmiEventTask(void *argument)
{
	uint32_t ucReciveLen = 0;
	//启动DMA接收
  vHMI_Start_DMA_Receive(ucHMI_Rx_Buffer[ucHMI_current_buf_idx]);

  while(1) {
    // 等待DMA接收完成信号
    if (osSemaphoreAcquire(xHmiRxSemHandle, osWaitForever) == osOK)
    {
			ucReciveLen = ulHMI_Get_DMA_Receive_Len();
			DEBUGINFO("HMI received len:%d ,data:",ucReciveLen);
			vPrint_Array(ucHMI_Rx_Buffer[ucHMI_current_buf_idx], ucReciveLen);

			// 处理接收到的数据
			HMI_Usart_GetDataHandler(ucHMI_Rx_Buffer[ucHMI_current_buf_idx],ucReciveLen);

      // 切换缓冲区并重启接收
      ucHMI_current_buf_idx ^= 1;
      vHMI_Start_DMA_Receive(ucHMI_Rx_Buffer[ucHMI_current_buf_idx]);
		}
	}
}

//send msg, calculate the crc...
void vHmiSendTask(void *argument)
{
	DwinMsgSt *sendMsg = NULL;

	DEBUGINFO("HMI_Send_Deal_Task Start\n");
	
	uint16_t calDat;
	uint8_t sendStream[50];
	memset(sendStream,0,50);
	while(1)
	{
		if(osMessageQueueGet(xHmi_Send_QueueHandle, &sendMsg, NULL, osWaitForever) == osOK)
		{
			//send
			sendStream[0]=0x5A;
			sendStream[1]=0xA5;
			sendStream[2]=sendMsg->length;
			sendStream[3]=sendMsg->cmd;
			memcpy(&sendStream[4],sendMsg->data,sendMsg->length-1);
			calDat = CRC16(&sendStream[3],sendMsg->length);
			sendStream[3+sendMsg->length] = calDat >> 8;
			sendStream[4+sendMsg->length] = calDat;
			vSendToHMI(sendStream,sendMsg->length+5);
			HMI_Free_DwinMsg(sendMsg);
		}
	}
}

//deal the cmd from wifi and uart6(hmi)
void vHmiRecvTask(void *argument)
{	
	DwinMsgSt *recMsg = NULL;
	DEBUGINFO("HMI_Rec_Deal_Task Start\r\n");
	while(1)
	{
		if(osMessageQueueGet(xHmi_Recv_QueueHandle, &recMsg, NULL, osWaitForever) != osOK)
		{
			//正常情况下不会走到这里
      		DEBUGINFO("xQueueReceive failed");
			continue;
		}
		DEBUGINFO("Receive from HMI\r\n");

		switch(recMsg->cmd) 
		{
		case DwinReadReg: 
			DEBUGINFO("DwinReadReg\r\n");
			switch((eDwinRegAdd)recMsg->data[0])
			{
				case addRegPic_Id:
					DEBUGINFO("addRegPic_Id\r\n");
					currentPage = (eDwinPage)recMsg->data[2];
					break;

				case addRegRunTime:
					// HmiRunTimeBCD[0] = BCDToh10(recMsg->data[2]);
					// HmiRunTimeBCD[1] = BCDToh10(recMsg->data[3]);
					// HmiRunTimeBCD[2] = BCDToh10(recMsg->data[4]);
					// HmiRunTimeBCD[3] = BCDToh10(recMsg->data[5]);
					HmiRunTimeBCD[0] = recMsg->data[2];
					HmiRunTimeBCD[1] = recMsg->data[3];
					HmiRunTimeBCD[2] = recMsg->data[4];
					HmiRunTimeBCD[3] = recMsg->data[5];
					
					DEBUGINFO("addRegRunTime %d%d:%d:%d\r\n",HmiRunTimeBCD[0],HmiRunTimeBCD[1],HmiRunTimeBCD[2],HmiRunTimeBCD[3]);
					break;

				case addRegRTC:
					DEBUGINFO("addRegRTC\r\n");
					DEBUGINFO("localRTCTime %d %d %d %d %d %d\r\n",localRTCTime[0],localRTCTime[1],localRTCTime[2],localRTCTime[3],localRTCTime[4],localRTCTime[5]);
					localRTCTime[0] = BCDToh10(recMsg->data[2]);
					localRTCTime[1] = BCDToh10(recMsg->data[3]);
					localRTCTime[2] = BCDToh10(recMsg->data[4]);
					localRTCTime[3] = BCDToh10(recMsg->data[6]);
					localRTCTime[4] = BCDToh10(recMsg->data[7]);
					localRTCTime[5] = BCDToh10(recMsg->data[8]);
					if(actFlag == rtcForUv){
						v_UvClean_Start(lastUvSetTime,localRTCTime);
						actFlag = rtcOnlyRead;
					}else if(actFlag == rtcForSetting){
						HMI_Update_Default_Setting_Page_RtcTime_Req(localRTCTime);
						memcpy(setRTCTime,localRTCTime,6);
						HMI_Change_Page(pgSetting);
						actFlag = rtcOnlyRead;
					}else if(actFlag == rtcForCorrect){
						//save correct time
						//HMI_Update_Default_Setting_Page_RtcTime_Req(localRTCTime);

						HMI_Save_Last_Correct_Date(localRTCTime);
						actFlag = rtcOnlyRead;
					}else if(actFlag == rtcForCheckCorrect){
						//chekc if need to display correct icon
						HMI_Update_CorrectStatus_Req(HMI_Check_Correct_Status(localRTCTime));
					}
					break;
				case addRegVersion:
					DEBUGINFO("addRegVersion");
					hmiVersion = recMsg->data[2];
					startFinishedFlag = 1;
					DEBUGINFO("startFinishedFlag = 1");
					break;

				default:

					break;
			}
		break;
		
		case DwinReadValue:
			DEBUGINFO("DwinReadValue\r\n");
			switch((eDwinValueAdd)((recMsg->data[0]<<8)+ recMsg->data[1]))
			{
				case addButton:
					DEBUGINFO("addButton\r\n");
					//if(Car_Get_Station_Status() ==InStation){
						HMI_Deal_HmiButtonCmd((eDwinButtonDef)((recMsg->data[3]<<8)+ recMsg->data[4]));	
					//}
					break;
				case addCarNum:
					DEBUGINFO("addCarNum\r\n");
				case addSetCarNum:
					DEBUGINFO("addSetCarNum\r\n");
					carNum[0] = recMsg->data[3];
					carNum[1] = recMsg->data[4];
					b_Eeprom_Check_Conn();
					b_Eeprom_Write_Buf(EEP_ADD_CAR_NUMBER,carNum,2);
					NumDisp_SetNumber((uint16_t)(carNum[0]<<8)+carNum[1]);
					break;
				case addPasswdEy:
					DEBUGINFO("addPasswdEy\r\n");
					UserPswd_Add_Encry_Passwd(recMsg->data[4]);
					HMI_Display_Text_EncrtPasswd();
					break;
				case addPasswdDy:
					DEBUGINFO("addPasswdDy\r\n");
					UserPswd_Add_Decry_Passwd(recMsg->data[4]);
					HMI_Display_Text_DecryPasswd();
					break;
				case addPasswdSys:
					DEBUGINFO("addPasswdSys\r\n");
					UserPswd_Add_Sys_Passwd(recMsg->data[4]);
					HMI_Display_Text_SysPasswd();
				case addUvWorkTime:
					DEBUGINFO("addUvWorkTime\r\n");
					lastUvSetTime = recMsg->data[4];
					break;
				case addSetUvDefaultWorkTime:
					DEBUGINFO("addSetUvDefaultWorkTime\r\n");
					u8_defaultUvDuration = recMsg->data[4];
					break;
				case addSetDataYY :
					DEBUGINFO("addSetDataYY\r\n");
					setRTCTime[0]=recMsg->data[4];
					break;
				case addSetDataMM:
					DEBUGINFO("addSetDataMM\r\n");
					setRTCTime[1]=recMsg->data[4];
					break;
				case addSetDataDD:
					DEBUGINFO("addSetDataDD\r\n");
					setRTCTime[2]=recMsg->data[4];
					break;
				case addSetDataHH:
					DEBUGINFO("addSetDataHH\r\n");
					setRTCTime[3]=recMsg->data[4];
					break;
				case addSetDataMIN:
					DEBUGINFO("addSetDataMIN\r\n");
					setRTCTime[4]=recMsg->data[4];
					break;
				case addSetDataSS:
					DEBUGINFO("addSetDataSS\r\n");
					setRTCTime[5]=recMsg->data[4];
					break;
				default:
				
					break;
			}
		
		break;
	
		default:
			break;
		}
		
		HMI_Free_DwinMsg(recMsg);
	}
}

//check if the hmi is ready after power on
void vHmiWaitTask(void *argument)
{

	DwinMsgSt *sendSt;
	startFinishedFlag = 0;
	while(1)
	{
		DEBUGINFO("HMI_Wait_Task running\r\n");
		sendSt = HMI_Malloc_DwinMsg(2);
		sendSt->cmd = DwinReadReg;
		sendSt->length = 3;
		sendSt->data[0]=addRegVersion;
		sendSt->data[1]=1;
		HMI_Send_Msg_To_SendTask(sendSt);
		osDelay(pdMS_TO_TICKS(300));	
		if(startFinishedFlag == 1){
			break;
		}
	}

	//START CHECK
	//get id from eeprom,and send to hmi		
	b_Eeprom_Check_Conn();
	b_Eeprom_Read_Buf(EEP_ADD_CAR_NUMBER,carNum,2);//first read is error??
	b_Eeprom_Read_Buf(EEP_ADD_CAR_NUMBER,carNum,2);
	NumDisp_SetNumber((uint16_t)(carNum[0]<<8)+carNum[1]);

	//GET CURRENT POSTION
	// b_Eeprom_Check_Conn();
	// CarStationStatus stationSt=Car_Get_Station_Status();
	
	UserPswd_Init();

	bEeprom_Read_Byte(EEP_ADD_UVCLEAN_TIME_MINUTES,&u8_defaultUvDuration);
	HMI_Update_DefaultUVTime_Req(u8_defaultUvDuration);//twice when first commu
	lastUvSetTime = u8_defaultUvDuration;
			

	//get setting
	bEeprom_Read_Byte(EEP_ADD_EN_VIRTUAL_BUTTON,&currentVirtualButtonEn);
	HMI_Update_VirtualBtSetting_Req(currentVirtualButtonEn);

	// bEeprom_Read_Byte(EEP_ADD_EN_IN_STATION_SENSOR,&currentInStationEn);
	// HMI_Update_InStationSetting_Req(currentInStationEn);


	//get last correct time
	b_Eeprom_Read_Buf(EEP_ADD_LAST_CORRECT_DATE,lastCorrectDate,6);

	//***************临时设置*****************/
	//设置wifi信号强度，0~4
	HMI_Update_WifiSignalBars_Req(3);
	//设置污车、洁车，0~1
	HMI_Update_DirtyStatus_Req(1);
	//设置当前位置，00000~99999
	HMI_Update_CurLocationId_Req(00000);
	//设置起始站点，000~999
	HMI_Update_SrcStation_Req(111);
	//设置目标站点，000~999
	HMI_Update_DestStation_Req(666);

	
	if( UserPswd_Get_Encry_Status())
	{
	//encryed
		RFID_Scan_Enable(1);
		hmiEnButton = 0;
		HMI_Show_Rf_Page();
	}else{
		RFID_Scan_Enable(0); 
		hmiEnButton = 1;
		HMI_Change_Page(pgHome);
	}

	HMI_Update_CarNum_Req(carNum);

	vBoxELockStatusCheck();
	
	// HMI_Display_Text_Stm32Version();
	HMI_Display_Text_HmiVersion();
	HMI_Get_Rtc();
	actFlag = rtcForCheckCorrect;

	//HMI_Change_Page(pgUvWorking); //test

	osThreadExit();
}
