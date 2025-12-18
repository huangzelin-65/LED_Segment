#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "NumDisplay.h"
#include "adaptor_box.h"
#include "Task_BoxCtrl.h"
#include "Task_BoxRfid.h"
#include "adaptor_eeprom.h"
#include "DwinHMI.h"
#include "UV_Clean.h"

//闪灯使用
#define FENZI 12
#define FENMU	6

//小车在站状态
static CarStationStatus mCarStationStatus=InStation;

extern uint8_t startFinishedFlag;
extern CarStatus_t CarStatus;
extern ServerToCarData_t ServerToCarData;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;


/**
外部获取进站状态
*/
CarStationStatus Car_Get_Station_Status(void){
	return mCarStationStatus;
}

/**
保存进站状态
*/
void Car_Set_Station_Status(CarStationStatus value)
{
	DEBUGINFO("%d\n",value);
	if(mCarStationStatus == value){
		return;
	}
	mCarStationStatus = value;
	CarStatus.xStationStatus = value; // 更新全局变量的在站状态
	bEeprom_Check_Conn();
	bEeprom_Write_Byte(EEP_ADD_CAR_STATION_STATUS,value);
}

/**
从eeprom获取进站状态
*/
static CarStationStatus Car_Read_Station_Status(void)
{
	uint8_t result = 0;
	bEeprom_Check_Conn();
	bEeprom_Read_Byte(EEP_ADD_CAR_STATION_STATUS,&result);	
	DEBUGINFO("result:%d\n",result);
	// 有时候读出来的值变为160，原因未知，暂时强制限制范围
	if(result > 1)
	{
		result = 1;
	}
	CarStatus.xStationStatus = result; // 更新全局变量的在站状态
	return (CarStationStatus)result;
}



void vBoxCtrlTask(void *argument)
{
	DEBUGINFO("start");

	eBoxCtrlType box_msg;

	//初始化数码管显示
	NumDisp_Init();

	// //设置数码管显示的数字
	// NumDisp_SetNumber((uint16_t)111);
	// NumDisp_BlueShan();

	// vUV_Clean_enable();
	// osDelay(pdMS_TO_TICKS(2000));
	// vUV_Clean_disable();

  	while(1)
	{
		if(startFinishedFlag == 1){
			break;
		}
		DEBUGINFO("waiting for HMI start");
		osDelay(pdMS_TO_TICKS(300));	
	}

	mCarStationStatus = Car_Read_Station_Status();

	// car out检测
	if( mCarStationStatus == OutStation ){
		HMI_Set_RFCardPage();	// 发送命令切换到”请刷rfid卡“页面;
	}
	
	// car in检测
	if( mCarStationStatus == InStation ){
		HMI_Force_Home_Page();	// 强制跳转到home页面
	}

	while(1)
	{
		if(osMessageQueueGet(xBox_Ctrl_QueueHandle, &box_msg, NULL, osWaitForever) == osOK)
		{
			DEBUGINFO("box_msg = %d\r\n",box_msg);
			switch (box_msg)
			{
				//*********************************** 车厢电子锁操作 **************************************
				case BoxElockOps:
					DEBUGINFO("UvClean_IsRunning=%d, mCarStationStatus=%d, xBoxLocked=%d, HMI_Is_Button_En=%d\r\n",
						UvClean_IsRunning(),mCarStationStatus,CarStatus.xBoxLocked,HMI_Is_Button_En());
					if( !UvClean_IsRunning()&&
						( mCarStationStatus == InStation ) && 
						( CarStatus.xBoxLocked == Locked ) && 
						HMI_Is_Button_En()) // 检测HMI是否允许按键操作
					{		
						//delay_ms(350);
						ELock_unLock();
					}
					break;


				//*********************************** 车厢RFID登录超时 **************************************
				case RfidLoginTimeout:
					RFID_ResetLoginStatus();
					HMI_CheckRFCard(0); 
					break;


				//*********************************** 更新进出站状态 **************************************
				case UpdateStationStatus:

					DEBUGINFO("ServerToCarData.xStationStatus=%d, mCarStationStatus=%d\r\n",ServerToCarData.xStationStatus,mCarStationStatus);

					// car out检测
					if(( ServerToCarData.xStationStatus == OutStation ) && ( mCarStationStatus == InStation ) )
					{
						if( CarStatus.xBoxLocked == Locked )
						{
							DEBUGINFO("Update to OutStation\r\n");
							Car_Set_Station_Status(OutStation);// 设置小车状态为OutStation
							HMI_Set_RFCardPage();	// 发送命令切换到”请刷rfid卡“页面;
						} else {
							DEBUGINFO("Can't Set OutStation!! Box not Locked!!\r\n");
						}
					}
					
					// car in检测
					if( (ServerToCarData.xStationStatus == InStation) && (mCarStationStatus == OutStation) )
					{
						DEBUGINFO("Update to InStation\r\n");
						Car_Set_Station_Status(InStation);// 设置小车状态为InStation
						HMI_Force_Home_Page(); 					// 跳转到home页面
					}
					break;


				//*********************************** 更新电子锁状态 **************************************
				case UpdateBoxLockStatus:
					if( CarStatus.xBoxLocked == Locked )
					{
						// 锁上
						DEBUGINFO("Locked\n");
						HMI_Update_LockStatus_Req(1); // 发送电子锁的状态到LCD(HMI)
					} else {
						// 解锁
						DEBUGINFO("UnLock\n");
						// 如果在消毒时，则停止消毒，跳转到消毒停止页面，记录已消毒的时间
						if(UvClean_IsRunning()){
							UvClean_Stop(); 
							UvClean_Save_Record();
							HMI_Change_Page(pgWarningUvCleanCanceled);
						}
						HMI_Update_LockStatus_Req(0); // 发送电子锁的状态到LCD(HMI)
					}
					break;
				

				//*********************************** 更新UV消毒状态 **************************************
				case UpdateUVCleanStatus:
					//消毒结束，保存本次消毒开始的rtc时间 + 消毒时长
					if( !CarStatus.ucUVTimeRemain )
					{
						UvClean_Save_Record();
					}
					//推送UV清洁时间到HMI
					HMI_Check_Uv_Clean(CarStatus.ucUVTimeRemain);
					DEBUGINFO("UVTimeRemain:%d\n",CarStatus.ucUVTimeRemain);
					break;

				default:
					break;
			} 
			Robot_Event(); // 上报小车状态
		}
	}
}


