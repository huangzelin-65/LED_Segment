#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_mqtt.h"
#include "LogDebugInfo.h"
#include "queue.h"
#include "stm32h5xx_hal.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "lwip.h"
#include "Robot.h"
#include "clist.h"
#include "app_freertos.h"
#include "semphr.h"

#define MQTT_HOST              "192.168.1.10" 
#define MQTT_QOS               MQTT_QOS_0
#define MQTT_KEEP_ALIVE_SEC    60
#define MQTT_CON_TIMEOUT_MS    3000
#define MQTT_CLIENT_ID         "WolfMQTTClientSimple"
#define MQTT_TOPIC_NAME        "bcss/v1/slhc/st_1/state"
#define MQTT_PUBLISH_MSG       "Test Publish"
#define MQTT_USERNAME          "hcms_mqtt"
#define MQTT_PASSWORD          "KM5zng23"
#ifdef ENABLE_MQTT_TLS
    #define MQTT_USE_TLS       1
    #define MQTT_PORT          8883
#else
    #define MQTT_USE_TLS       0
    #define MQTT_PORT          1883
#endif
#define MQTT_MAX_PACKET_SZ     2048
#define INVALID_SOCKET_FD      -1
#define PRINT_BUFFER_SIZE      1024
#define MQTT_TX_BUF_SIZE       1024
#define MQTT_RX_BUF_SIZE       1024
#define MQTT_SUBSCRIBE_COUNT     2
// #define MQTT_STATIC_ARRAY
extern CarStatus_t CarStatus;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
extern osMutexId_t wifiUsartMutexHandle;
extern osMutexId_t mqttMutexHandle;
static byte mSendBuf[MQTT_MAX_PACKET_SZ];
static byte mReadBuf[MQTT_MAX_PACKET_SZ];
char Mqtt_SendBuffer[MQTT_TX_BUF_SIZE];
MqttResult_t mqtt_result = MQTT_ERROR;
MqttWaitState_t mqtt_waitstate = MQTT_WAIT_STATE_IDLE;
int mqtt_ready2read = 0;//接收到mqtt数据，可以开始读取
int mqtt_rest2read = 0;//剩余需要区域读取得mqtt数据长度
int mqtt_socket_id = -1;//mqtt底层tcp连接时，被分配得socket ip
volatile word16 mPacketIdLast;//mqtt唯一id
MqttTopic subscribe_topics[MQTT_SUBSCRIBE_COUNT];//订阅的话题
MqttNet mNetwork;//网络结构体
MqttClient mClient;//mqtt客户端
int mSockFd = INVALID_SOCKET_FD;
#ifdef MQTT_STATIC_ARRAY
char mqtt_readbuffer[MQTT_RX_BUF_SIZE];
#endif
int mqtt_isConnected = 0;//mqtt连接状态
int MqttReadReady = 0;//mqtt开始接收数据
extern osMessageQueueId_t xMqttManagerQueueHandle;//处理mqtt任务的消息队列
MqttSocket_t mqtt_socket = {
    .ip_addr[0] = 0,
    .host = 0,
    .id = -1,
    .status = 0,
    .type = 0
};
List *mqtt_list = NULL;
//发送消息给线程，处理相关消息类型，指定处理内容
void Mqtt_SendMsg(MqttMsgType_t msg,char *data)
{
    if(xMqttManagerQueueHandle != NULL)
    {
        MqttMsgdata_t *msg_data = pvPortMalloc(sizeof(MqttMsgdata_t));
        if(msg_data == NULL)return;
        msg_data->type = msg;
        msg_data->data = data;
        DEBUGINFO("uxQueueGetQueueLength:%d uxQueueSpacesAvailable:%d\n",uxQueueGetQueueLength(xMqttManagerQueueHandle),uxQueueSpacesAvailable(xMqttManagerQueueHandle));
        if (xQueueSend(xMqttManagerQueueHandle, &msg_data, portMAX_DELAY) == pdPASS) 
        {
            DEBUGINFO("msg :%d\n",msg);
        } 
    } 
}

