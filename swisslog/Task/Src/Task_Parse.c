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
  uint8_t seted_slave_addr_current ;			//未设置时默认为SLAVE_ADDR_INIT，设置后具有断电保存功能
  uint8_t seted_slave_addr_last;
typedef enum
{
	FUNC_DISPLAY           = 0x01,
	FUNC_SET_BLINK_or_NOT  = 0x02,
	FUNC_CHANGE_SLAVE_ADDR = 0x03
}FUNC_CODE;
RS485_Recv_Data_Bit rs485_recv = {0};
uint8_t cal_crc16_low = 0;			//计算的CRC校验码低八位
uint8_t cal_crc16_high = 0;
//==============================================================



extern osSemaphoreId_t xRS485RxSemHandle;
extern osSemaphoreId_t xSegBlinkSemHandle;
extern osSemaphoreId_t xChangeSlaveAddrSemHandle;

extern uint16_t Rx_Len ;
extern uint8_t Rx_Buffer[Rx_Buf_Size];
uint8_t Package_Data[Rx_Buf_Size];
uint8_t Blink_Flag = 0 ; //当收到闪烁指令时标志位置1，根据标志位判断显示模式是常量还是闪烁
uint8_t Display_Flag = 0 ; //闪烁模式下显示数字会调用xSegment_Blink，这个标志位是用来区分谁调用,如果是设置显示模式则为0，设置显示数字则为1
void LED_Segment_ON(uint8_t num1, uint8_t num2, uint8_t num3);

char* Check_Rx_Data_Valid(uint8_t* uPackage_Data)
{
	//检验从机地址
	rx_slave_addr = uPackage_Data[0];
	if( rx_slave_addr!= seted_slave_addr_current)
	{
		DEBUGINFO("SLAVE_ADDR_Invalid!");
		return NULL ;
	}
	//检验功能码
	rx_func_code = uPackage_Data[1];
	if(rx_func_code!=FUNC_DISPLAY && rx_func_code!=FUNC_SET_BLINK_or_NOT && rx_func_code!=FUNC_CHANGE_SLAVE_ADDR)
	{
		DEBUGINFO("FUNC_CODE_Invalid!");
		return NULL ;
	}
	//检验CRC校验码
	rx_crc16_low   = uPackage_Data[6];
	rx_crc16_high  = uPackage_Data[7];
	cal_crc16_low  = (CRC16(uPackage_Data,6) >> 8);
	cal_crc16_high = (CRC16(uPackage_Data,6));
	if(rx_crc16_low != cal_crc16_low || rx_crc16_high != cal_crc16_high)
	{
		DEBUGINFO("CRC_Mismatch!");
		return NULL ;
	}
	return "Data_Valid!";
}

void xPackage_Parse(void *argument)
{
    while(1)
    {
        if(xSemaphoreTake(xRS485RxSemHandle, portMAX_DELAY) == pdTRUE)
        {
            //DEBUGINFO("RX_Buffer:");
            //vPrint_Array(Rx_Buffer, Rx_Len);
            memcpy(Package_Data,Rx_Buffer,Rx_Len);
            DEBUGINFO("Package_Data:");
            vPrint_Array(Package_Data, Rx_Len);
            char* pCheck_Rx_Data_Valid = Check_Rx_Data_Valid(Package_Data);
            if(pCheck_Rx_Data_Valid != NULL)
				{
				//存储LED显示数字
            	    //根据功能码释放对应信号量,执行相应任务
					switch (Package_Data[1])
					{
						case FUNC_DISPLAY :
							{
							//rs485_recv.rx_data_bit1 = Package_Data[2];
							//rs485_recv.rx_data_bit2 = Package_Data[3];
							//rs485_recv.rx_data_bit3 = Package_Data[4];
							//前面表示高四位，小数点要错位，结合协议来看
							rs485_recv.rx_data_bit1 = ((Package_Data[3] >> 4) * 10) + (Package_Data[2] & 0x0F);
							rs485_recv.rx_data_bit2 = ((Package_Data[4] >> 4) * 10) + (Package_Data[3] & 0x0F);
							//在显示时第三位固定不显示小数点
							//rs485_recv.rx_data_bit3 = ((Package_Data[4] >> 4) * 10) + (Package_Data[4] & 0x0F);
							rs485_recv.rx_data_bit3 = (Package_Data[4] & 0x0F);
							if(Blink_Flag == 0)
							{
							LED_Segment_ON(rs485_recv.rx_data_bit1,rs485_recv.rx_data_bit2,rs485_recv.rx_data_bit3);
							}
							else if(Blink_Flag == 1)
							{
								Display_Flag = 1 ;
								xSemaphoreGive(xSegBlinkSemHandle);
							}
							break;
							}
						case FUNC_SET_BLINK_or_NOT :
						{
							Display_Flag = 0 ;
							if(Package_Data[2] == LED_SEGMENT_NOT_BLINK)
							{
								Blink_Flag = 0 ;
								LED_Segment_ON(rs485_recv.rx_data_bit1,rs485_recv.rx_data_bit2,rs485_recv.rx_data_bit3);
							}
							if(Package_Data[2] == LED_SEGMENT_BLINK)
							{
								Blink_Flag = 1 ;
								xSemaphoreGive(xSegBlinkSemHandle);
							}
							break ;
						}
						case FUNC_CHANGE_SLAVE_ADDR :
						{
						xSemaphoreGive(xChangeSlaveAddrSemHandle);
						break ;
						}
					}
				}
            else
            {
            	DEBUGINFO("RS485_RECEIVE_fAILED!");
            }
        }
    }
}
