#include "main.h"
#include "FreeRTOS.h"
#include "LogDebugInfo.h"
#include "semphr.h"
#include "cmsis_os2.h"
#include "Task_rs485.h"
#include "Task_Parse.h"
#include "Calculate_Crc16.h"
#include <string.h>

//====================RS485接收到的数据=============================
uint8_t rx_slave_addr = 0;				//收到的从机地址，
uint8_t rx_func_code  = 0 ;				//功能码
uint8_t rx_data_bit[3] = {0};			//接收到的数据位
uint8_t rx_crc16_low = 0;				//收到的CRC校验码低八位
uint8_t rx_crc16_high = 0;
//==============================================================

//====================数码管通信协议(7字节数据)================================
//uint8_t seted_slave_addr = 0;			//本机的从机地址，默认为SLAVE_ADDR_Init，后续提供修改功能，提供断电保存
  uint8_t seted_slave_addr = 0x01;			//先暂时等于初始地址
typedef enum
{
	FUNC_DISPLAY           = 0x01,
	FUNC_SET_BLINK_or_NOT  = 0x02,
	FUNC_CHANGE_SLAVE_ADDR = 0x03
}FUNC_CODE;
uint8_t cal_crc16_low = 0;			//计算的CRC校验码低八位
uint8_t cal_crc16_high = 0;
//==============================================================


//  声明的名称和freertos.c一致（带Handle后缀）
extern osSemaphoreId_t xRS485RxSemHandle;
extern uint16_t Rx_Len ;
extern uint8_t Rx_Buffer[Rx_Buf_Size];
uint8_t Package_Data[Rx_Buf_Size];
extern osSemaphoreId_t xSegBlinkSemHandle;
extern uint8_t num1_display ;
extern uint8_t num2_display ;
extern uint8_t num3_display ;

char* Check_Rx_Data_Valid(uint8_t* uPackage_Data)
{
	//检验从机地址
	rx_slave_addr = uPackage_Data[0];
	if( rx_slave_addr!= seted_slave_addr)
	{
		DEBUGINFO("SLAVE_ADDR_Invalid!");
		return NULL ;
	}
	//检验功能码
	rx_func_code = uPackage_Data[1];
	if(rx_func_code!=FUNC_CODE[0] && rx_func_code!=FUNC_CODE[2] && rx_func_code!=FUNC_CODE[3])
	{
		DEBUGINFO("FUNC_CODE_Invalid!");
		return NULL ;
	}
	//检验CRC校验码
	rx_crc16_low   = uPackage_Data[7];
	rx_crc16_high  = uPackage_Data[8];
	cal_crc16_low  = (CRC16(uPackage_Data,7) >> 8);
	cal_crc16_high = (CRC16(uPackage_Data,7));
	if(rx_crc16_low != cal_crc16_low || rx_crc16_high != cal_crc16_high)
	{
		DEBUGINFO("CRC_Mismatch!");
		return NULL ;
	}
}

void xPackage_Parse(void *argument)
{
    while(1)
    {
        if(xSemaphoreTake(xRS485RxSemHandle, portMAX_DELAY) == pdTRUE)
        {
            DEBUGINFO("RX_Buffer:");
            vPrint_Array(Rx_Buffer, Rx_Len);
            memcpy(Package_Data,Rx_Buffer,Rx_Len);
            DEBUGINFO("Package_Data:");
            vPrint_Array(Package_Data, Rx_Len);
            char* pCheck_Rx_Data_Valid = Check_Rx_Data_Valid(Package_Data);
            if(pCheck_Rx_Data_Valid != NULL)
				{
				num1_display =Package_Data[0];
				num2_display =Package_Data[1];
				num3_display =Package_Data[2];
				xSemaphoreGive(xSegBlinkSemHandle);
				}
            else
            {
            	DEBUGINFO("RS485_RECEIVE_fAILED!");
            }
        }
    }
}
