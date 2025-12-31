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

extern uint8_t startFinishedFlag;
extern CarStatus_t CarStatus;
extern ServerToCarData_t ServerToCarData;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;
extern osMessageQueueId_t xMotion_QueueHandle;


/**
保存屏幕锁定状态
*/
void vSet_Screen_LockStatus(eLockStatusType value)
{
	DEBUGINFO("%d\n",value);

	if(Locked == value)
	{
		if(CarStatus.xBoxLocked == Locked){
			DEBUGINFO("Lock Screen");
			HMI_Set_RFCardPage();	// 发送命令切换到”请刷rfid卡“页面;
		} else {
			DEBUGINFO("Can't Lock Screen!! Box not Locked!!\r\n");
		}
	}
	else if (UnLock == value)
	{
		DEBUGINFO("UnLock Screen");
		HMI_Force_Home_Page(); 		// 跳转到home页面
	}
	else
	{
		DEBUGINFO("Unknow Lock Status");
		return;
	}

	CarStatus.xScreenLockStatus = value; // 更新全局变量的在站状态
	bEeprom_Check_Conn();
	bEeprom_Write_Byte(EEP_ADD_SCREEN_LOCK_STATUS,value); // 保存到eeprom
}

/**
从eeprom获取屏幕锁定状态
*/
static eLockStatusType xRead_Screen_LockStatus(void)
{
	uint8_t result = 0;
	bEeprom_Check_Conn();
	bEeprom_Read_Byte(EEP_ADD_SCREEN_LOCK_STATUS,&result);	
	DEBUGINFO("Read Status:%d\n",result);
	
	// 有时候读出来的值变为160，原因未知，暂时强制限制范围
	if(result > 1){
		result = 1;
	}
	
	return (eLockStatusType)result;
}



void vBoxCtrlTask(void *argument)
{
	DEBUGINFO("start");

	eBoxCtrlType box_msg;
	uint8_t ucMotion_msg;

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

	CarStatus.xScreenLockStatus = xRead_Screen_LockStatus(); // 读取屏幕锁定状态
	vSet_Screen_LockStatus(CarStatus.xScreenLockStatus); // 设置屏幕锁定状态


	while(1)
	{
		if(osMessageQueueGet(xBox_Ctrl_QueueHandle, &box_msg, NULL, osWaitForever) == osOK)
		{
			// DEBUGINFO("box_msg = %d",box_msg);
			switch (box_msg)
			{
				//*********************************** 车厢电子锁操作 **************************************//
				case BoxElockOps:
					DEBUGINFO("case BoxElockOps");
					DEBUGINFO("UvClean_IsRunning=%d, xIsCarRunning=%d, xBoxLocked=%d, HMI_Is_Button_En=%d",
						UvClean_IsRunning(),CarStatus.xIsCarRunning,CarStatus.xBoxLocked,HMI_Is_Button_En());
					if( !UvClean_IsRunning()&&
						( CarStatus.xIsCarRunning == CarStop ) &&
						( CarStatus.xBoxLocked == Locked ) && 
						HMI_Is_Button_En()) // 检测HMI是否允许按键操作
					{		
						//delay_ms(350);
						ELock_unLock();
					}
					break;


				//*********************************** 车厢RFID登录超时 **************************************//
				case RfidLoginTimeout:
					DEBUGINFO("case RfidLoginTimeout");
					RFID_ResetLoginStatus();
					HMI_CheckRFCard(0); 
					break;


				//*********************************** 更新电子锁状态 **************************************//
				case UpdateBoxLockStatus:
					DEBUGINFO("case UpdateBoxLockStatus");
					if( CarStatus.xBoxLocked == Locked )
					{
						// 锁上
						DEBUGINFO("Locked\n");
						HMI_Update_LockStatus_Req(1); // 发送电子锁的状态到LCD(HMI)
					} else {
						// 解锁
						DEBUGINFO("UnLock\n");
						// 如果在消毒时，则停止消毒，跳转到消毒停止页面，记录已消毒的时间
						if(UvClean_IsRunning())
						{
							UvClean_Stop(); 
							UvClean_Save_Record();
							HMI_Change_Page(pgWarningUvCleanCanceled);
						}
						// 如果在运动中，则停止运动
						if(CarStatus.xIsCarRunning != CarStop)
						{
							ucMotion_msg = CarStop;
							if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
							{
								DEBUGINFO("send motion msg error");
							}
						}
						HMI_Update_LockStatus_Req(0); // 发送电子锁的状态到LCD(HMI)
					}
					break;
				

				//*********************************** 更新UV消毒状态 **************************************//
				case UpdateUVCleanStatus:
					DEBUGINFO("case UpdateUVCleanStatus");
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
		2、电子锁关闭，蓝灯闪烁
		3、电子锁打开，红蓝闪烁
		*/
		if(UvClean_IsRunning()){
			vRGB_LED(YELLOW);
		}else{	
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
