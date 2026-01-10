#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include "Calculate.h"
#include "LogDebugInfo.h"
#include "adaptor_rfid.h"
#include "adaptor_eeprom.h"
#include "adaptor_box.h"
#include "BoxRfidReader.h"
#include "DwinHMI.h"
#include "Task_BoxCtrl.h"

#define 	MAX_CARD_PSWD 					10
#define 	CARD_PSWD_BUF_LEN  				60

#ifdef RFCARD_MANUAL_READ
static uint8_t pasdType = RFID_PASD_A;
#endif

static uint8_t localPasswd[6]= {0x11,0x22,0x33,0x44,0x55,0x66};
//static uint8_t deafultPasswd[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


static uint8_t userBlock = 10;
 
static uint8_t txtAdmin[16] = {	0x6c,0x65,0x76,0x65,0x6c,0x5f,
							0x35,0x00,0x00,0x00,0x00,0x00
							,0x00,0x00,0x00,0x00};
static uint8_t txtUser[16] ={	0x6c,0x65,0x76,0x65,0x6c,0x5f,
							0x31,0x00,0x00,0x00,0x00,0x00
							,0x00,0x00,0x00,0x00};

static uint8_t enScan=1;//TODO if=0. can control the scan enable
static uint8_t isLogin=0;			//是否已经登录,0,no,1,user,5,admin
static uint8_t rfidTimeout = 1;		//判断本次刷卡是否超时，1：超时

uint8_t CardPasswordBuf[CARD_PSWD_BUF_LEN] = {0};//全局变量 IDcard的密码buf
uint8_t CardPasswordNum = 0;//全局变量 IDcard的密码个数

uint8_t ucBoxRfid_Rx_Buffer[2][BOX_RFID_RX_BUF_SIZE]; // 接收缓冲区
uint8_t ucBoxRfid_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern CarStatus_t CarStatus;
extern osTimerId_t xBoxRfidLoginTimerHandle;
extern osSemaphoreId_t xBoxRfidRxSemHandle;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;


/**
已经登录
*/
void RFID_Set_Logined(uint8_t level)
{
	if(level ==0){
		return;
	}
	DEBUGINFO("Box RFID Logined\n");
	HMI_CheckRFCard(level); 
	isLogin = level;
}

/**
重置登录状态
*/
void RFID_ResetLoginStatus(void)
{
	DEBUGINFO("RFID_ResetLoginStatus\n");
	isLogin = 0;
}
/*
获取登录状态
*/
uint8_t RFID_GetLoginStatus(void)
{
	return isLogin;
}


/*
启用RFID扫描
*/
void RFID_Scan_Enable(uint8_t en)
{
	enScan = 1;//TODO if enable . can control the scan enable//20200605: always on 
 	//enScan = en;
	osTimerStop(xBoxRfidLoginTimerHandle);
}


void RFID_Print_CardPasswd(void)
{
	//uint8_t i = 0;
	if(CardPasswordNum==0) {
		DEBUGINFO("use default passwd:FF FF FF FF FF FF\n");
	}
	else {
			DEBUGINFO("total password=%d\n",CardPasswordNum);
	
			for(int i =0;i<CardPasswordNum;i++){

				DEBUGINFO("No.%d :%2X %2X %2X %2X %2X %2X\n",i+1,
					CardPasswordBuf[i*6],
					CardPasswordBuf[i*6+1],
					CardPasswordBuf[i*6+2],
					CardPasswordBuf[i*6+3],
					CardPasswordBuf[i*6+4],
					CardPasswordBuf[i*6+5]);
			}
			DEBUGINFO("Print end.\n");
	}
}


/**
 * 更新卡密码信息。
 *
 * 该函数检查EEPROM连接状态，如果连接正常，则读取EEPROM中的卡密码数量和密码信息。
 * 如果卡密码数量超过10，则重置卡密码数量为0。
 *
 * @note 该函数不返回任何值。
 */
