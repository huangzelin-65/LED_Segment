/*
 * Task_Wifi.c
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Wifi.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "adaptor_mqtt.h"

extern osMessageQueueId_t xWifi_Parse_QueueHandle;

/* WiFi管理任务入口函数 */
void vWifiManagerTask(void *argument)
{
  DEBUGINFO("vWifiManagerTask\r\n");
  osDelay(pdMS_TO_TICKS(3000));//wifi模块上电需要等待3秒才可以发送命令
  Wifi_ConnectStart();
  while (1)
  {
    Wifi_ConnectProcess();
    if(Wifi_IsChanged())//通知mqtt任务，wifi状态发送变化
    {
        Mqtt_SendMsg(MQTT_MSG_WIFI_CHANGE);
    }
	  osDelay(pdMS_TO_TICKS(100));
  }
}
    

/* wifi接收任务入口函数 */
void vWifiReceiveTask(void *argument)
{
  DEBUGINFO("vWifiReceiveTask\r\n");
  uint8_t read_buffer[WIFI_RX_BUF_SIZE];
  WifiParseData_t *wifi_data = NULL;
  Wifi_ReceiveInit();//启动串口空闲中断，DMA接收数据
  while (1)
  {
      //等待接收串口的数据
      if(xQueueReceive(xWifi_Parse_QueueHandle, &wifi_data, portMAX_DELAY) == pdTRUE) {
        int dataLength = 0;
        bool parse_rbuf = false;

        memset(read_buffer,0,sizeof(read_buffer));

        //当last_read_id与Size相等时，代表没有数据更新，此时不解析数据
        if(wifi_data->last_read_id < wifi_data->size)
        {
          for(int i = wifi_data->last_read_id;i < wifi_data->size;i++)
          {
            read_buffer[i - wifi_data->last_read_id] = wifi_data->rx_buffer[i];
            dataLength++;
          }
          DEBUGINFO("read_buffer 1:%s\n",read_buffer);
          parse_rbuf = true;
        }
        else if(wifi_data->last_read_id > wifi_data->size)
        {
          int j = 0;
          for(int i = wifi_data->last_read_id;i < sizeof(wifi_data->rx_buffer);i++)
          {
            read_buffer[i - wifi_data->last_read_id] = wifi_data->rx_buffer[i];
            j++;
            dataLength++;
          }
          for(int i = 0;i < wifi_data->size;i++)
          {
            read_buffer[j + i] = wifi_data->rx_buffer[i];
            dataLength++;
          } 
          DEBUGINFO("read_buffer 2:%s\n",read_buffer);
          parse_rbuf = true;      
        }

        if(parse_rbuf)
        {
          Wifi_ConnectAck(read_buffer,dataLength);
          Mqtt_ParseData(read_buffer,dataLength);
        }  
        vPortFree(wifi_data);       
      }
  } 
}


