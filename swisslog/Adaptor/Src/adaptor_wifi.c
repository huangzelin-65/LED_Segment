#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_wifi.h"
#include "LogDebugInfo.h"
#include "queue.h"

extern osMessageQueueId_t xWifi_Parse_QueueHandle;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
extern osMutexId_t wifiUsartMutexHandle;
char Wifi_SendBuffer[WIFI_TX_BUF_SIZE];//wifi数据发送的buffer
uint8_t Wifi_ReceiveBuffer[WIFI_RX_BUF_SIZE];//保存wifi数据，必要时需要加大长度
WifiState_t wifi_state = WIFI_IDLE;//连接wifi的步骤状态
WifiResult_t wifi_result = WIFI_ERROR;//连接wifi的步骤结果
uint16_t wifi_last_read_id = 0;//wifi数据解析的最后一个位置
WifiParseData_t wifi_parse_array[WIFI_ARRAY_LEN];
int wifi_parse_pos = 0; 
WifiStatus_t wifi_status = {
    .connect_state = WIFI_ERROR,
    .ip = {0},
    .rssi = 0
};//wifi模块所有状态

HAL_StatusTypeDef Wifi_SendATCmd(const char *cmd,int32_t timeout_ms)
{
  if (wifiUsartMutexHandle == NULL) return HAL_TIMEOUT;

  if (osMutexAcquire(wifiUsartMutexHandle, portMAX_DELAY) != osOK) return HAL_TIMEOUT;    

  HAL_StatusTypeDef status;

  uint16_t len = snprintf(Wifi_SendBuffer, sizeof(Wifi_SendBuffer), "%s\r\n", cmd);

  status = HAL_UART_Transmit(&huart6, (uint8_t*)Wifi_SendBuffer, len, timeout_ms);

  DEBUGINFO("cmd 6:%s status:%d len:%d",Wifi_SendBuffer,status,len);

  osMutexRelease(wifiUsartMutexHandle);

  return status;
}