void vBoxLEDTask(void *argument)
{
  uint16_t Counter = 0;

	while(1)
	{
		//light control
		/*
		1、消毒中，黄灯
		2、in statation,且电子锁关闭，蓝灯闪烁
		3、in statation,且电子锁打开，红蓝闪烁
		4、out statation,且电子锁关闭，蓝灯常亮
		5、out statation,且电子锁打开，红蓝闪烁
		*/
		if(UvClean_IsRunning()){
			vRGB_LED(YELLOW);
		}else if(mCarStationStatus == InStation){	
			// 小车进站
			if(CarStatus.xBoxLocked == Locked){
				// 车厢锁上
				if(Counter%FENZI == 0){
					vRGB_LED(LED_OFF);
				}else if(Counter%FENZI == FENMU){
					vRGB_LED(BLUE);
				}
			}else{
				// 车厢解锁
				if(Counter%FENZI==0){
					vRGB_LED(RED);
				}else if(Counter%FENZI == FENMU){
					vRGB_LED(BLUE);
				}
			}
		}else{
			// 小车出站
			if(CarStatus.xBoxLocked == Locked){
				// 车厢锁上
				vRGB_LED(BLUE);
			}else{
				// 车厢解锁
				if(Counter%FENZI == 0){
					vRGB_LED(RED);
				}else if(Counter%FENZI == FENMU){
					vRGB_LED(BLUE);
				}
			}
		}
		
		// //cpu 状态指示灯，闪烁表示正常运行
		// if(Counter%FENZI==0){
		// 	LED_SYS_OFF();
		// }else if(Counter%FENZI==FENMU){
		// 	LED_SYS_ON();
		// }
		Counter++;
		osDelay(pdMS_TO_TICKS(100));
	}
}
