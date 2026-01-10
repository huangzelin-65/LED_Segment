#include "main.h"
#include "ZhongnengRfidReader.h"
#include "Task_CarRfid.h"
#include "adaptor_rfid.h"
#include <string.h>
#include "LogDebugInfo.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "adaptor_power.h"

#ifdef ZHONGNENG_RFID

uint8_t Read_9bit_id[23] = {
		0x1D,0x17,0xA1,0x00,0x09,	//固定前缀
		PASSWORD_16BYTE,			//密码宏
		0x00,						//校验码，后续计算
		0xB2						//固定后缀
};

uint8_t set_reading_way_succeed[8] = {0x1D,0x08,0xA1,0x01,0x00,0x00,0xA8,0xB2};

//读卡器的开LED跟关BUZZER的指令及成功回复
uint8_t set_LED_and_BUZZER[8] = {0X1D,0X08,0XA2,0X00,		//固定前缀
							LED_and_BUZZER,				//打开LED,关闭蜂鸣器
							0X00,						//校验码
							0XB2} ;						//固定后缀

uint8_t set_LED_BUZZER_succeed[8]={0x1D,0x08,0xA2,0x01,0x00,0x00,0xAB,0xB2};

//设置仅上报一次卡号的指令及成功回复
uint8_t set_Report_Interval [10] =  {0x1D,0x0A,0xA3,0x00,	//固定前缀
								REPORT_INTERVAL_TIME,	//上报间隔，全F表示上报一次
								0xA9,					//校验码
								0xB2};					//固定后缀

uint8_t set_Report_Interval_succeed [8] = {0x1D,0x08,0xA3,0x01,0x00,0x00,0xAA,0xB2 };

//u8 receive_RfidReader_Report [30] = {0};		//用来存储设置读卡器指令后的上报反馈



extern UART_HandleTypeDef huart5;
extern osSemaphoreId_t xCarRfidRxSemHandle;
extern SemaphoreHandle_t xUART_TxSemaphore_ZHONGNENG ;
extern u32 ucReciveLen ;
extern u32 package_start_idx  ;
extern u8 ucCarRfid_Rx_Buffer[CAR_RFID_RX_BUF_SIZE];
/**
 * @brief 串口发送指令，接收响应并校验
 * @param huart: 串口句柄
 * @param send_buf: 发送缓冲区
 * @param send_len: 发送长度
 * @param expect_buf: 期望的响应数据
 * @param expect_len: 期望的响应长度
 * @param timeout: 超时时间
 * @retval "OK": 成功，"FAIL": 失败
 */
char* RFID_Uart_SendRecvCheck(UART_HandleTypeDef *huart,
                                     u8 *send_buf, uint16_t send_len,
                                     u8* expect_buf,uint16_t expect_len,
                                     u32 timeout)
{
    HAL_StatusTypeDef uart_state;  	//用于判断是否发送
    osStatus_t  sem_state;           //用于判断信号量的获取是否成功

    //发送指令操作
    uart_state = HAL_UART_Transmit_DMA(huart, send_buf, send_len);
    if (uart_state != HAL_OK)
    {
        DEBUGINFO("RFID: Send cmd failed (uart state: %d)\r\n", uart_state);
        return "FAIL";
    }

    sem_state = osSemaphoreAcquire(xUART_TxSemaphore_ZHONGNENG, pdMS_TO_TICKS(RFID_WAIT_REPLY_TIMEOUT));
    if (sem_state != osOK)
    {
        DEBUGINFO("RFID: Send cmd timeout\r\n");
        HAL_UART_AbortReceive(huart);
        return "FAIL";
    }

    static uint8_t dma_start_rx_flag = 0 ;
    if(dma_start_rx_flag == 0)
    {
        //接收读卡器的响应
        dma_start_rx_flag = 1;
        //启动循环DMA
        vCarRfid_Start_DMA_Receive(ucCarRfid_Rx_Buffer);
    }

    sem_state = osSemaphoreAcquire(xCarRfidRxSemHandle, osWaitForever);
    if (sem_state != osOK)
    {
        DEBUGINFO("RFID: Recv response timeout (uart state: %d)\r\n", uart_state);
        HAL_UART_AbortReceive_IT(huart);   	// 信号量获取异常时，关闭接收
        return "FAIL";
    }
    ucReciveLen = ulCarRfid_Get_DMA_Receive_Len(&package_start_idx);

    //判断是否设置成功
    if (memcmp(expect_buf, &ucCarRfid_Rx_Buffer[package_start_idx], expect_len) != 0)
    {
        DEBUGINFO("RFID: Response mismatch\r\n");
        return "FAIL";
    }

    return "OK";
}


//因为读卡器的初始化指令中包含校验码，因此需要根据宏设置的密码计算校验码
u8 Calc_XOR_CheckCode(u8* tset_command, u8 tset_command_Len)
{
    u8 xor_check = 0x00;
    for(u16 i = 1; i <= tset_command_Len-3; i++)  // 下标1（0x17）~ 下标20（密码最后1字节）
    {
        xor_check ^= tset_command[i];
    }
    return xor_check;
}

// 二进制字节数组转十进制数字字符串，仅处理ID段
void bin_id_to_num_str(uint8_t *src, u32 src_len, char *dst)
{
    if (!src || !dst || src_len == 0) {
        memset(dst, 0, CARD_NUM_LEN + 1);		//用字符串dst来存储数据
        return;
    }
    memset(dst, 0, CARD_NUM_LEN + 1);
    for (u32 i = 0; i < src_len && i < CARD_NUM_LEN; i++) {
        // 每个字节的十进制值转单个数字字符（如0x01→'1'，0x00→'0'）
        dst[i] = (src[i] % 10) + '0';
    }
}