//wifi模块状态初始化
void Wifi_Init(void)
{
    DEBUGINFO("Start\n");
    Wifi_SetPower(WIFI_POWER_OFF); //关闭wifi电源 
    osDelay(pdMS_TO_TICKS(500));//wifi模块先掉电
    Wifi_SetPower(WIFI_POWER_ON); //开启wifi电源 
    osDelay(pdMS_TO_TICKS(3000));//wifi模块上电需要等待3秒才可以发送命令
}
//启动串口空闲中断，关闭DMA半传输中断和传输完成中断，只响应串口空闲完成中断；
void Wifi_ReceiveInit(void)
{
    HAL_StatusTypeDef status;
    status = HAL_UARTEx_ReceiveToIdle_DMA(&huart6, Wifi_ReceiveBuffer, WIFI_RX_BUF_SIZE);
    if(status == HAL_OK)
    {
      DEBUGINFO("HAL_UARTEx_ReceiveToIdle_DMA huart6 OK");
      __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel2, DMA_IT_HT);
      __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel2, DMA_IT_TC); 
    }
}
//从中断中发送队列，线程中中获取队列，并解析数据
void Wifi_ParseDataStart(uint8_t *rx_buffer,uint16_t last_read_id,uint16_t size)
{
    #ifdef WIFI_USE_MALLOC
    WifiParseData_t *wifi_data = pvPortMalloc(sizeof(WifiParseData_t));
    wifi_data->last_read_id = last_read_id;
    wifi_data->size = size;
    wifi_data->rx_buffer = rx_buffer;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (xQueueSendFromISR(xWifi_Parse_QueueHandle, &wifi_data, &xHigherPriorityTaskWoken) == pdPASS) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 必要时切换任务
    }   
    #else
    WifiParseData_t data;
    data.last_read_id = last_read_id;
    data.size = size;
    data.rx_buffer = rx_buffer;
    if(wifi_parse_pos >= WIFI_ARRAY_LEN)
    {
        wifi_parse_pos = 0;
    }
    wifi_parse_array[wifi_parse_pos] = data;//赋值操作，线程中取出使用
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    WifiParseData_t *wifi_data = &wifi_parse_array[wifi_parse_pos];
    // DEBUGINFO("wifi_data %p rx_buffer:%p",wifi_data,wifi_data->rx_buffer);
    if (xQueueSendFromISR(xWifi_Parse_QueueHandle, &wifi_data, &xHigherPriorityTaskWoken) == pdPASS) {
        wifi_parse_pos = (wifi_parse_pos + 1) % WIFI_ARRAY_LEN;//移到下个位置
    }    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 必要时切换任务
    #endif
}
//接收中断中调用，处理wifi接收的数据
void Wifi_ReceiveData(uint16_t Size)
{
    // printf("Wifi_ReceiveData\n");
    Wifi_ParseDataStart(Wifi_ReceiveBuffer,wifi_last_read_id,Size);
    wifi_last_read_id = Size;
}
//调用此结果，判断此时wifi是否正常连接
bool Wifi_IsConnected(void)
{
    if(wifi_status.connect_state == WIFI_OK)
    {
      return true;
    }
    return false;
}
//判断wifi状态是否发生变化
bool Wifi_IsChanged(void)
{
    static WifiResult_t wifi_temp_connect_state = WIFI_ERROR;
    if(wifi_temp_connect_state != wifi_status.connect_state)
    {
        DEBUGINFO("Wifi_IsChanged %d",wifi_status.connect_state);
        wifi_temp_connect_state = wifi_status.connect_state;
        return true;
    }
    return false;
}
//启动状态机，开始连接wifi,一般需要wifi上电之后的3秒，才能启动连接wifi
void Wifi_ConnectStart(void)
{
    DEBUGINFO("Wifi_ConnectStart\n");
    wifi_result = WIFI_OK;
    wifi_state = WIFI_AT;
}
//线程中运行wifi连接过程，发送命令，等待结果回复，再进行下一步，直到返回成功；
void Wifi_ConnectProcess(void)
{  
    //wifi等待命令结果
    if(wifi_result != WIFI_OK)
    {
        // if(wifi_state != WIFI_IDLE)DEBUGINFO("wait wifi_result,cur state:%d",wifi_state);
        if((wifi_state - 1) == WIFI_AT)//如果没有回复AT,则代表是透传模式，需要退出
        {
            wifi_state = WIFI_TPMODE_EXIT_1;
        }
        else
        {
            return;
        }
    }
    //wifi状态机执行完毕
    if(wifi_state > WIFI_END)
    {  
        wifi_state = WIFI_IDLE;
    }   
    //wifi状态切换打印  
    static WifiState_t wifi_pre_state = WIFI_IDLE;
    if(wifi_state != wifi_pre_state)
    {
        wifi_pre_state = wifi_state;
        DEBUGINFO("wifi_state:%d\n",wifi_state);
        memset(Wifi_SendBuffer,0,sizeof(Wifi_SendBuffer));
    }
    switch (wifi_state)
    {
    case WIFI_AT:
        {
            Wifi_SendATCmd("AT",2000);
        }
        break; 
    case WIFI_TPMODE_EXIT_1:
        {
            Wifi_SendATCmd("+++",2000);
        }
        break; 
    case WIFI_TPMODE_EXIT_2:
        {
            Wifi_SendATCmd("a",2000);
        }
        break;                 
    case WIFI_CHECK_CONNET:
        {
            Wifi_SendATCmd("AT+LIP",2000);
        }
        break;         
    case WIFI_SET_CONNECT:
        {
            snprintf(Wifi_SendBuffer, WIFI_TX_BUF_SIZE, "AT+RAP=%s,%s", WIFI_SSID,WIFI_PSW);
            Wifi_SendATCmd(Wifi_SendBuffer,2000);             
        }
        break;
    case WIFI_END:
        {
          wifi_state = WIFI_IDLE;
        }   
        break;                                        
    default:
        if(wifi_state != WIFI_IDLE)DEBUGINFO("error wifi_state:%d",wifi_state);
        break;
    }
    if(wifi_state != WIFI_IDLE && wifi_state != WIFI_END)
    {
        wifi_state++; 
        wifi_result = WIFI_ERROR;
    }     
}
//wifi模块连接路由过程中，ack的校验
void Wifi_ConnectAck(uint8_t* rbuf,int len)
{
    //解析wifi的数据
    {
        char target_mqtt_str[] = "+RSSI=";
        // 查找目标前缀在rbuf中的位置
        char *result = strstr((char *)rbuf, target_mqtt_str);
        if (result != NULL) {
            DEBUGINFO("find:%s\n", target_mqtt_str);  
            int rssi;
            int ret = sscanf(result, "+RSSI=%d", &rssi);
            DEBUGINFO("ret:%d rssi:%d\n",ret,rssi);  
            wifi_status.rssi = rssi;              
        }            
    }  
    //保存wifi IP
    {
        char target_mqtt_str[] = WIFI_CHECK_IP;
        char *result = strstr((char *)rbuf, target_mqtt_str);
        if (result != NULL) {  
            wifi_result = WIFI_OK; 
            wifi_status.connect_state = WIFI_OK; 
            wifi_state = WIFI_END;    
            char *equal_pos = strchr((char *)rbuf, '=');
            if (equal_pos != NULL) {
                char *ip_str = equal_pos + 1;
                // 复制IP到字符串数组（strcpy会自动添加终止符）
                // 先检查IP长度，避免数组溢出（可选，增强安全性）
                if (strlen(ip_str) >= sizeof(wifi_status.ip)) {
                    DEBUGINFO("IP too long\n");
                }
                else
                {
                    strcpy(wifi_status.ip, ip_str); 
                    DEBUGINFO("WIFI IP:%s\n",wifi_status.ip);         
                }          
            }                
        }        
    }      
    if(wifi_state <= WIFI_AT) 
    {
        return;
    }
    DEBUGINFO("wifi_state:%d\n",wifi_state);      
    switch(wifi_state - 1)
    {
        case WIFI_AT:
        {
            char target_mqtt_str[] = "OK";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("WIFI_AT ok\n");    
                wifi_result = WIFI_OK;
                wifi_state = WIFI_CHECK_CONNET;    
            }            
        }
        break;  
        case WIFI_TPMODE_EXIT_1:
        {
            char target_mqtt_str[] = "a";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("WIFI_TPMODE_EXIT_1 ok\n");    
                wifi_result = WIFI_OK;     
            }            
        }
        break;
        case WIFI_TPMODE_EXIT_2:
        {
            char target_mqtt_str[] = "+ok";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("WIFI_TPMODE_EXIT_2 ok\n");    
                wifi_result = WIFI_OK;     
            }            
        }
        break;                
        case WIFI_CHECK_CONNET:
        {
            char target_mqtt_str[] = WIFI_CHECK_IP;
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("WIFI_CHECK_CONNET ok\n");   
                wifi_result = WIFI_OK; 
                wifi_status.connect_state = WIFI_OK; 
                wifi_state = WIFI_END;     
            } 
            else
            {
                DEBUGINFO("WIFI_CHECK_CONNET to WIFI_SET_CONNECT\n");
                wifi_result = WIFI_OK;  
            }           
        }
        break;              
        case WIFI_SET_CONNECT:
        {
            char target_mqtt_str[] = "WIFI_CONNECT";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("WIFI_SET_CONNECT ok\n");     
                wifi_result = WIFI_OK;
                wifi_status.connect_state = WIFI_OK;     
            }           
        }
        break;                    
        default:if(wifi_state != WIFI_IDLE)DEBUGINFO("error wifi_state:%d",wifi_state);
        break;        
    }
}

void Wifi_SetPower(WifiPower_t power)
{
    if(power == WIFI_POWER_OFF)
    {
        GPIO_WRITE(Wifi_Power,GPIO_PIN_SET); 
    }
    else
    {
        GPIO_WRITE(Wifi_Power,GPIO_PIN_SET); 
    }
}
//wifi模块本身的ota
void Wifi_OtaProcess(void)
{
    Wifi_SendATCmd("AT+RAP=dianys,88888888",2000);//开启手机热点给WiFi模块连接
    osDelay(10000);
    Wifi_SendATCmd("AT+HTTPOTA=http://120.78.6.197:8888/down/LqmWTlmwNdIU.bin",2000);//没讯提供ota连接
    //等待ota结果
}
