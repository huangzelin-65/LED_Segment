/*
 * Task_Rfid.c
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */
#include <Common.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdlib.h>
#include "Calculate.h"
#include "motor_LD25B60G.h"
#include "queue.h"
#include "LogDebugInfo.h"
#include "adaptor_rfid.h"
#include "adaptor_wifi.h"
#include "DwinHMI.h"
#include"Task_CarRfid.h"

#ifdef ZHONGNENG_RFID
#include "ZhongnengRfidReader.h"
#include "semphr.h"
#endif

char pcCardNum[10] = {0};
u8 ucRfidDataLen = 0;
u8 ucMotion_msg;
uint8_t ucCarRfid_Rx_Buffer[CAR_RFID_RX_BUF_SIZE];
uint32_t ucReciveLen = 0;         //DMA接收数据长度
uint32_t package_start_idx = 0 ;  //DMA接收数据包起始位置


extern osMessageQueueId_t xMotion_QueueHandle;
extern CarToServerData_t CarToServerData;
extern CarStatus_t CarStatus;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;
extern osSemaphoreId_t xCarRfidRxSemHandle;

//获取9位卡号

#ifdef ZHONGNENG_RFID
char* pcGetRfidCardNum_ZHONGNENG(uint8_t *data, u32 RfidDataLen)
{
    static char pcTempCardNum[10] = {0};
    memset(pcTempCardNum, 0, sizeof(pcTempCardNum));
    // 长度校验，二进制数据包至少需要17字节
    if (!data || RfidDataLen < Rfid_Rx_Package_LEN)
    {
        DEBUGINFO("RFID invalid parameters\r\n");
        return NULL;
    }

   //  验证起始/结束符
    if (data[0] != 0x1D || data[RfidDataLen - 1] != 0xB2)
    {
        DEBUGINFO("RFID data format error\r\n");
        return NULL;
    }

    //判断密钥是否正确
    if(data[5] == 0x03)
    {
    	DEBUGINFO("Invalid password\r\n");
    	return NULL;
    }

    //判断标签是否丢失
    if(data[5] == 0x04)
    {
    	DEBUGINFO("Tag loss\r\n");
    	return NULL;
    }
    // 异或校验
    uint8_t i = 0;
    uint8_t XOR_check_bit = 0;
    for(i = 1; i <= Rfid_Rx_Package_LEN - 3; i++)
    {
        XOR_check_bit ^= data[i];
    }
    if(XOR_check_bit != data[Rfid_Rx_Package_LEN - 2])
    {
        DEBUGINFO("RFID data mismatch\r\n");
        return NULL;
    }

    // 提取9位卡号
    if (CHAR_OFFSET + CARD_NUM_LEN > RfidDataLen - 2)
    {
        DEBUGINFO("Invalid RFID format, insufficient length\r\n");
        return NULL;
    }
    else
    {
        bin_id_to_num_str(&data[CHAR_OFFSET], CARD_NUM_LEN, pcTempCardNum);
        pcTempCardNum[CARD_NUM_LEN] = '\0';
        return pcTempCardNum;
    }
}
#else
char* pcGetRfidCardNum(char *data, u32 RfidDataLen)
{
    static char pcTempCardNum[10] = {0}; // 存储9位卡号加终止符
    // 重置临时缓冲区
    memset(pcTempCardNum, 0, sizeof(pcTempCardNum));

    // 验证输入参数
    if (!data || RfidDataLen < 2)
    {
        DEBUGINFO("RFID invalid parameters\r\n");
        return NULL; // 返回空指针而非字符
    }

    // 验证起始/结束符
    if (data[0] != '$' || data[RfidDataLen - 1] != '#')
    {
        DEBUGINFO("RFID data format error\r\n");
        return NULL;
    }

    // 定位第一个"#"位置
    char *firstEnd = strchr(data, '#'); 
    if (!firstEnd) return NULL;

    // 提取第一段子串（从$到#）
    u32 segmentLen = firstEnd - data + 1;
    // 验证数据长度有效性
    if (segmentLen >= RfidDataLen || segmentLen * 2 != RfidDataLen) 
    {
        DEBUGINFO("RFID segment length error\r\n");
        return NULL;
    }

    // 重复性校验：比较两段是否相同
    if (memcmp(data, data + segmentLen, segmentLen) != 0) 
    {
        DEBUGINFO("RFID data mismatch\r\n");
        return NULL; // 两段不一致
    }

    // 提取9位卡号，确保不会越界
    // 假设CHAR_OFFSET是8（"$E000000"的长度），CARD_NUM_LEN是9
    if (segmentLen >= (CHAR_OFFSET + CARD_NUM_LEN + 1)) // +1确保有终止符空间
    {                                            
        // 修正：使用data而不是segment1，因为我们已经验证了数据有效性
        strncpy(pcTempCardNum, data + CHAR_OFFSET, CARD_NUM_LEN);
        pcTempCardNum[CARD_NUM_LEN] = '\0'; // 确保终止符
        return pcTempCardNum;
    }
    else
    {
        DEBUGINFO("Invalid RFID format, insufficient length\r\n");
        return NULL;
    }
}
#endif

// 获取小车当前位置
void vGetCarPosition(char *data)
{
  char pcCurPos[6] = {0};
  uint32_t ulCurPos = 0;

  // 通过标签编号获取小车当前位置
  strncpy(pcCurPos, data, POS_LEN);
  pcCurPos[POS_LEN] = '\0';
  ulCurPos = strtoul(pcCurPos, NULL, 10); // 将字符串转换为无符号长整型数(10进制)

  // 跟上一次读取的位置比较，如果不相同，则更新当前位置
  if(CarStatus.dwCurPos != ulCurPos)
  {
    // 记录上一次位置
    CarStatus.dwPrevPos = CarStatus.dwCurPos; 
    DEBUGINFO("dwPrevPos:%lu\r\n",CarStatus.dwPrevPos);
    // 更新当前位置
    CarStatus.dwCurPos = ulCurPos; 
    DEBUGINFO("dwCurPos:%lu\r\n",CarStatus.dwCurPos);

    HMI_Update_CurLocationId_Req(ulCurPos);
  }
}

