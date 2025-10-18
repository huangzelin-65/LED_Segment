#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "NumDisplay.h"
#include "adaptor_box.h"
#include "Task_BoxCtrl.h"
#include "adaptor_eeprom.h"
#include "DwinHMI.h"
#include "UV_Clean.h"

//闪灯使用
#define FENZI 12
#define FENMU	6

//小车在站状态
static CarStationStatus mCarStationStatus=InStation;

//锁状态
static ELockStatus mNowLockStatus = Lock;
static uint8_t mOldElock1 = 255;
static uint8_t mOldElock2 = 255;

//按键状态
static uint8_t mButtonPress = 0;

extern _CarRunStatus_obj CarRunStatus_obj;
extern _CarCheckFlag_obj CarCheckFlag_obj;
extern ServerToCarData ServerToCarData_obj;

ELockStatus Car_Get_Elock_Status(void){
	return mNowLockStatus;
}

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
	if(mCarStationStatus == value){
		return;
	}
	mCarStationStatus=value;
	bEeprom_Check_Conn();
	bEeprom_Write_Byte(EEP_ADD_CAR_STATION_STATUS,value);
}

/**
从eeprom获取进站状态
*/
static CarStationStatus Car_Read_Station_Status(void)
{
	uint8_t result=0;
	bEeprom_Check_Conn();
	bEeprom_Read_Byte(EEP_ADD_CAR_STATION_STATUS,&result);	
	DEBUGINFO("result:%d\n",result);
	return (CarStationStatus)result;
}

uint8_t Button_Gpio_Press_Status(void)
{
	return mButtonPress;
}

void Button_Gpio_Press_Set(uint8_t Val)
{
	 mButtonPress = Val;
	 DEBUGINFO("mButtonPress:%d\n",mButtonPress);
}

void vBoxCtrlTask(void *argument)
{
  //初始化数码管显示
  NumDisp_Init();

  // //设置数码管显示的数字
  // NumDisp_SetNumber((uint16_t)111);
  // NumDisp_BlueShan();

  // vUV_Clean_enable();
  // osDelay(2000);
  // vUV_Clean_disable();

  // vRGB_LED(RED)
  // osDelay(2000);
  // vRGB_LED(GREEN);
  // osDelay(2000);
  // vRGB_LED(BLUE);
  // osDelay(2000);
  // vRGB_LED(YELLOW);
  // osDelay(2000);
  // vRGB_LED(WHITE);
  // osDelay(2000);
  // vRGB_LED(LED_OFF);

  uint16_t Counter = 0;
	
	Button_Gpio_Press_Set(0);

	uint8_t	elock1 = ELOCK1_LEVEL;//1:上锁 0:没上锁
	uint8_t	elock2 = ELOCK2_LEVEL;//1:上锁 0:没上锁
	uint8_t lockIcon = 2;

	mCarStationStatus = Car_Read_Station_Status();
	// car out检测
	if( mCarStationStatus == OutStation ){
		HMI_Set_RFCardPage();	// 发送命令切换到”请刷rfid卡“页面;
	}
	
	// car in检测
	if( mCarStationStatus == InStation ){
		HMI_Force_Home_Page();	// 强制跳转到home页面
	}

	

	DEBUGINFO("start\n");

  while(1)
  {
		// elock control
		elock1 = ELOCK1_LEVEL;//1:上锁 0:没上锁 
		elock2 = ELOCK2_LEVEL;//1:上锁 0:没上锁

		// car out检测
		if( ServerToCarData_obj.xStationStatus == OutStation && mCarStationStatus == InStation){
			if(mCarStationStatus==InStation){
				Car_Set_Station_Status(OutStation);// 设置小车状态为OutStation
			}
			HMI_Set_RFCardPage();	// 发送命令切换到”请刷rfid卡“页面;
		}
		
		// car in检测
		// 如果虚拟按键没有设置currentInStationEn，则强制跳转到home页面
		if( ServerToCarData_obj.xStationStatus == InStation && mCarStationStatus == OutStation){
			HMI_Force_Home_Page();
		}

		//新款车厢没有按钮
		// if(!UvClean_IsRunning()&&
		// 	mCarStationStatus==InStation&&
		// 	HMI_Is_Button_En())
		// {
		// 	Button_LED_En(1);		
		// }
		// else{
		// 	Button_LED_En(0);
		// }


		//check button
		if(!UvClean_IsRunning()&&
			mCarStationStatus==InStation&&
			(elock1||elock2) && 
			Button_Gpio_Press_Status()&&
			HMI_Is_Button_En())
		{		
			DEBUGINFO("elock1=%d elock2=%d",elock1,elock2);
			//delay_ms(350);
			if(ELock_unLock()) 
			{
				DEBUGINFO("error: unlock fail\n");
			}

			Button_Gpio_Press_Set(0);
		}
		
			
	  //check lock
	  // 轮询电子锁状态是否发生变化
		if(elock1!= mOldElock1 || elock2!= mOldElock2)
		{
			// 锁上
			if(elock1||elock2){
				CarCheckFlag_obj.BoxLocked = 1; //车厢锁上
				DEBUGINFO("Box Locked\n");
				mNowLockStatus=Lock;
				HMI_Update_LockStatus_Req(1); // 发送电子锁的状态到LCD(HMI)
			}else{
				// 未锁上
				// 如果在消毒时，则停止消毒，跳转到消毒停止页面，记录已消毒的时间
				CarCheckFlag_obj.BoxLocked = 0; //车厢未锁上
				if(UvClean_IsRunning()){
					UvClean_Stop(); 
					UvClean_Save_Record();
					HMI_Change_Page(pgWarningUvCleanCanceled);
				}
				DEBUGINFO("Box UnLock\n");
				mNowLockStatus=Open;
				HMI_Update_LockStatus_Req(0); // 发送电子锁的状态到LCD(HMI)
			}
			mOldElock1 = elock1;
			mOldElock2 = elock2;
		}

		if(Counter%5 == 0)
		{		
			if(elock1==1 || elock2==1)
			{
				if(lockIcon != 1){
					HMI_Update_LockStatus_Req(1);
					lockIcon = 1;
				}
			}else{
				if(lockIcon != 0){
					HMI_Update_LockStatus_Req(0);
					lockIcon = 0;
				}
			}
		}
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
		}else if(mCarStationStatus==InStation){	
			if(mNowLockStatus==Lock){
				if(Counter%FENZI==0){
					vRGB_LED(LED_OFF);
				}else if(Counter%FENZI==FENMU){
					vRGB_LED(BLUE);
				}
			}else{
				if(Counter%FENZI==0){
					vRGB_LED(RED);
				}else if(Counter%FENZI==FENMU){
					vRGB_LED(BLUE);
				}
			}
		}else{
			
			if(mNowLockStatus==Lock){
				vRGB_LED(BLUE);
			}else{
				if(Counter%FENZI==0){
					vRGB_LED(RED);
				}else if(Counter%FENZI==FENMU){
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
		osDelay(100);
	}
}

extern osMessageQueueId_t xUV_QueueHandle;

void vUVCleanTask(void *argument)
{
    uint8_t msg = 0;

    while(1)
    {
      if(osMessageQueueGet(xUV_QueueHandle, &msg, NULL, osWaitForever) == osOK)
      {
        //处理数据
        if(!msg)
        {
            UvClean_Save_Record();
        }
        //推送UV清洁时间到HMI
        HMI_Check_Uv_Clean(msg);
        DEBUGINFO("xiaoDuTimer:%d\n",msg);

      }
    }
}