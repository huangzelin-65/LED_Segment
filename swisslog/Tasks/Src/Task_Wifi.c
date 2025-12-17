#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Wifi.h"
#include "LogDebugInfo.h"
#include "adaptor_wifi.h"
// #include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "adaptor_mqtt.h"
#include "Calculate.h"

// #define WIFI_OTA

extern osMessageQueueId_t xWifi_Parse_QueueHandle;

/* WiFi管理任务入口函数 */
void vWifiManagerTask(void *argument)
{
  int wifi_get_rssi_cnt = 0; 
  int wifi_get_ip_cnt = 0;  
  DEBUGINFO("vWifiManagerTask\r\n");
  Wifi_Init();
  #ifdef MQTT_WIFI
    #ifdef WIFI_OTA
    Wifi_OtaProcess();
    #else
    Wifi_ConnectStart(); 
    #endif
  #endif
  while (1)
  {
    Wifi_ConnectProcess();
    if(Wifi_IsChanged())//通知mqtt任务，wifi状态发送变化
    {
      if(Wifi_IsConnected())
      {
        Mqtt_SendMsg(MQTT_MSG_START,NULL);
      }
    }
    //增加wifi模块强度查询
    if(Wifi_IsConnected())
    {
      if(waitforperiod(&wifi_get_rssi_cnt,100))//10秒获取一次wifi信号强度
      {
        // Wifi_SendATCmd("AT+RSSI",2000);
        DEBUGINFO("wifi status connect_state:%d rssi:%d ip:%s\n",wifi_status.connect_state,wifi_status.rssi,wifi_status.ip);
      }
      if(waitforperiod(&wifi_get_ip_cnt,50))//5秒获取一次获取ip地址,如果没有获取
      {
        if(wifi_status.ip[0] == 0)
        {
          Wifi_SendATCmd("AT+LIP",2000);
        }
      }   
    }    
	  osDelay(pdMS_TO_TICKS(100));
  }
}
    

/* wifi接收任务入口函数 */
void vWifiReceiveTask(void *argument)
{
  DEBUGINFO("vWifiReceiveTask\r\n");
  uint8_t read_buffer[WIFI_RX_BUF_SIZE];
  uint8_t printf_buffer[LOG_LENGTH_LONG];    
  WifiParseData_t *wifi_data = NULL;
  Wifi_ReceiveInit();//启动串口空闲中断，DMA接收数据
  while (1)
  {
    //等待接收串口的数据
    if(xQueueReceive(xWifi_Parse_QueueHandle, &wifi_data, portMAX_DELAY) == pdTRUE) {
      // DEBUGINFO("wifi_data %p",wifi_data);
      int dataLength = 0;
      bool parse_rbuf = false;
      memset(read_buffer,0,sizeof(read_buffer));
      DEBUGINFO("last_read_id:%d size:%d rx_buffer:%p\n",wifi_data->last_read_id,wifi_data->size,wifi_data->rx_buffer);
      //当last_read_id与Size相等时，代表没有数据更新，此时不解析数据
      if(wifi_data->last_read_id < wifi_data->size)
      {
        for(int i = wifi_data->last_read_id;i < wifi_data->size;i++)
        {
          read_buffer[i - wifi_data->last_read_id] = wifi_data->rx_buffer[i];
          dataLength++;
        }
        // DEBUGINFO("read_buffer 1:%s dataLength:%d\n",read_buffer,dataLength);
        parse_rbuf = true;
      }
      else if(wifi_data->last_read_id > wifi_data->size)
      {
        int j = 0;
        for(int i = wifi_data->last_read_id;i < WIFI_RX_BUF_SIZE;i++)
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
        // DEBUGINFO("read_buffer 2:%s dataLength:%d\n",read_buffer,dataLength);
        parse_rbuf = true;      
      }

      if(parse_rbuf)
      {
        memset(printf_buffer,0,sizeof(printf_buffer));
        memcpy(printf_buffer,read_buffer,dataLength);
        for (int i = 0; i < (dataLength - 1); i++) { 
            if (printf_buffer[i] == '\0') {
                printf_buffer[i] = ' ';
            }
        }
        // DEBUGINFO("printf_buffer:%s\n",printf_buffer);
        // vPrint_Array(read_buffer,dataLength);
        if(read_buffer[0] == '\0')
        {
          read_buffer[0] = ' ';//去掉字符串起始的结束符
          // DEBUGINFO("remove start zero\n");
        }
        Wifi_ConnectAck(read_buffer,dataLength);
        Mqtt_ParseData(read_buffer,dataLength);
      }
      #ifdef WIFI_USE_MALLOC 
      vPortFree(wifi_data);      
      #endif 
    } 
  } 
}


