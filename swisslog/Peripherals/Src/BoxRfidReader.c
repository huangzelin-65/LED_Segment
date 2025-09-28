#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "LogDebugInfo.h"
#include "BoxRfidReader.h"
#include <string.h>
#include "Calculate.h"
#include "adaptor_rfid.h"


#define WAIT_TIME			500
#define TX_READ_BUF_LEN 	64

static uint8_t g_bReceAA=0; 			//开始接受字符标志
static uint8_t g_cReceNum;     		//接收字节数
static uint8_t g_cReceBuf[TX_READ_BUF_LEN]; //接收数据buf
static uint16_t g_cCommand;     		//命令码
static uint8_t g_bReceOk=0; 			//接收正确标志

static uint8_t CardID[4];

static uint8_t uartRecMode = 0;//0:cmd mode,1:auto read mode


/*
处理串口接收的字符
cmd mode
*/
static void vBoxRfid_DealReciveCmd(uint8_t etx)
{
	uint8_t len;
	uint8_t verify = 0;

	//DEBUGINFO("etx = %X\r\n",etx);

	if (g_bReceAA)
	{  
		g_bReceAA = 0;
		switch (etx)
		{	 
			case 0x00:
				g_cReceBuf[g_cReceNum] = 0xAA;
				g_cReceNum++;
			break;
			
			case 0xBB:
				g_cReceNum = 0;
			break;
			
			default:
				g_cReceBuf[g_cReceNum++] = etx;
			break;
		}
	}else{
		//接收数据
		switch (etx)
		{	 
			case 0xAA:
				g_bReceAA = 1;
			default:
				g_cReceBuf[g_cReceNum++] = etx;
			break;
		}
	}
	
	len = (g_cReceBuf[1]<<8) + g_cReceBuf[0];
	if ((g_cReceNum == len + 2) &&  len != 0 ) 
	{	
		for (int j=2; j<g_cReceNum -1; j++)
			verify ^= g_cReceBuf[j];
		if (verify==g_cReceBuf[g_cReceNum-1])
		{   
			g_bReceOk  = 1;
			g_cCommand = (g_cReceBuf[5]<<8) + g_cReceBuf[4];
			g_bReceAA  = 0;
		}
	}

	if (g_cReceNum >= sizeof(g_cReceBuf))
	  g_cReceNum=0;   
}

/*
处理串口接收的字符
auto update
for example; header:02
			 len:	10
			 data:  00 01 02 03 ... 0f
			 chk    len+data
			 end:   03
*/
/**
 * 本函数用于处理接收到的数据，包括接收数据包的头部、数据长度、数据内容和校验码。
 *
 * @param etx 接收到的数据字节
 * @note
 *   - 如果接收到数据包头部（0x02），则重置接收状态和接收缓冲区。
 *   - 如果接收到的数据长度与预期长度匹配，则进行数据校验。
 *   - 如果数据校验通过，则设置接收完成标志。
 *   - 如果接收缓冲区满，则清空缓冲区。
 */
static void vBoxRfid_DealReciveData(uint8_t etx)
{
	uint8_t len;
	uint8_t verify = 0;

	//DEBUGINFO("etx = %X\r\n",etx);

	if (g_bReceAA)
	{  
		g_cReceBuf[g_cReceNum++] = etx;
	}else{
		if(etx == 0x02){
			g_bReceOk  = 0;
			g_bReceAA = 1;
			g_cReceNum = 0;
			return;
		}
	}
	
	len = g_cReceBuf[0];
	if ((g_cReceNum == len + 3) &&  len != 0 ) 
	{	
		for (int j=0; j<(len + 1); j++)
			verify ^= g_cReceBuf[j];
		if (verify==g_cReceBuf[g_cReceNum-2])
		{   
			g_bReceOk  = 1;
			g_bReceAA  = 0;
		}
	}

	if (g_cReceNum >= sizeof(g_cReceBuf))
	  g_cReceNum=0;   
}