static void RFID_Update_CardPasswd(void)
{
	uint8_t num = 0;
	if(b_Eeprom_Check_Conn() == true)
	{
		b_Eeprom_Read_Byte(EEP_ADD_IDCARD_PASSWORD_NUM,&num);
		if(num>10) {
			CardPasswordNum=0;
		}
		else {
			b_Eeprom_Read_Buf(EEP_ADD_IDCARD_PASSWORD,CardPasswordBuf,num*6);
			CardPasswordNum = num;
		}
		DEBUGINFO("CardPasswordNum=%d\n",CardPasswordNum);
		RFID_Print_CardPasswd();

	}
	else
	{
		DEBUGINFO("b_Eeprom_Check_Conn fail\n");
	}
}

/**
 * 写入卡密码
 *
 * 该函数用于写入卡密码到EEPROM中。它首先检查EEPROM的连接状态，如果连接正常，则检查卡密码是否已经存在。
 * 如果卡密码不存在，则将其写入EEPROM中，并更新卡密码数量。
 *
 * @param ucData 要写入的卡密码数据
 * @return 1表示卡密码已经存在，0表示写入成功
 */
uint8_t RFID_Write_CardPasswd(uint8_t *ucData)
{
	if(b_Eeprom_Check_Conn() == true)
	{
		if(CardPasswordNum>0) {
			for(int i=0; i<CardPasswordNum; i++) {
				if(compareArray(ucData,CardPasswordBuf+i*6,6)==0)
					return 1;
			}
		}
		if(CardPasswordNum<MAX_CARD_PSWD) {
			if(CardPasswordNum>0)
				b_Eeprom_Write_Buf(EEP_ADD_IDCARD_PASSWORD+6,CardPasswordBuf,CardPasswordNum*6);
			b_Eeprom_Write_Buf(EEP_ADD_IDCARD_PASSWORD,ucData,6);
			CardPasswordNum++;
			b_Eeprom_Write_Byte(EEP_ADD_IDCARD_PASSWORD_NUM,CardPasswordNum);
		}
		else {
			b_Eeprom_Write_Buf(EEP_ADD_IDCARD_PASSWORD+6,CardPasswordBuf,(MAX_CARD_PSWD-1)*6);
			b_Eeprom_Write_Buf(EEP_ADD_IDCARD_PASSWORD,ucData,6);
		}
		RFID_Update_CardPasswd();
	}
	else
	{
		DEBUGINFO("b_Eeprom_Check_Conn fail\n");
	}

	return 0;
}


/*
刷卡后的有效时间
*/
static void RFID_Start_Loginin_Timer(void)
{
	DEBUGINFO("RFID_Start_Loginin_Timer\n");
	//统计刷卡成功后是否超时
	osTimerStop(xBoxRfidLoginTimerHandle);
	rfidTimeout = 0;
	osTimerStart(xBoxRfidLoginTimerHandle, pdMS_TO_TICKS(1000*5));
}



/**
 * 检查用户权限等级。
 *
 * 该函数通过比较传入的temp数组与预定义的txtAdmin和txtUser数组来判断用户是否为管理员或普通用户。
 * 如果temp数组与txtAdmin或txtUser数组匹配，则返回temp数组第6个元素的值，表示用户等级。
 * 如果temp数组与txtAdmin和txtUser数组都不匹配，则返回0，表示用户权限等级未知。
 *
 * @param temp 用户权限信息数组
 * @return 用户权限等级（0表示未知）
 */
static uint8_t RFID_CheckUserLevel(uint8_t *temp)
{
	uint8_t result = 0;

	result = 1;
	uint8_t i =0;
	for(i=0;i<16;i++){
		if(temp[i]!=txtAdmin[i]){
			result = 0;
			break;
		}
	}

	if(result == 1){
		DEBUGINFO("admin:%X\n",temp[6]);
		return temp[6];
	}

	result = 1;
	for(i=0;i<16;i++){
		if(temp[i]!=txtUser[i]){
			result = 0;
			break;
		}
	}
	
	if(result == 1){
		DEBUGINFO("user:%X\n",temp[6]);
		return temp[6];
	}
	
	return result;
}

/*
获取是否超时
*/
uint8_t RFID_GetTimeOut(void)
{
	return rfidTimeout;
}


/**
* 刷卡超时
*/
void vBoxRfidLoginTimerCallback(void *argument)
{
	rfidTimeout = 1;

	// 发送车厢RFID登录超时消息
	eBoxCtrlType x_BoxMsg = RfidLoginTimeout;
	osMessageQueuePut(xBox_Ctrl_QueueHandle, &x_BoxMsg, 0, pdMS_TO_TICKS(100));
}

