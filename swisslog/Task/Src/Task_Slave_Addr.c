#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "cmsis_os2.h"
#include "Task_rs485.h"
#include "LogDebugInfo.h"

extern osSemaphoreId_t xChangeSlaveAddrSemHandle;
extern uint8_t seted_slave_addr_current ;			//未设置时默认为SLAVE_ADDR_INIT，设置后具有断电保存功能
extern uint8_t seted_slave_addr_last;
extern uint8_t Package_Data[Rx_Buf_Size];
void SlaveAddr_SaveToFlash(uint8_t save_addr);

//修改从机地址
void xChangeSlaveAddr(void *argument)
{
	while(1)
	{
		if(xSemaphoreTake(xChangeSlaveAddrSemHandle, portMAX_DELAY) == pdTRUE)
		{
			seted_slave_addr_last = seted_slave_addr_current ;
			seted_slave_addr_current = Package_Data[2];
			SlaveAddr_SaveToFlash(seted_slave_addr_current);
			DEBUGINFO("Change Slave Addr Succeed!");
			DEBUGINFO("LAST Slave_Addr: %x",seted_slave_addr_last);
			DEBUGINFO("CURRENT Slave_Addr: %x",seted_slave_addr_current);
		}
	}
}