//获取标签位置类型
void vGetTagPosType(char *data)
{
  u8 pcPosType[2] = {0};
  u8 ucPosType = 0;

  //把(data + POS_TYPE_OFFSET)作为十位数，(data + POS_TYPE_OFFSET + 1) 作为个位数，转换为一个无符号整型数
  ucPosType = substring_to_uint(data, POS_TYPE_OFFSET, 2);

  // 更新位置类型
  CarToServerData.ucPosType = ucPosType;

  pcPosType[1] = ucPosType/10; //位置类型高10位
  pcPosType[0] = ucPosType%10; //位置类型个位
  DEBUGINFO("vGetTagPosType: int-%d  array-%d,%d\r\n",ucPosType,pcPosType[1],pcPosType[0]);

  //检测到停止标签
  if((pcPosType[1] == 0) || (pcPosType[1] == 1))
  {
    // 小车不在停止状态，且在自动模式下，才发指令停止电机
    if((CarStatus.xIsCarRunning != CarStop) \
      && (CarStatus.ToggleSwtichPosition == ToggleFront)
      && (CarStatus.xAutoMode == Auto))
    {
      // 电机停止原因
      CarStatus.xMotorStopReason = ByStopTag;
      DEBUGINFO("MotorStopReason: ByStopTag\r\n");

      ucMotion_msg = CarStop;
      if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
      {
        DEBUGINFO("send motion msg error\r\n");
      }
    }

  }
}

//获取标签速度设置
void vGetTagSpeed(char *data)
{
  u8 pcSpeed[2] = {0};
  u8 ucSpeed = 0;

  //把(data + POS_TYPE_OFFSET)作为十位数，(data + POS_TYPE_OFFSET + 1) 作为个位数，转换为一个无符号整型数
  ucSpeed = substring_to_uint(data, SPEED_OFFSET, 2);

  pcSpeed[1] = ucSpeed/10; //速度高10位
  pcSpeed[0] = ucSpeed%10; //速度个位
  DEBUGINFO("vGetTagSpeed:int-%d  array-%d,%d\r\n",ucSpeed,pcSpeed[1],pcSpeed[0]);

  // 判断目标速度在1-3之间
  if((pcSpeed[0]>=1) && (pcSpeed[0]<=3))
  {
    // 更新预设速度模式
    CarStatus.xSetSpeed = pcSpeed[0]; 

    ucMotion_msg = CarRunning;
    if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
    {
      DEBUGINFO("send motion msg error\r\n");
    }
  }
}


//任务入口函数
void vCarRfidTask(void *argument)
{
  char pcPreviousCardNum[10] = {0};
  static uint8_t temp_continuous_buf[CAR_RFID_RX_BUF_SIZE] = {0};
#ifdef ZHONGNENG_RFID
  CarRfid_Init_With_Retry();		//调用读卡器的初始化函数，失败时重新初始化
#else
  //启动DMA接收
  vCarRfid_Start_DMA_Receive(ucCarRfid_Rx_Buffer);
#endif
  while(1)
  {
    if (osSemaphoreAcquire(xCarRfidRxSemHandle, osWaitForever) == osOK)
    {
      ucReciveLen = ulCarRfid_Get_DMA_Receive_Len(&package_start_idx);
      ucCarRfid_Rx_Buffer_Wrap_process(ucCarRfid_Rx_Buffer,package_start_idx,
    		  	  	  	  	  	  	  	  ucReciveLen,temp_continuous_buf);
#ifdef ZHONGNENG_RFID
      DEBUGINFO("rfid received len:%d\r\n",ucReciveLen);
      //vPrint_Array(temp_continuous_buf,ucReciveLen);
      char* pResult = pcGetRfidCardNum_ZHONGNENG(temp_continuous_buf, ucReciveLen);
#else
      DEBUGINFO("rfid received len:%d,data:%s\r\n",
    		  	  ucReciveLen,(u8* )&ucCarRfid_Rx_Buffer[package_start_idx]);

              // 解析RFID卡号
     char* pResult = pcGetRfidCardNum((char*)temp_continuous_buf, ucReciveLen);
#endif
      if (pResult != NULL)
      {
        strcpy(pcCardNum, pResult);

        //cardNum的长度是否等于9, 且cardNum是否和上一次读取的cardNum不一样
        if((strlen(pcCardNum) == CARD_NUM_LEN) && (strcmp(pcCardNum, pcPreviousCardNum) != 0))
        {
          DEBUGINFO("cardNum:%s,len:%d\r\n",pcCardNum,strlen(pcCardNum));

          strcpy(pcPreviousCardNum, pcCardNum);

          //把卡号通过wifi发送到上位机
          //vSendToWifiTX((uint8_t *)pcCardNum, strlen(pcCardNum));

          // 获取小车当前位置
          vGetCarPosition(pcCardNum);

          //获取标签位置类型，并判断是否需要停车
          vGetTagPosType(pcCardNum);

          //获取标签速度设置，并设置速度
          vGetTagSpeed(pcCardNum);

          //上报小车状态
          Robot_Event(); 
        }
      }
      else
      {
        // 处理错误情况
        DEBUGINFO("Failed to get RFID card number\r\n");
        // 可以清零pcCardNum或者做其他错误处理
        memset(pcCardNum, 0, sizeof(pcCardNum));
      }
    }
    
  }

}