/**
读卡器任务,验证密码
*/
void vBoxRfidTask(void *argument)
{
	RFID_Update_CardPasswd();

	osDelay(pdMS_TO_TICKS(1000));
	M5_Beep(1); 
	osDelay(pdMS_TO_TICKS(500));
#ifdef RFCARD_MANUAL_READ	
	while(M5_ReadWriteMode()!=0) osDelay(pdMS_TO_TICKS(500));
#else
	M5_ReadOnlyMode8(userBlock,localPasswd,16);
#endif

	DEBUGINFO("M5_ReadWriteMode succss");
	
	uint8_t temp[16];
	uint8_t level = 0;
	while(1)
	{
		//if((Car_Get_Station_Status() == InStation) && (enScan >0))
		if( (CarStatus.xIsCarRunning != CarRunning) && (enScan >0) )// 车辆运行中不能刷卡开箱
		{
			#ifdef RFCARD_MANUAL_READ
			uint8_t result;
	
			//读取卡号和卡的类型
			if((result=M5_FindCard(0x26))!=0 )//通用 CPU 和 M1卡
			{
				DEBUGINFO("find error result=0x%x\n",result);
				continue;
			}
			DEBUGINFO("M5_FindCard succss\n");
			
			if((result=M5_Anticoll())!=0)
			{
				DEBUGINFO("anticoll error result=0x%x\n",result);
				continue;
			}
			DEBUGINFO("M5_Anticoll succss\n");
			
			if((result=M5_SelectCard())!=0)
			{
				DEBUGINFO("select error result=0x%x\n",result);
				continue;
			}
			
			if((result=M5_Auth(pasdType, userBlock, localPasswd))==0){
				memset(temp,0,16);
				level = 0;
				if((result=M5_Easy_Read(userBlock,pasdType, localPasswd,temp))==0)
				{								
					level = RFID_CheckUserLevel(temp);								
					if(level>0){
						RFID_Start_Loginin_Timer();
						RFID_Set_Logined(level);
						M5_Beep(1); 
						osDelay(pdMS_TO_TICKS(500));
					if((result=M5_FindCard(0x26))!=0 )
						continue;
			
					if((result=M5_Anticoll())!=0)	
						continue;
		
					if((result=M5_SelectCard())!=0)
						continue;

						M5_Halt();
					}
				}
			}				
			#else
			//DEBUGINFO("Car not Running");
			if(M5_WaitCard(temp)){
				level = 0;
				level = RFID_CheckUserLevel(temp);	
				RFID_Start_Loginin_Timer();
				RFID_Set_Logined(level); 
			}
			memset(temp,0,16);
			#endif
		}
		
		//osDelay(pdMS_TO_TICKS(500));
		osDelay(pdMS_TO_TICKS(1000));
	}
}

void vBoxRfidEventTask(void *argument)
{
	uint32_t ucReciveLen = 0;
	//启动DMA接收
  vBoxRfid_Start_DMA_Receive(ucBoxRfid_Rx_Buffer[ucBoxRfid_current_buf_idx]);

  while(1) {
    // 等待DMA接收完成信号
    if (osSemaphoreAcquire(xBoxRfidRxSemHandle, osWaitForever) == osOK)
    {
      //DEBUGINFO("ucBoxRfid_current_buf_idx:%d\r\n",ucBoxRfid_current_buf_idx);

			ucReciveLen = ulBoxRfid_Get_DMA_Receive_Len();
			DEBUGINFO("Box rfid received len:%d ,data:",ucReciveLen);
			vPrint_Array(ucBoxRfid_Rx_Buffer[ucBoxRfid_current_buf_idx],ucReciveLen);
			
			if(CarStatus.xIsCarRunning != CarRunning)
			{
				// 处理接收到的数据
				vBoxRfid_ReceiveDataHandler(ucBoxRfid_Rx_Buffer[ucBoxRfid_current_buf_idx],ucReciveLen);
			}
			

      // 切换缓冲区并重启接收
      ucBoxRfid_current_buf_idx ^= 1;
      vBoxRfid_Start_DMA_Receive(ucBoxRfid_Rx_Buffer[ucBoxRfid_current_buf_idx]);
    }
    
  }

}