/**
串口接收中断
*/
void vBoxRfid_ReceiveDataHandler(uint8_t* ucReceiveData,uint32_t ucReceiveLen)
{
	DEBUGINFO("uartRecMode = %d\r\n",uartRecMode);
	for(int i=0; i<ucReceiveLen; i++)
	{
		if(uartRecMode == 1){
			vBoxRfid_DealReciveData(ucReceiveData[i]);
		}else{
			vBoxRfid_DealReciveCmd(ucReceiveData[i]);
		}
	}
}

uint8_t M5_ReadWriteMode(void)
{
	uint32_t i;
	uint8_t sendBuf[16];
	
	sendBuf[0]=0xAA;//STX1;
	sendBuf[1]=0XBB;//STX2;
	sendBuf[2]=0x06;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x01;
	sendBuf[7]=0x06;
	sendBuf[8]=0x01;
	sendBuf[9]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8];

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,10);
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		//i--;
		osDelay(1);
	}
	g_bReceOk=0; 
	  
	return 1;	
}

/*
 *	set read mode 8,update the result of the <block>
 *	block; the block update
 *	passwd
 *	bitNum ; max is 16
 *
 */
/**
 * 只读模式八:
 * 		读出指定块号的16字节数据模式，设置一次永久有效。
 * 例如
 * 发送：AA BB 0 D 00 00 00 01 08 04 ff ff ff ff ff ff 10 1D
 * 返回：AA BB 06 00 00 00 01 08 00 09
 * 
 * @param block   要读取的块号。
 * @param passwd  6字节的密码。
 * @param bitNum  位数。
 *
 * @return 读取结果，0xFF表示失败，其他值表示成功。
 */
uint8_t M5_ReadOnlyMode8(uint8_t block,uint8_t passwd[6],uint8_t bitNum)
{
	uint32_t i;
	uint8_t sendBuf[17];

	DEBUGINFO("\r\n");
	
	sendBuf[0]=0xAA;//STX1;
	sendBuf[1]=0XBB;//STX2;
	sendBuf[2]=0x0D;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x01;
	sendBuf[7]=0x08;
	sendBuf[8]=block;

	sendBuf[9]=passwd[0];
	sendBuf[10]=passwd[1];
	sendBuf[11]=passwd[2];
	sendBuf[12]=passwd[3];
	sendBuf[13]=passwd[4];
	sendBuf[14]=passwd[5];

	sendBuf[15]=bitNum;

	sendBuf[16]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8]^
		sendBuf[9]^sendBuf[10]^sendBuf[11]^sendBuf[12]^sendBuf[13]^
		sendBuf[14]^sendBuf[15];

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,17);
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			DEBUGINFO("g_bReceOk==1\r\n");
			g_bReceOk=0;
			uartRecMode = 1;
			return g_cReceBuf[6];
		}
		//i--;
		osDelay(1);
	}
	g_bReceOk=0; 
	
	return 1;	
}


uint8_t M5_Beep(uint16_t time)
{
	uint32_t i;
	uint8_t sendBuf[16];
	
	sendBuf[0]=0xAA;//STX1;
	sendBuf[1]=0XBB;//STX2;
	sendBuf[2]=0x07;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x06;
	sendBuf[7]=0x01;
	sendBuf[8]=time&0xFF;
	sendBuf[9]=(time>>8)&0xFF;
	sendBuf[10]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8]^sendBuf[9];

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,11);
	
	i=WAIT_TIME;
	
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		i--;
		osDelay(1);
	}
	g_bReceOk=0; 
	  
	return 1;	
}
//寻卡操作
//返回0 成功 ，其它失败
uint8_t M5_FindCard(uint8_t mode) 
{
	uint32_t i;
	uint8_t sendBuf[16];
	
	sendBuf[0]=0xAA;//STX1;
	sendBuf[1]=0XBB;//STX2;
	sendBuf[2]=0x06;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x01;
	sendBuf[7]=0x02;
	sendBuf[8]=mode;
	sendBuf[9]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8];

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,10);
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		osDelay(1);
		i--;
	}
	g_bReceOk=0; 
	  
    return 1;		
}

