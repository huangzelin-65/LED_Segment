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


#define CARD_NUM_LEN 9 // 9位卡号
#define CHAR_OFFSET 8 //卡号开始位置：跳过"$E000000"（8字节）
#define POS_LEN 5 // 5位位置编号
#define SPEED_OFFSET 5 // 速度偏移量
#define POS_TYPE_OFFSET 7 // 位置类型偏移量

char pcCardNum[10] = {0};
u8 ucRfidDataLen = 0;
u8 ucMotion_msg;
uint8_t ucCarRfid_Rx_Buffer[2][CAR_RFID_RX_BUF_SIZE];
uint8_t ucCarRfid_current_buf_idx = 0;  // 当前使用的缓冲区索引

extern osMessageQueueId_t xMotion_QueueHandle;
extern CarToServerData_t CarToServerData;
extern CarStatus_t CarStatus;
extern osMessageQueueId_t xRfid_Rx_QueueHandle;
extern osSemaphoreId_t xCarRfidRxSemHandle;

//获取9位卡号
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
  if(CarToServerData.dwCurPos != ulCurPos)
  {
    // 记录上一次位置
    CarToServerData.dwPrevPos = CarToServerData.dwCurPos; 
    DEBUGINFO("dwPrevPos:%lu\r\n",CarToServerData.dwPrevPos);
    // 更新当前位置
    CarToServerData.dwCurPos = ulCurPos; 
    DEBUGINFO("dwCurPos:%lu\r\n",CarToServerData.dwCurPos);

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
    // 速度有变化时才更新
    if(CarStatus.xRealSpeed != pcSpeed[0])
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
}


//任务入口函数
void vCarRfidTask(void *argument)
{
  uint32_t ucReciveLen = 0;
  
  
  //启动DMA接收
  vCarRfid_Start_DMA_Receive(ucCarRfid_Rx_Buffer[ucCarRfid_current_buf_idx]);

  while(1) {
    // 等待DMA接收完成信号
    if (osSemaphoreAcquire(xCarRfidRxSemHandle, osWaitForever) == osOK)
    {
      ucReciveLen = ulCarRfid_Get_DMA_Receive_Len();
      DEBUGINFO("ucCarRfid_current_buf_idx:%d\r\n",ucCarRfid_current_buf_idx);
      DEBUGINFO("rfid received len:%d,data:%s\r\n",ucReciveLen,ucCarRfid_Rx_Buffer[ucCarRfid_current_buf_idx]);
      //DEBUGINFO("rfid received len:%d\r\n",ucReciveLen);
      //vPrint_Array(ucCarRfid_Rx_Buffer[ucCarRfid_current_buf_idx], ucReciveLen);
      
      //uint8_t* temp_buffer = ucCarRfid_Rx_Buffer[ucCarRfid_current_buf_idx];

      // 解析RFID卡号
      char* pResult = pcGetRfidCardNum((char*)ucCarRfid_Rx_Buffer[ucCarRfid_current_buf_idx], ucReciveLen);

      // 切换缓冲区并重启接收
      ucCarRfid_current_buf_idx ^= 1;
      vCarRfid_Start_DMA_Receive(ucCarRfid_Rx_Buffer[ucCarRfid_current_buf_idx]);

      //判断卡号非空
      if (pResult != NULL)
      {
        strcpy(pcCardNum, pResult);

        //cardNum的长度是否等于9
        if(strlen(pcCardNum) == CARD_NUM_LEN)
        {
          DEBUGINFO("cardNum:%s,len:%d\r\n",pcCardNum,strlen(pcCardNum));

          //把卡号通过wifi发送到上位机
          //vSendToWifiTX((uint8_t *)pcCardNum, strlen(pcCardNum));

          // 获取小车当前位置
          vGetCarPosition(pcCardNum);

          //获取标签位置类型，并判断是否需要停车
          vGetTagPosType(pcCardNum);

          //获取标签速度设置，并设置速度
          vGetTagSpeed(pcCardNum);
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