//发送消息给WiFi模块
HAL_StatusTypeDef Mqtt_SendATCmd(const char *cmd,int32_t timeout_ms)
{
  if (wifiUsartMutexHandle == NULL) return HAL_TIMEOUT;

  if (osMutexAcquire(wifiUsartMutexHandle, portMAX_DELAY) != osOK) return HAL_TIMEOUT;

  HAL_StatusTypeDef status;

  uint16_t len = snprintf(Mqtt_SendBuffer, sizeof(Mqtt_SendBuffer), "%s\r\n", cmd);

  status = HAL_UART_Transmit(&huart6, (uint8_t*)Mqtt_SendBuffer, len, timeout_ms);

  DEBUGINFO("cmd 6:%s",Mqtt_SendBuffer);

  osMutexRelease(wifiUsartMutexHandle);

  return status;
}
//mqtt底层连接服务器函数
int Mqtt_NetConnect(void *context, const char* host, word16 port,int timeout_ms)
{
    #ifdef MQTT_USE_WIFI
    static int cnt = 0;
    //检测socket连接情况
    mqtt_result = MQTT_ERROR;
    mqtt_waitstate = MQTT_WAIT_STATE_SOCKET_CHECK;
    memset(Mqtt_SendBuffer,0,MQTT_TX_BUF_SIZE);
    snprintf(Mqtt_SendBuffer, MQTT_TX_BUF_SIZE, "AT+SOCKET?");
    Mqtt_SendATCmd(Mqtt_SendBuffer, 2000);  
    DEBUGINFO("mqtt_net_connect MQTT_WAIT_STATE_SOCKET_CHECK\n");
    while(mqtt_result == MQTT_ERROR)
    {
        if(cnt++ >= timeout_ms)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(pdMS_TO_TICKS(1));
    }; 
    //判断socket状态
    if(mqtt_socket.id != -1)
    {
        if(mqtt_socket.status == SOCKET_CONNECTED)
        {
            DEBUGINFO("mqtt_net_connect MQTT_WAIT_STATE_SOCKET_CHECK success\n");
            return MQTT_CODE_SUCCESS;
        }
    }   
    //启动socket
    mqtt_result = MQTT_ERROR;
    mqtt_waitstate = MQTT_WAIT_STATE_SOCKET_OPEN;
    memset(Mqtt_SendBuffer,0,MQTT_TX_BUF_SIZE);
    snprintf(Mqtt_SendBuffer, MQTT_TX_BUF_SIZE, "AT+SOCKET=1,%s,%d", host, port);
    Mqtt_SendATCmd(Mqtt_SendBuffer, 2000);  
    DEBUGINFO("mqtt_net_connect MQTT_WAIT_STATE_SOCKET_OPEN,host:%s port:%ld\n",host,port);
    cnt = 0;
    while(mqtt_result == MQTT_ERROR)
    {
        if(cnt++ >= timeout_ms)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(pdMS_TO_TICKS(1));
    };
    cnt = 0;
    DEBUGINFO("mqtt_net_connect MQTT_WAIT_STATE_SOCKET_OPEN success\n");
    return MQTT_CODE_SUCCESS;
    #else    
    int rc;
    int sockFd, *pSockFd = (int*)context;
    struct sockaddr_in addr;
    struct addrinfo *result = NULL;
    struct addrinfo hints;

    if (pSockFd == NULL) {
        return MQTT_CODE_ERROR_BAD_ARG;
    }

    (void)timeout_ms;
    DEBUGINFO("mqtt_net_connect");
    /* get address */
    XMEMSET(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    XMEMSET(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    DEBUGINFO("mqtt_net_connect getaddrinfo host:%s",host);
    rc = getaddrinfo(host, NULL, &hints, &result);
    DEBUGINFO("mqtt_net_connect getaddrinfo rc:%d",rc);
    if (rc >= 0 && result != NULL) {
        struct addrinfo* res = result;

        /* prefer ip4 addresses */
        while (res) {
            if (res->ai_family == AF_INET) {
                break;
            }
            res = res->ai_next;
        }
        if (res) {
            addr.sin_port = htons(port);
            addr.sin_family = AF_INET;
            addr.sin_addr =
                ((struct sockaddr_in*)(res->ai_addr))->sin_addr;
        }
        else {
            rc = -1;
        }
        freeaddrinfo(result);
    }
    if (rc < 0) {
        return MQTT_CODE_ERROR_NETWORK;
    }

    sockFd = socket(addr.sin_family, SOCK_STREAM, 0);
    if (sockFd < 0) {
        return MQTT_CODE_ERROR_NETWORK;
    }

    /* Start connect */
    rc = connect(sockFd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc < 0) {
        DEBUGINFO("NetConnect: Error %d (Sock Err %d)",
            rc, socket_get_error(*pSockFd));
        close(sockFd);
        return MQTT_CODE_ERROR_NETWORK;
    }

    /* save socket number to context */
    *pSockFd = sockFd;

    return MQTT_CODE_SUCCESS;
    #endif
}
//mqtt底层读取数据接口函数
int Mqtt_NetRead(void *context, byte* buf, int buf_len, int timeout_ms)
{
    #ifdef MQTT_USE_WIFI
        // DEBUGINFO("start\n");
        #ifdef MQTT_STATIC_ARRAY
            static int cnt = 0;
            while(!mqtt_ready2read)
            {
                if(cnt++ >= timeout_ms)
                {
                    cnt = 0;
                    return MQTT_CODE_ERROR_TIMEOUT;
                }
                osDelay(pdMS_TO_TICKS(1));
            };
            cnt = 0;
            if(mqtt_rest2read)//需要及时消耗完成
            {
                mqtt_rest2read -= buf_len;
                if(mqtt_rest2read <= 0)
                {
                    mqtt_ready2read = 0;
                }
            }
            return buf_len; 
        #else
            if(mqtt_list != NULL)    
            {                
                static int cnt = 0;
                while(!Mqtt_GetListSize())
                {
                    if(cnt++ >= timeout_ms)
                    {
                        cnt = 0;
                        DEBUGINFO("MQTT_CODE_ERROR_TIMEOUT\n");
                        return MQTT_CODE_ERROR_TIMEOUT;
                    }
                    osDelay(pdMS_TO_TICKS(1));
                };
                cnt = 0;   
                MqttReceiveData_t *rec_data = Mqtt_GetListTail();
                if(rec_data == NULL)return MQTT_CODE_ERROR_BAD_ARG; 
                // DEBUGINFO("buf_len:%d len:%d rest_len:%d\n",buf_len,rec_data->len,rec_data->rest_len);
                if(rec_data->len == rec_data->rest_len)
                {
                    memcpy(mReadBuf,rec_data->data,rec_data->len);
                }
                rec_data->rest_len -= buf_len;
                if(rec_data->rest_len <= 0)
                {
                    rec_data->rest_len = 0;
                    // DEBUGINFO("Mqtt_PopListTail rec_data:%p data:%p\n",rec_data,rec_data->data);
                    // DEBUGINFO("mqtt_list size:%d\n",list_size(mqtt_list));
                    vPortFree(rec_data->data);
                    Mqtt_PopListTail();                    
                }
                return buf_len;                             
            }
            else
            {
                return MQTT_CODE_ERROR_BAD_ARG;
            }
        #endif
    #else
        int rc;
        int *pSockFd = (int*)context;
        int bytes = 0;
        struct timeval tv;

        if (pSockFd == NULL) {
            return MQTT_CODE_ERROR_BAD_ARG;
        }
        DEBUGINFO("mqtt_net_read start\n");
        /* Setup timeout */
        setup_timeout(&tv, timeout_ms);
        (void)setsockopt(*pSockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
                sizeof(tv));
        DEBUGINFO("mqtt_net_read buf_len:%d\n",buf_len);
        /* Loop until buf_len has been read, error or timeout */
        while (bytes < buf_len) {
            rc = (int)recv(*pSockFd, &buf[bytes], buf_len - bytes, 0);
            if (rc <= 0) {
                rc = socket_get_error(*pSockFd);
                if (rc == 0)
                    break; /* timeout */
                DEBUGINFO("NetRead: Error %d", rc);
                return MQTT_CODE_ERROR_NETWORK;
            }
            bytes += rc; /* Data */
        }
        DEBUGINFO("mqtt_net_read bytes:%d\n",bytes);
        if (bytes == 0) {
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        DEBUGINFO("mqtt_net_read success\n");
        return bytes;
    #endif
}
//mqtt底层写数据接口函数
int Mqtt_NetWrite(void *context, const byte* buf, int buf_len,int timeout_ms)
{
    DEBUGINFO("Mqtt_NetWrite timeout_ms:%d",timeout_ms);
    #ifdef MQTT_USE_WIFI
    memset(Mqtt_SendBuffer,0,1024);
    int prefix_len = snprintf(Mqtt_SendBuffer, MQTT_TX_BUF_SIZE, "AT+SOCKETSENDLINE=%d,%d,", mqtt_socket_id, buf_len);
    // 检查前缀生成是否正常，以及剩余空间是否足够容纳 buf
    if (prefix_len < 0 || prefix_len >= 1024) {
        // 前缀生成失败（缓冲区不足），处理错误
        return MQTT_CODE_ERROR_OUT_OF_BUFFER;
    }
    int remaining_space = 1024 - prefix_len;
    if (remaining_space < buf_len) {
        // buf 太长，缓冲区不足，处理错误（例如截断或报错）
        buf_len = remaining_space; 
    }
    mqtt_result = MQTT_ERROR;
    mqtt_waitstate = MQTT_WAIT_STATE_WRITE;    
    // 步骤2：用 memcpy 复制 buf 的全部内容（包括中间的 '\0'）
    memcpy(Mqtt_SendBuffer + prefix_len, buf, buf_len);   
 
    if (wifiUsartMutexHandle == NULL) return MQTT_CODE_ERROR_TIMEOUT;

    if (osMutexAcquire(wifiUsartMutexHandle, portMAX_DELAY) != osOK) return MQTT_CODE_ERROR_TIMEOUT;

    HAL_UART_Transmit(&huart6, (uint8_t*)Mqtt_SendBuffer, (prefix_len + buf_len), 3000);

    osMutexRelease(wifiUsartMutexHandle);

    static int cnt = 0;
    while(mqtt_result == MQTT_ERROR)
    {
        if(cnt++ >= timeout_ms)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(pdMS_TO_TICKS(1));
    };
    cnt = 0;
    return MQTT_CODE_SUCCESS; 
    #else

    int rc;
    int *pSockFd = (int*)context;
    struct timeval tv;

    if (pSockFd == NULL) {
        return MQTT_CODE_ERROR_BAD_ARG;
    }

    /* Setup timeout */
    setup_timeout(&tv, timeout_ms);
    (void)setsockopt(*pSockFd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,
            sizeof(tv));

    rc = (int)send(*pSockFd, buf, buf_len, 0);
    if (rc < 0) {
        DEBUGINFO("NetWrite: Error %d (Sock Err %d)",
            rc, socket_get_error(*pSockFd));
        return MQTT_CODE_ERROR_NETWORK;
    }

    return rc;
    #endif
}
//mqtt底层断开服务器函数
int Mqtt_NetDisconnect(void *context)
{
    #ifdef MQTT_USE_WIFI
    mqtt_result = MQTT_ERROR;
    mqtt_waitstate = MQTT_WAIT_STATE_SOCKET_CLOSE;
    memset(Mqtt_SendBuffer,0,1024);
    snprintf(Mqtt_SendBuffer, 1024, "AT+SOCKETDEL=%d",mqtt_socket_id);
    Mqtt_SendATCmd(Mqtt_SendBuffer, 2000);  
    DEBUGINFO("mqtt_net_disconnect socket:%ld\n",mqtt_socket_id);
    static int cnt = 0;
    while(mqtt_result == MQTT_ERROR)
    {
        if(cnt++ >= 30)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(pdMS_TO_TICKS(1));
    };
    cnt = 0;
    DEBUGINFO("mqtt_net_disconnect success\n");
    return MQTT_CODE_SUCCESS;    
    #else    
    int *pSockFd = (int*)context;

    if (pSockFd == NULL) {
        return MQTT_CODE_ERROR_BAD_ARG;
    }

    close(*pSockFd);
    *pSockFd = INVALID_SOCKET_FD;

    return MQTT_CODE_SUCCESS;
    #endif    
}

//客户端所有发送消息使用的id
static word16 Mqtt_GetPacketid(void)
{
    if (mPacketIdLast >= MAX_PACKET_ID) {
        mPacketIdLast = 0;
    }
    return ++mPacketIdLast;
}
//客户端接收到信息时的回调，此处做数据解析处理
static int Mqtt_MessageCb(MqttClient *client, MqttMessage *msg,byte msg_new, byte msg_done)
{
    byte buf[PRINT_BUFFER_SIZE+1];
    word32 len;

    (void)client;

    if (msg_new) {
        /* Determine min size to dump */
        len = msg->topic_name_len;
        if (len > PRINT_BUFFER_SIZE) {
            len = PRINT_BUFFER_SIZE;
        }
        XMEMCPY(buf, msg->topic_name, len);
        buf[len] = '\0'; /* Make sure its null terminated */
        /* Print incoming message */
        DEBUGINFO("MQTT Message: Topic %s, Qos %d, Len %u",buf, msg->qos, msg->total_len);
    }

    /* Print message payload */
    len = msg->buffer_len;
    if (len > PRINT_BUFFER_SIZE) {
        len = PRINT_BUFFER_SIZE;
    }
    XMEMCPY(buf, msg->buffer, len);
    buf[len] = '\0'; /* Make sure its null terminated */
    DEBUGINFO("Payload (%d - %d) printing %d bytes:" LINE_END "%s",
        msg->buffer_pos, msg->buffer_pos + msg->buffer_len, len, buf);

    if (msg_done) {
        DEBUGINFO("MQTT Message: Done");

        MqttRcMsg_t *mqtt_msg = pvPortMalloc(sizeof(MqttRcMsg_t));
        if(mqtt_msg != NULL)
        {
            mqtt_msg->topic_name = pvPortMalloc(msg->topic_name_len + 1);
            if(mqtt_msg->topic_name != NULL)
            {
                XMEMCPY(mqtt_msg->topic_name, msg->topic_name, msg->topic_name_len);
                mqtt_msg->topic_name[msg->topic_name_len] = '\0';

                mqtt_msg->data = pvPortMalloc(msg->buffer_len + 1);
                if(mqtt_msg->data != NULL)
                {
                    XMEMCPY(mqtt_msg->data, msg->buffer, msg->buffer_len);
                    mqtt_msg->data[msg->buffer_len] = '\0';
                    Robot_SendMsg(ROBOT_MSG_PARSE,mqtt_msg);                    
                }
                else
                {
                    DEBUGINFO("pvPortMalloc Message fail!!!");
                }
            }
            else
            {
                DEBUGINFO("pvPortMalloc Message fail!!!");
            }
        }
        else
        {
            DEBUGINFO("pvPortMalloc Message fail!!!");
        }
    }
    return MQTT_CODE_SUCCESS;
}
//传输加密的回调,暂时不用
static int Mqtt_TlsCb(MqttClient* client)
{
    (void)client;
    return 0;
}
//mqtt参数初始化，连接服务器、订阅话题
int MqttInit(const char *client_id)
{
    MqttObject mqttObj;
    int rc = 0;
    //初始化客户端
    XMEMSET(&mNetwork, 0, sizeof(mNetwork));
    mNetwork.connect = Mqtt_NetConnect;
    mNetwork.read = Mqtt_NetRead;
    mNetwork.write = Mqtt_NetWrite;
    mNetwork.disconnect = Mqtt_NetDisconnect;
    mNetwork.context = &mSockFd;
    rc = MqttClient_Init(&mClient, &mNetwork, Mqtt_MessageCb,
        mSendBuf, sizeof(mSendBuf), mReadBuf, sizeof(mReadBuf),
        MQTT_CON_TIMEOUT_MS);
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }
    DEBUGINFO("MQTT Init Success");
    //连接服务器第一步
    rc = MqttClient_NetConnect(&mClient, MQTT_HOST, MQTT_PORT,
        MQTT_CON_TIMEOUT_MS, MQTT_USE_TLS, Mqtt_TlsCb);
    if (rc != MQTT_CODE_SUCCESS) {
        DEBUGINFO("MqttClient_NetConnect fail");
        goto exit;
    }
    DEBUGINFO("MQTT Network Connect Success: Host %s, Port %d, UseTLS %d",
        MQTT_HOST, MQTT_PORT, MQTT_USE_TLS);
    //连接服务器第二步
    XMEMSET(&mqttObj, 0, sizeof(mqttObj));
    mqttObj.connect.keep_alive_sec = MQTT_KEEP_ALIVE_SEC;
    mqttObj.connect.client_id = client_id;
    mqttObj.connect.username = MQTT_USERNAME;
    mqttObj.connect.password = MQTT_PASSWORD;
    rc = MqttClient_Connect(&mClient, &mqttObj.connect);
    if (rc != MQTT_CODE_SUCCESS) {
        DEBUGINFO("MqttClient_Connect fail");
        goto exit;
    }
    DEBUGINFO("MQTT Broker Connect Success: ClientID %s, Username %s, Password %s",
        client_id,
        (MQTT_USERNAME == NULL) ? "Null" : MQTT_USERNAME,
        (MQTT_PASSWORD == NULL) ? "Null" : MQTT_PASSWORD);
    mqtt_isConnected = 1;
    return rc;
exit:
    if (rc != MQTT_CODE_SUCCESS) {
        DEBUGINFO("MQTT Error %d: %s", rc, MqttClient_ReturnCodeToString(rc));
    }
    return rc;
}
/**
 * @brief 在一个可能包含'\0'的字节数组中查找所有"+WFDATA="的起始位置。
 * 
 * @param arr 待搜索的字节数组。
 * @param arr_len 数组的总长度。
 * @param result 用于存储结果（起始索引）的数组。
 * @param max_result 结果数组的最大容量。
 * @return int 实际找到的匹配项数量。
 */
int Mqtt_FindAllStrPositions(uint8_t *arr,char *target,int arr_len, int *result, int max_result)
{
    int target_len = strlen(target);
    int count = 0;

    // 遍历数组，直到剩下的长度不足以容纳目标字符串
    for (int i = 0; i <= arr_len - target_len; i++) {
        // 检查从当前位置i开始的字符串是否与目标匹配
        if (memcmp(arr + i, target, target_len) == 0) {
            // 如果找到匹配项，且结果数组还有空间，则存储位置
            if (count < max_result) {
                result[count] = i;
            }
            count++;
        }
    }

    return count;
}
//将数据解析到数组里面（动态）
void Mqtt_ParseData2List(uint8_t *result,int len)
{
    if(mqtt_list == NULL)return;//链表还未初始化       
    int sid, data_len, pos;
    // "+WFDATA=%d,%d,%[^\n]" 匹配前缀，然后按逗号分隔读取两个整数，最后读取剩余所有字符的起始位置
    int ret = sscanf((char*)result, "+WFDATA=%d,%d,%n", &sid, &data_len,&pos);
    DEBUGINFO("ret:%d sid:%d len:%d data_len:%d pos:%d\n",ret,sid,len,data_len,pos);  
    if(sid != mqtt_socket_id)
    {
        DEBUGINFO("sid is different,mqtt_socket_id:%d",mqtt_socket_id);
        return;
    }
    // 检查是否成功读取3个部分
    if (ret == 2 && (len > data_len))
    {                    
        if (mqttMutexHandle != NULL)
        {
            // DEBUGINFO("mqtt_list add start\n");
            if (osMutexAcquire(mqttMutexHandle, portMAX_DELAY) == osOK)
            {                            
                MqttReceiveData_t *rec_data =  pvPortMalloc(sizeof(MqttReceiveData_t));
                if(rec_data != NULL)
                {
                    rec_data->data = pvPortMalloc(data_len + 1);
                    rec_data->len = data_len;
                    rec_data->rest_len = data_len;
                    if(rec_data->data != NULL)
                    {
                        memcpy(rec_data->data, result + pos, data_len);
                        // DEBUGINFO("list_insert_head rec_data:%p data:%p\n",rec_data,rec_data->data);
                        int rc = list_insert_head(mqtt_list,rec_data);
                        if(rc == -1)
                        {
                            DEBUGINFO("mqtt_list fail\n");
                        }
                        else
                        {
                            DEBUGINFO("mqtt_list size:%d\n",list_size(mqtt_list));
                        }
                    }
                    else
                    {
                        vPortFree(rec_data);
                    }
                }

                osMutexRelease(mqttMutexHandle); 
            }
            // DEBUGINFO("mqtt_list add end\n");
        }
    }
}
//解析mqtt数据包,搜索是否有"+WFDATA="开头的，有则解析
void Mqtt_ParseTcpData(uint8_t* rbuf,int len)
{
    {
        char target_mqtt_str[] = "+SOCKET_OPEN:";
        // 查找目标前缀在rbuf中的位置
        char *result = strstr((char *)rbuf, target_mqtt_str);
        if (result != NULL) {
            DEBUGINFO("find:%s\n", target_mqtt_str);    
            int sid, error;
            // 1. 从result（目标前缀的起始位置）开始解析，确保格式匹配
            // 2. 修改变量名，避免与char* result冲突（用ret接收sscanf返回值）
            int ret = sscanf(result, "+SOCKET_OPEN:%d,%d", &sid, &error);
            if (ret == 2) {  // 检查是否成功提取2个整数
                DEBUGINFO("sid:%d mqtt_socket_id:%d\n", sid,mqtt_socket_id); 
                if((mqtt_socket_id == sid) && (error == 48))//socket 从连接状态变为未连接
                {
                    DEBUGINFO("need to restart mqtt"); 
                    Mqtt_Restart();                   
                }
            }                
        }
    }    
    #ifdef MQTT_STATIC_ARRAY
        char target_str[] = "+WFDATA=";
        // 查找目标前缀在rbuf中的位置
        char *result = strstr((char *)rbuf, target_str);
        if (result != NULL) {
            DEBUGINFO("find:%s\n", target_str);  
            int sid, data_len, pos;
            // "+WFDATA=%d,%d,%[^\n]" 匹配前缀，然后按逗号分隔读取两个整数，最后读取剩余所有字符的起始位置
            int ret = sscanf(result, "+WFDATA=%d,%d,%n", &sid, &data_len,&pos);
            DEBUGINFO("ret:%d sid:%d len:%d data_len:%d pos:%d rbuf:%ld result:%ld\n",ret,sid,len,data_len,pos,rbuf,result);  
            // 检查是否成功读取3个部分
            if (ret == 2 && (len > data_len)) {
                //静态数组形式
                memcpy(mqtt_readbuffer, result + pos, data_len);
                DEBUGINFO("mqtt_socket_id:%ld",mqtt_socket_id);    
                if(sid == mqtt_socket_id)
                {
                    memcpy(mReadBuf,mqtt_readbuffer,data_len + 1);
                    mqtt_rest2read = data_len;
                    mqtt_ready2read = 1;
                }

            }
            else
            {
                DEBUGINFO("error data_len:%d len:%d ret:%d\n", data_len,len,ret);
            }                
        } 
    #else
        // 假设最多查找 10 个位置
        char target_str[] = "+WFDATA=";
        int positions[10];
        int found_count = Mqtt_FindAllStrPositions(rbuf,target_str,len,positions,10);

        if (found_count > 0) {
            // DEBUGINFO("found_count: %d\n", found_count);
            for (int i = 0; i < found_count; i++) {
                // DEBUGINFO("  position(%d): %d\n", i, positions[i]);
                Mqtt_ParseData2List((rbuf + positions[i]),len);
            }
        } 
    #endif
}
//mqtt解析数据
void Mqtt_ParseData(uint8_t* rbuf,int len)
{
    //解析tcp的数据
    Mqtt_ParseTcpData(rbuf,len);    
    if(mqtt_waitstate != MQTT_WAIT_STATE_IDLE)
    {
        // DEBUGINFO("mqtt_waitstate %d\n",mqtt_waitstate);
    }
    switch(mqtt_waitstate)
    {
        case MQTT_WAIT_STATE_SOCKET_CHECK:
        {
            {
                //+SOCKET:id,type,status,host,port
                //+SOCKET:0,1,3,192.168.1.130,777
                // 假设目标字符串前缀为"+SOCKET:"
                char *target_str = "+SOCKET:";
                char *result = strstr((char *)rbuf, target_str);
                if (result != NULL) {
                    DEBUGINFO("find:%s\n", target_str);    
                    // 解析格式：前缀"+SOCKET:"后依次为%d,%d,%d,%s,%d（逗号分隔）
                    int ret = sscanf(result, "+SOCKET:%d,%d,%d,%15s,%d", &mqtt_socket.id, &mqtt_socket.type, &mqtt_socket.status, mqtt_socket.ip_addr, &mqtt_socket.port);
                    if (ret == 5) {  // 成功提取5个内容
                        DEBUGINFO("get parameters success\n");
                        DEBUGINFO("id:%d\n", mqtt_socket.id);       // 对应第一个逗号后的值：0
                        DEBUGINFO("type:%d\n", mqtt_socket.type);       // 对应第二个逗号后的值：1
                        DEBUGINFO("status:%d\n", mqtt_socket.status);       // 对应第三个逗号后的值：3
                        DEBUGINFO("ip_addr:%s\n", mqtt_socket.ip_addr); // 对应第四个逗号后的值：192.168.1.130
                        DEBUGINFO("port:%d\n", mqtt_socket.port);       // 对应第五个逗号后的值：777
                        // 可根据需要将提取的值赋值给其他变量（如业务变量）
                    } else {
                        DEBUGINFO("get parameters fail\n");
                    }                
                } 
            }
            {
                char target_mqtt_str[] = "OK";
                char *result = strstr((char *)rbuf, target_mqtt_str);
                if (result != NULL) {
                    DEBUGINFO("MQTT_WAIT_STATE_SOCKET_CHECK ok\n");    
                    mqtt_result = MQTT_OK;        
                }                 
            }           
        }
        break;
        case MQTT_WAIT_STATE_SOCKET_OPEN:
        {
            char target_mqtt_str[] = "+SOCKET_OPEN:";
            // 查找目标前缀在rbuf中的位置
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("find:%s\n", target_mqtt_str);    
                int sid, error;
                // 1. 从result（目标前缀的起始位置）开始解析，确保格式匹配
                // 2. 修改变量名，避免与char* result冲突（用ret接收sscanf返回值）
                int ret = sscanf(result, "+SOCKET_OPEN:%d,%d", &sid, &error);
                if (ret == 2) {  // 检查是否成功提取2个整数
                    DEBUGINFO("get num suc\n");
                    DEBUGINFO("sid:%d\n", sid); 
                    DEBUGINFO("error:%d\n", error);
                    if(error == 0) {
                        mqtt_socket_id = sid;
                        mqtt_result = MQTT_OK;  
                    }
                } else {
                    DEBUGINFO("get parameters fail\n");
                }                
            }
        }
        break;
        case MQTT_WAIT_STATE_TPMODE:
        {
            char target_mqtt_str[] = "OK";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("Tp mode ok\n");    
                mqtt_result = MQTT_OK;        
            }            
        }
        break;        
        case MQTT_WAIT_STATE_WRITE:
        {
            char target_mqtt_str[] = "OK";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                // DEBUGINFO("socket write ok\n");    
                mqtt_result = MQTT_OK;        
            }            
        }
        break;
        case MQTT_WAIT_STATE_SOCKET_CLOSE:
        {
            char target_mqtt_str[] = "OK";
            char *result = strstr((char *)rbuf, target_mqtt_str);
            if (result != NULL) {
                DEBUGINFO("socket close ok\n");    
                mqtt_result = MQTT_OK;      
            }            
        }
        break;        
        default:break;
    }   
}
//发布消息调用接口
void Mqtt_PublishMsg(char *pub_topic, char *pub_buf, uint16_t data_len, uint8_t qos, uint8_t retain)
{
    if(!mqtt_isConnected)return;
    MqttObject mqttObj;
    XMEMSET(&mqttObj, 0, sizeof(mqttObj));
    mqttObj.publish.qos = qos;
    mqttObj.publish.retain = retain;
    mqttObj.publish.topic_name = pub_topic;
    mqttObj.publish.packet_id = Mqtt_GetPacketid();
    mqttObj.publish.buffer = (byte*)pub_buf;
    mqttObj.publish.total_len = data_len;
    mqttObj.publish.buffer_len = data_len;
    int rc = MqttClient_Publish(&mClient, &mqttObj.publish);
    // int rc = MqttClient_Publish_WriteOnly(&mClient, &mqttObj.publish,wolfmqtt_PublishCb);
    // int rc = MqttClient_Publish_ex(&mClient, &mqttObj.publish,wolfmqtt_PublishCb);
    DEBUGINFO("MqttClient_Publish rc:%d\n",rc);  
}
//订阅话题调用接口
int Mqtt_SubscribeMsg(MqttTopic *topics,int count)
{
    MqttObject mqttObj;
    XMEMSET(&mqttObj, 0, sizeof(mqttObj));
    mqttObj.subscribe.packet_id = Mqtt_GetPacketid();
    mqttObj.subscribe.topic_count = count;
    mqttObj.subscribe.topics = topics;
    int rc = MqttClient_Subscribe(&mClient, &mqttObj.subscribe);
    if (rc == MQTT_CODE_SUCCESS) {
        DEBUGINFO("MQTT Subscribe Success");
    }
    return rc;
}