//防冲突操作
uint8_t M5_Anticoll(void) 
{
	uint32_t i;
	uint8_t sendBuf[16];
	
	sendBuf[0]=0xAA;
	sendBuf[1]=0XBB;
	sendBuf[2]=0x06;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;
	sendBuf[6]=0x02;
	sendBuf[7]=0x02;
	sendBuf[8]=0x04;
	sendBuf[9]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8];

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,10);
	
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			if(g_cReceBuf[6]==0x00)
			{
				CardID[0]=g_cReceBuf[7];
				CardID[1]=g_cReceBuf[8];
				CardID[2]=g_cReceBuf[9];
				CardID[3]=g_cReceBuf[10];
				//DEBUGINFO("get CardID: %x %x %x %x\n",CardID[0],CardID[1],CardID[2],CardID[3]);
			}
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		osDelay(1);		
		i--;		
	}
		g_bReceOk=0; 
    return 1;		
}


//选卡操作
uint8_t M5_SelectCard(void) 
{
	uint32_t i,SendLen;
	uint8_t sendBuf[24];
	
	sendBuf[0]=0xAA;
	sendBuf[1]=0XBB;
	sendBuf[2]=0x09;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;
	sendBuf[6]=0x03;
	sendBuf[7]=0x02;
	sendBuf[8]=CardID[0];
	sendBuf[9]=CardID[1];
	sendBuf[10]=CardID[2];
	sendBuf[11]=CardID[3];
	sendBuf[12]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8]^sendBuf[9]^sendBuf[10]^sendBuf[11];
	SendLen=13;
	if(sendBuf[12]==0xAA)
	{
		sendBuf[13]=0x00; 
		SendLen=14;
	}	

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	
	vSendToBoxRfid(sendBuf,SendLen);
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		osDelay(1); 	
		i--;				
	}
	g_bReceOk=0;

    return 1;		
}


//认证操作
uint8_t M5_Auth(uint8_t mode,uint8_t block,uint8_t *BufferPassword) 
{
	uint32_t i,y,SendLen;
	uint8_t BCC=0;
	uint8_t sendBuf[48];
	
	sendBuf[0]=0xAA;
	sendBuf[1]=0XBB;//STX;
	sendBuf[2]=0x0D;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;
	sendBuf[6]=0x07;
	sendBuf[7]=0x02;
	sendBuf[8]=mode;   //模式
	sendBuf[9]=block;  //块号
	BCC=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7]^sendBuf[8]^sendBuf[9];
	y=0;
	for(i=0;i<6;i++)
	{
		sendBuf[10+i+y]=BufferPassword[i];
		BCC^= sendBuf[10+i+y];
		if(sendBuf[10+i+y]==0xAA)
		{	
			y++;
			sendBuf[10+i+y]=0x00;  
		}	
	}	
	sendBuf[10+i+y]=BCC;	

	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	
	SendLen=10+i+y+1;
	if(sendBuf[10+i+y]==0xAA)
	{
		sendBuf[10+i+y+1]=0x00; 
		SendLen=10+i+y+2;
	}	
	vSendToBoxRfid(sendBuf,SendLen);
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		osDelay(1); 
		i--;				
	}
	g_bReceOk=0; 
	
	return 1;		
}

//体眠
uint8_t M5_Halt(void) 
{
	uint32_t i;
	uint8_t sendBuf[10];
	//DEBUGINFO("M5_Halt\n");
	sendBuf[0]=0xAA;//STX1;
	sendBuf[1]=0XBB;//STX2;
	sendBuf[2]=0x05;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x04;
	sendBuf[7]=0x02;
	sendBuf[8]=sendBuf[4]^sendBuf[5]^sendBuf[6]^sendBuf[7];


	g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,9);
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];	
		}
		osDelay(1);		
		i--;				
	}
	g_bReceOk=0;
	return 1;		
}

