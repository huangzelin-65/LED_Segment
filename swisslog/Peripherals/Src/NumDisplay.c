#include "main.h"
#include "Calculate.h"
#include "NumDisplay.h"
#include "adaptor_box.h"
#include "LogDebugInfo.h"

	
//extern uint16_t carIDNum;

static uint8_t setCarID[] = {0x01,0x06,0x02,0x00,0x01,0x0,0x02,0xA2,0x67};
static uint8_t changLedBlue[]=		{0x01,0x06,0x2,0x00,0x02,0x00,0x00,0xD3,0xA6};
static uint8_t changLedBlueShan[]=	{0x01,0x06,0x2,0x00,0x02,0x00,0x01,0x12,0x66};
//static uint8_t changLedBlueRedShan[]= {0x01,0x06,0x2,0x00,0x02,0x00,0x02,0x52,0x67};


void NumDisp_SetNumber(uint16_t num)
{
	setCarID[5] = (num >> 8);
	setCarID[6] = num;
	setCarID[7] = (CRC16(setCarID,7) >> 8);
	setCarID[8] = (CRC16(setCarID,7));
	vSendToNumDisp(setCarID,sizeof(setCarID));//发送小车号
  DEBUGINFO("setCarID:%d\r\n",num);
  DEBUGINFO("setCarID[5]:%X,setCarID[5]:%X\r\n",setCarID[5],setCarID[6]);
}


void NumDisp_BlueShan(void)
{
	vSendToNumDisp(changLedBlueShan,sizeof(changLedBlueShan));
}

void NumDisp_Blue(void)
{
	vSendToNumDisp(changLedBlue,sizeof(changLedBlue));
}