//需要订阅的话题初始化
int Mqtt_SubscribeTopicInit(uint16_t id)
{
    DEBUGINFO("start");
    char sub_topic[MQTT_SUBSCRIBE_COUNT][64];
    for (int i = 0; i < MQTT_SUBSCRIBE_COUNT; i++)
    {
        switch (i)
        {
            case 0:
            {
                snprintf(sub_topic[i], 64, MQTT_SUB_ACTION, id);                
                subscribe_topics[i].topic_filter = sub_topic[i];
                subscribe_topics[i].qos = MQTT_QOS;
            }
            break;
            case 1:
            {
                snprintf(sub_topic[i], 64, MQTT_SUB_CONN_ACK, id);                
                subscribe_topics[i].topic_filter = sub_topic[i];
                subscribe_topics[i].qos = MQTT_QOS;
            }
            break;
            case 2:
            {
                subscribe_topics[i].topic_filter = MQTT_SUB_ACTION;//此处话题需要根据实际需要更换
                subscribe_topics[i].qos = MQTT_QOS;
            }
            break;                    
            default:
            break;
        }
    }
    
    int rc = Mqtt_SubscribeMsg(subscribe_topics,MQTT_SUBSCRIBE_COUNT);
    if (rc == MQTT_CODE_SUCCESS) {
        DEBUGINFO("Mqtt_SubscribeTopicInit Success");
    }    
    return rc;
}