//readBuf len = 16
//don't input 0xaa
uint8_t M5_Easy_Read(uint8_t block,uint8_t pswdType,uint8_t *BufferPassword,uint8_t *readBuf) 
{
	uint32_t i;
	uint8_t sendBuf[18];
	uint8_t sendLen=17;
	sendBuf[0]=0xAA;
	sendBuf[1]=0XBB;
	sendBuf[2]=0x0D;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x08;
	sendBuf[7]=0x06;
	sendBuf[8]=pswdType;
	sendBuf[9]=block;
	memcpy(sendBuf+10,BufferPassword,6);

	uint8_t temp = sendBuf[4];
	for(i=5;i<16;i++){
		temp = temp ^sendBuf[i];
	}
	sendBuf[16]=temp;
	if(sendBuf[16] == 0xaa){
		sendBuf[17]= 0x00;
		sendLen = 18;
		sendBuf[2] += 1;
	}

	//g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,sendLen);
	
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			memcpy(readBuf,g_cReceBuf+7,16);
			return g_cReceBuf[6];
		}
		osDelay(1);			
	}
	g_bReceOk=0;
	i--;		
	return 1;		
}

//writeBuf len = 16
//don't input 0xaa
uint8_t M5_Easy_Write(uint8_t block,uint8_t pswdType,uint8_t *BufferPassword,uint8_t *writeBuf) 
{
	uint32_t i;
	uint8_t sendBuf[34];
	uint8_t sendLen=33;
	sendBuf[0]=0xAA;
	sendBuf[1]=0XBB;
	sendBuf[2]=0x1D;
	sendBuf[3]=0x00;
	sendBuf[4]=0x00;
	sendBuf[5]=0x00;		
	sendBuf[6]=0x09;
	sendBuf[7]=0x06;
	sendBuf[8]=pswdType;
	sendBuf[9]=block;
	memcpy(sendBuf+10,BufferPassword,6);
	memcpy(sendBuf+16,writeBuf,16);

	uint8_t temp = sendBuf[4];
	for(i=5;i<32;i++){
		temp = temp ^sendBuf[i];
	}
	sendBuf[32]=temp;
	if(sendBuf[32] == 0xaa){
		sendBuf[33]= 0x00;
		sendLen = 34;
		sendBuf[2] += 1;
	}

	//g_cReceBuf[6]=0xFF;
	g_bReceOk=0;
	vSendToBoxRfid(sendBuf,sendLen);
	
	i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			return g_cReceBuf[6];
		}
		osDelay(1);		
		i--;				
	}
	g_bReceOk=0;
	return 1;		
}

//readBuf len = 16
//don't input 0xaa
uint8_t M5_Easy_ChangePswd(uint8_t block,uint8_t pswdType,uint8_t *oldPassword,uint8_t *newPasswd) 
{
	uint8_t writeBuf[16]={0,0,0,0,0,0,
					0xff,0x07,0x80,0x69,
					0xff,0xff,0xff,0xff,0xff,0xff};
	memcpy(writeBuf,newPasswd,6);
	uint8_t readBk = (block/4)*4+3;	
	return M5_Easy_Write(readBk,pswdType,oldPassword,writeBuf);	
}

/**
 * 等待读取卡片数据。
 *
 * 该函数会阻塞直到读取到卡片数据或超时。
 *
 * @param readBuf 读取到的卡片数据存储缓冲区指针。
 * @return 读取成功返回1，超时返回1。
 */
uint8_t M5_WaitCard(uint8_t *readBuf) 
{
	g_bReceOk=0;	
	uint32_t i=WAIT_TIME;
	while(i)
	{
		if(g_bReceOk==1)
		{
			g_bReceOk=0;
			memcpy(readBuf,g_cReceBuf+1,16);
			return 1;
		}
		osDelay(1);			
	}
	g_bReceOk=0;
		
	return 1;		
}



