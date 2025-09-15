/*
 * Task_Rfid.c
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "motor_LD25B60G.h"
#include "queue.h"
#include "LogDebugInfo.h"
#include "adaptor_rfid.h"
#include "adaptor_wifi.h"


#define CARD_NUM_LEN 9 // 9位卡号
#define CHAR_OFFSET 8 //卡号开始位置：跳过"$E000000"（8字节）
#define POS_LEN 5 // 5位位置编号
#define SPEED_OFFSET 5 // 速度偏移量
#define POS_TYPE_OFFSET 7 // 位置类型偏移量

char pcCardNum[10] = {0};
u8 ucRfidDataLen = 0;
u8 ucMotion_msg;
Rfid_Rx_Frame_t frame;


extern QueueHandle_t xMotion_QueueHandle;
extern CarToPlcData CarToPlcData_obj;
extern _CarCheckFlag_obj CarCheckFlagobj;
extern _CarRunStatus_obj CarRunStatus_obj;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;


/**
 * 获取RFID卡号
 *
 * 该函数从提供的数据中提取RFID卡号。它首先验证数据的起始和结束符，接着定位第一个"#"位置，
 * 提取第一段子串（从$到#），并进行重复性校验。如果数据格式正确，函数将卡号提取到cardNum数组中。
 *
 * @param data   RFID数据
 * @param RfidDataLen RFID数据长度
 */
char* pcGetRfidCardNum(char *data, u32 RfidDataLen)
{
  static char pcTempCardNum[10] = {0};

  // 验证起始/结束符
  if (!data || RfidDataLen < 2 || data[0] != '$' || data[RfidDataLen - 1] != '#')
  {
    DEBUGINFO("RFID data error\r\n");
    return '\0'; // 格式错误
  }

  // 定位第一个"#"位置
  char *firstEnd = strchr(data, '#'); 
  if (!firstEnd) return '\0';

  // 提取第一段子串（从$到#）
  u32 segmentLen = firstEnd - data + 1;
  // 验证数据长度有效性
  if (segmentLen >= RfidDataLen) 
  {
    return '\0';
  }
  char* segment1 = malloc(segmentLen + 1);
  if (!segment1) return '\0';

  strncpy(segment1, data, segmentLen);
  segment1[segmentLen] = '\0';

  // 重复性校验：比较两段是否相同
  if (strncmp(data, data + segmentLen, segmentLen) != 0) 
  {
    DEBUGINFO("RFID data error\r\n");
    free(segment1);
    return '\0'; // 两段不一致
  }

  // 提取9位卡号，卡号位置：跳过"$E000000"（8字节）
  if (strlen(segment1) >= (CHAR_OFFSET + CARD_NUM_LEN)) // $ + 8字符偏移 + 9位卡号
  {                                            
    strncpy(pcTempCardNum, segment1 + CHAR_OFFSET, CARD_NUM_LEN);  // 提取9位卡号
    pcTempCardNum[CARD_NUM_LEN] = '\0';      // 显式添加终止符
  }
  else
  {
    DEBUGINFO("Invalid RFID format\r\n");
  }
  free(segment1);
  return pcTempCardNum;
}

// 获取小车当前位置
void vGetCarPosition(char *data)
{
  char pcCurPos[6] = {0};
  u32 ulCurPos = 0;

  // 通过标签编号获取小车当前位置
  strncpy(pcCurPos, data, POS_LEN);
  pcCurPos[POS_LEN] = '\0';
  ulCurPos = strtoul(pcCurPos, NULL, 10); // 将字符串转换为无符号长整型数(10进制)

  // 跟上一次读取的位置比较，如果不相同，则更新当前位置
  if(CarToPlcData_obj.dwCurPos != ulCurPos)
  {
    // 记录上一次位置
    CarToPlcData_obj.dwPrevPos = CarToPlcData_obj.dwCurPos; 
    DEBUGINFO("dwPrevPos:%d\r\n",CarToPlcData_obj.dwPrevPos);
    // 更新当前位置
    CarToPlcData_obj.dwCurPos = ulCurPos; 
    DEBUGINFO("dwCurPos:%d\r\n",CarToPlcData_obj.dwCurPos);
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
  CarToPlcData_obj.bPosType = ucPosType;

  pcPosType[1] = ucPosType/10; //位置类型高10位
  pcPosType[0] = ucPosType%10; //位置类型个位
  DEBUGINFO("vGetTagPosType: int-%d  array-%d,%d\r\n",ucPosType,pcPosType[1],pcPosType[0]);

  //检测到停止标签
  if((pcPosType[1] == 0) || (pcPosType[1] == 1))
  {
    // 小车不在停止状态，且在自动模式下，才发指令停止电机
    if((CarRunStatus_obj.IsCarRunning != CarStop) \
      && (CarCheckFlagobj.ToggleSwtichPosition == ToggleFront)
      && (CarRunStatus_obj.AutoMode == Auto))
    {
      ucMotion_msg = CarStop;
      //if(xQueueSend(xMotion_QueueHandle, &ucMotion_msg, pdMS_TO_TICKS(100)) != pdPASS)
      if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
      {
        DEBUGINFO("vGetTagPosType() send motion msg error\r\n");
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
    // 速度有变化时才更新
    if(CarToPlcData_obj.bSpdMode != pcSpeed[0])
    {
      // 更新预设速度模式
      CarRunStatus_obj.SetSpeed = pcSpeed[0]; 

      ucMotion_msg = CarRunning;
      //if(xQueueSend(xMotion_QueueHandle, &ucMotion_msg, pdMS_TO_TICKS(100)) != pdPASS)
      if(osMessageQueuePut(xMotion_QueueHandle, &ucMotion_msg, 0, pdMS_TO_TICKS(100)) != osOK)
      {
        DEBUGINFO("vGetTagSpeed() send motion msg error\r\n");
      }

    }
    
  }
  
}

//任务入口函数
void vRfidTask(void *argument)
{
  
  //启动DMA接收
  vRfid_Start_GPDMA_Receive();

  while(1) {
    // 等待DMA接收完成信号
    if (osMessageQueueGet(xRfid_Rx_QueueHandle, &frame, NULL, osWaitForever) == osOK) 
    {

      DEBUGINFO("rfid received:%s, len:%d\r\n",frame.data,frame.len);
      // 解析RFID卡号
      strcpy(pcCardNum, pcGetRfidCardNum((char*)frame.data, frame.len));
      //判断卡号非空和cardNum的长度是否大于等于9
      if((strcmp(pcCardNum, "\0") != 0)&&(strlen(pcCardNum) >= CARD_NUM_LEN))
      {

        DEBUGINFO("cardNum:%s,len:%d\r\n",pcCardNum,strlen(pcCardNum));

        //把卡号通过wifi发送到上位机
        vSendToWifiTX((uint8_t *)pcCardNum, strlen(pcCardNum));

        // 获取小车当前位置
        vGetCarPosition(pcCardNum);

        //获取标签位置类型，并判断是否需要停车
        vGetTagPosType(pcCardNum);

        //获取标签速度设置，并设置速度
        vGetTagSpeed(pcCardNum);

      }
      else
      {
        DEBUGINFO("GetRfidCardNum error\r\n");
      }

      // 重启DMA接收(DMA循环模式下，重启后从缓冲区起始地址覆盖写入)
      vRfid_Start_GPDMA_Receive();
    }

    

  }

}