void Mqtt_SetMsgCb(MqttClient *client,MqttMsgCb msg_cb)
{
    if(client == NULL) return;

    client->msg_cb = msg_cb;
}

void Mqtt_ListInit(void)
{
    mqtt_list = list_init();
    if(mqtt_list != NULL)
    {
        DEBUGINFO("mqtt_list init Success");
    }
}

int Mqtt_GetListSize(void)
{
    int rc = 0;
    if (mqttMutexHandle != NULL)
    {
        // DEBUGINFO("start");
        if (osMutexAcquire(mqttMutexHandle, portMAX_DELAY) == osOK)
        {
            rc = list_size(mqtt_list);
            osMutexRelease(mqttMutexHandle); 
        }
        // DEBUGINFO("end");       
    }
    return rc;
}

MqttReceiveData_t* Mqtt_GetListTail(void)
{
    MqttReceiveData_t *rec_data = NULL;
    if (mqttMutexHandle == NULL)return rec_data;
    // DEBUGINFO("start");
    if (osMutexAcquire(mqttMutexHandle, portMAX_DELAY) == osOK)
    {
        rec_data = list_get_last(mqtt_list); 
        osMutexRelease(mqttMutexHandle); 
    }
    // DEBUGINFO("end");
    return rec_data;
}

void Mqtt_PopListTail(void)
{
    // DEBUGINFO("start");
    if (mqttMutexHandle == NULL)return;
    if (osMutexAcquire(mqttMutexHandle, portMAX_DELAY) == osOK)
    {
        list_pop_tail(mqtt_list,vPortFree);
        osMutexRelease(mqttMutexHandle); 
    }  
    // DEBUGINFO("end");  
}

//Mqtt消息通知
void Mqtt_Notify(uint32_t value)
{
    if(MqttNotifyTaskHandle != NULL)
    {
        DEBUGINFO("value:%lx",value);
        BaseType_t xReturn = pdPASS;
        xReturn = xTaskNotify(MqttNotifyTaskHandle, 
                    value, 
                    eSetValueWithoutOverwrite);
        if(xReturn != pdPASS)
        {
            DEBUGINFO("xReturn is not pdPASS:%ld\n",xReturn);
        } 
    }
    else
    {
        DEBUGINFO("MqttNotifyTaskHandle NULL");
    }
}
//重启mqtt服务
void Mqtt_Restart(void)
{
    DEBUGINFO("start");
    mqtt_isConnected = 0;
    MqttReadReady = 0;
    Mqtt_SendMsg(MQTT_MSG_START,NULL); 
}
//获取mqtt服务连接状态
int Mqtt_IsConnected(void)
{
    return mqtt_isConnected;
}