//设置读卡器的读卡方式
RFID_StatusTypeDef Set_Reading_Way(void)
{
	//计算校验码
	Read_9bit_id[sizeof(Read_9bit_id)-2] = Calc_XOR_CheckCode(Read_9bit_id, sizeof(Read_9bit_id));
    
    char* comm_result = NULL;
    comm_result = RFID_Uart_SendRecvCheck(&huart5, Read_9bit_id, sizeof(Read_9bit_id),
                                        set_reading_way_succeed, sizeof(set_reading_way_succeed),
                                        RFID_WAIT_REPLY_TIMEOUT);
    if (strcmp(comm_result, "OK") != 0)
    {
        DEBUGINFO("RFID: Reading_way setting failed, error code: %d\r\n", RFID_STATUS_READING_WAY_FAIL);
        return RFID_STATUS_READING_WAY_FAIL;
    }
    return RFID_STATUS_OK;
}

//设置LED跟蜂鸣器
RFID_StatusTypeDef Set_LED_BUZZER(void)
{
	set_LED_and_BUZZER[sizeof(set_LED_and_BUZZER)-2] = Calc_XOR_CheckCode(set_LED_and_BUZZER, sizeof(set_LED_and_BUZZER));
    char* comm_result = NULL;

    comm_result = RFID_Uart_SendRecvCheck(&huart5, 
                                        set_LED_and_BUZZER, sizeof(set_LED_and_BUZZER),
                                        set_LED_BUZZER_succeed, sizeof(set_LED_BUZZER_succeed),
                                        RFID_WAIT_REPLY_TIMEOUT);
    if (strcmp(comm_result, "OK") != 0)
    {
        DEBUGINFO("RFID: LED_BUZZER setting failed, error code: %d\r\n", RFID_STATUS_LED_BUZZER_FAIL);
        return RFID_STATUS_LED_BUZZER_FAIL;
    }
    return RFID_STATUS_OK;
}

//设置上报间隔
RFID_StatusTypeDef Set_Report_Interval(void)
{
	set_Report_Interval [sizeof(set_Report_Interval)-2] = Calc_XOR_CheckCode(set_Report_Interval, sizeof(set_Report_Interval));
    char* comm_result = NULL;

    comm_result = RFID_Uart_SendRecvCheck(&huart5, 
                                        set_Report_Interval, sizeof(set_Report_Interval),
                                        set_Report_Interval_succeed, sizeof(set_Report_Interval_succeed),
                                        RFID_WAIT_REPLY_TIMEOUT);
    if (strcmp(comm_result, "OK") != 0)
    {
        DEBUGINFO("RFID: Report interval setting failed, error code: %d\r\n", RFID_STATUS_REPORT_INTERVAL_FAIL);
        return RFID_STATUS_REPORT_INTERVAL_FAIL;
    }
    return RFID_STATUS_OK;
}
//读卡器的初始化，设置三种指令
char* vCarRfidInit(void)
{
    RFID_StatusTypeDef ret[3] = {RFID_STATUS_OK,RFID_STATUS_OK,RFID_STATUS_OK};
    Sem_Init_ZHONGNENG();
    //
    ret[0] = Set_Reading_Way();
    if(ret[0] != RFID_STATUS_OK)
    {
    	return NULL;
    }

    ret[1] = Set_LED_BUZZER();
    if(ret[1] != RFID_STATUS_OK)
    {
    	return NULL;
    }

    ret[2] = Set_Report_Interval();
    if(ret[2] != RFID_STATUS_OK)
    {
    	return NULL;
    }
    return "OK";
}

//DMA 发送的信号量初始化
void Sem_Init_ZHONGNENG(void)
{
	 xUART_TxSemaphore_ZHONGNENG = xSemaphoreCreateBinary();
	 if(xUART_TxSemaphore_ZHONGNENG == NULL)
	 {
	         DEBUGINFO("ERROR: UART TX/RX信号量创建失败！\r\n");
	         while(1);
	  }
}

void CarRfid_Init_With_Retry(void)
{
	u8 Init_Reader_Cnt = 0 ;		//记录当前初始化次数
	  char* vCar_RfidInit_State = NULL ;
	  while(Init_Reader_Cnt<MAX_INIT_READER_CNT)
	  {
		  vRfid_Car_Power_Init();
		  vCar_RfidInit_State = vCarRfidInit();				//首次初始化
		  Init_Reader_Cnt ++ ;
		  if(vCar_RfidInit_State != NULL)
		  {
			  break ;
		  }
		  else		//初始化失败逻辑
		  {
			  DEBUGINFO("RFID READER INIT FAILED! \r\n ");
			  if(Init_Reader_Cnt < MAX_INIT_READER_CNT)
			  {
				  DEBUGINFO("TRY TO INIT RFID READER AGAIN! \r\n ");
				  vRfid_Car_Power_DeInit();				//关闭底盘RFID电源
				  osDelay(pdMS_TO_TICKS(100));			//断电100ms后重启
			  }
		  }
	  }
	  if(vCar_RfidInit_State != NULL)
	  {
	      DEBUGINFO("RFID READER INIT SUCCEED FINALLY, INIT CNT: %d\r\n ", Init_Reader_Cnt);
	      Init_Reader_Cnt = 0;
	  }
	  else if(Init_Reader_Cnt == MAX_INIT_READER_CNT)
	  {
	      DEBUGINFO("RFID READER INIT FAILED FINALLY, INIT CNT: %d\r\n ", Init_Reader_Cnt);
	      Init_Reader_Cnt = 0;
		  HAL_UART_DMAStop(&huart5);
		  memset(ucCarRfid_Rx_Buffer,0,CAR_RFID_RX_BUF_SIZE);
	  }
}

#endif
