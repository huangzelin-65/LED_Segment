#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "adaptor_mqtt.h"
#include "LogDebugInfo.h"
#include "queue.h"
#include "wolfmqtt/mqtt_client.h"
#include "stm32h5xx_hal.h"

#define MQTT_WIFI              //开启此宏，mqtt数据通过WiFi模块tcp功能发送
#define MQTT_HOST              "192.168.1.10" 
#define MQTT_QOS               MQTT_QOS_0
#define MQTT_KEEP_ALIVE_SEC    60
#define MQTT_CMD_TIMEOUT_MS    30000
#define MQTT_CON_TIMEOUT_MS    5000
#define MQTT_CLIENT_ID         "WolfMQTTClientSimple"
#define MQTT_TOPIC_NAME        "bcss/v1/slhc/st_1/state"
#define MQTT_SUB_TOPIC_NAME    "pc" 
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

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
static byte mSendBuf[MQTT_MAX_PACKET_SZ];
static byte mReadBuf[MQTT_MAX_PACKET_SZ];
char Mqtt_SendBuffer[MQTT_TX_BUF_SIZE];
MqttResult_t mqtt_result = MQTT_ERROR;
MqttWaitState_t mqtt_waitstate = MQTT_WAIT_STATE_IDLE;
int mqtt_ready2read = 0;//接收到mqtt数据，可以开始读取
int mqtt_rest2read = 0;//剩余需要区域读取得mqtt数据长度
int mqtt_socket_id = 0;//mqtt底层tcp连接时，被分配得socket ip
volatile word16 mPacketIdLast;//mqtt唯一id
MqttTopic topics[1];//订阅的话题
MqttObject mqttObj;//mqtt对象，用于连接客户端
MqttNet mNetwork;//网络结构体
MqttClient mClient;//mqtt客户端
int mSockFd = INVALID_SOCKET_FD;
char mqtt_readbuffer[MQTT_RX_BUF_SIZE];

HAL_StatusTypeDef Mqtt_SendATCmd(const char *cmd,int32_t timeout_ms)
{
  HAL_StatusTypeDef status;

  uint16_t len = snprintf(Mqtt_SendBuffer, sizeof(Mqtt_SendBuffer), "%s\r\n", cmd);

  status = HAL_UART_Transmit(&huart6, (uint8_t*)Mqtt_SendBuffer, len, timeout_ms);

  DEBUGINFO("cmd 6:%s",Mqtt_SendBuffer);

  return status;
}
//mqtt底层连接服务器函数
int Mqtt_NetConnect(void *context, const char* host, word16 port,int timeout_ms)
{
    #ifdef MQTT_WIFI
    mqtt_result = MQTT_ERROR;
    mqtt_waitstate = MQTT_WAIT_STATE_SOCKET_OPEN;
    memset(Mqtt_SendBuffer,0,MQTT_TX_BUF_SIZE);
    snprintf(Mqtt_SendBuffer, MQTT_TX_BUF_SIZE, "AT+SOCKET=1,%s,%d", host, port);
    Mqtt_SendATCmd(Mqtt_SendBuffer, 2000);  
    DEBUGINFO("mqtt_net_connect MQTT_WAIT_STATE_SOCKET_OPEN,host:%s port:%ld\n",host,port);
    static int cnt = 0;
    while(mqtt_result == MQTT_ERROR)
    {
        if(cnt++ >= timeout_ms)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(1);
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
    PRINTF("mqtt_net_connect");
    /* get address */
    XMEMSET(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    XMEMSET(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    PRINTF("mqtt_net_connect getaddrinfo host:%s",host);
    rc = getaddrinfo(host, NULL, &hints, &result);
    PRINTF("mqtt_net_connect getaddrinfo rc:%d",rc);
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
        PRINTF("NetConnect: Error %d (Sock Err %d)",
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
    #ifdef MQTT_WIFI
    DEBUGINFO("Mqtt_NetRead timeout_ms:%d",timeout_ms);
    static int cnt = 0;
    while(!mqtt_ready2read)
    {
        if(cnt++ >= timeout_ms)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(1);
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
        int rc;
        int *pSockFd = (int*)context;
        int bytes = 0;
        struct timeval tv;

        if (pSockFd == NULL) {
            return MQTT_CODE_ERROR_BAD_ARG;
        }
        PRINTF("mqtt_net_read start\n");
        /* Setup timeout */
        setup_timeout(&tv, timeout_ms);
        (void)setsockopt(*pSockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,
                sizeof(tv));
        PRINTF("mqtt_net_read buf_len:%d\n",buf_len);
        /* Loop until buf_len has been read, error or timeout */
        while (bytes < buf_len) {
            rc = (int)recv(*pSockFd, &buf[bytes], buf_len - bytes, 0);
            if (rc <= 0) {
                rc = socket_get_error(*pSockFd);
                if (rc == 0)
                    break; /* timeout */
                PRINTF("NetRead: Error %d", rc);
                return MQTT_CODE_ERROR_NETWORK;
            }
            bytes += rc; /* Data */
        }
        PRINTF("mqtt_net_read bytes:%d\n",bytes);
        if (bytes == 0) {
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        PRINTF("mqtt_net_read success\n");
        return bytes;
    #endif
}
//mqtt底层写数据接口函数
int Mqtt_NetWrite(void *context, const byte* buf, int buf_len,int timeout_ms)
{
    DEBUGINFO("Mqtt_NetWrite timeout_ms:%d",timeout_ms);
    #ifdef MQTT_WIFI
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
 
    HAL_UART_Transmit(&huart6, (uint8_t*)Mqtt_SendBuffer, (prefix_len + buf_len), 3000);

    static int cnt = 0;
    while(mqtt_result == MQTT_ERROR)
    {
        if(cnt++ >= timeout_ms)
        {
            cnt = 0;
            return MQTT_CODE_ERROR_TIMEOUT;
        }
        osDelay(1);
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
        PRINTF("NetWrite: Error %d (Sock Err %d)",
            rc, socket_get_error(*pSockFd));
        return MQTT_CODE_ERROR_NETWORK;
    }

    return rc;
    #endif
}
//mqtt底层断开服务器函数
int Mqtt_NetDisconnect(void *context)
{
    #ifdef MQTT_WIFI
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
        osDelay(1);
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
int MqttInit(void)
{
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
        goto exit;
    }
    DEBUGINFO("MQTT Network Connect Success: Host %s, Port %d, UseTLS %d",
        MQTT_HOST, MQTT_PORT, MQTT_USE_TLS);
    //连接服务器第二步
    XMEMSET(&mqttObj, 0, sizeof(mqttObj));
    mqttObj.connect.keep_alive_sec = MQTT_KEEP_ALIVE_SEC;
    mqttObj.connect.client_id = MQTT_CLIENT_ID;
    mqttObj.connect.username = MQTT_USERNAME;
    mqttObj.connect.password = MQTT_PASSWORD;
    rc = MqttClient_Connect(&mClient, &mqttObj.connect);
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }
    DEBUGINFO("MQTT Broker Connect Success: ClientID %s, Username %s, Password %s",
        MQTT_CLIENT_ID,
        (MQTT_USERNAME == NULL) ? "Null" : MQTT_USERNAME,
        (MQTT_PASSWORD == NULL) ? "Null" : MQTT_PASSWORD);

    //订阅话题
    XMEMSET(&mqttObj, 0, sizeof(mqttObj));
    topics[0].topic_filter = MQTT_SUB_TOPIC_NAME;
    topics[0].qos = MQTT_QOS;
    mqttObj.subscribe.packet_id = Mqtt_GetPacketid();
    mqttObj.subscribe.topic_count = sizeof(topics) / sizeof(MqttTopic);
    mqttObj.subscribe.topics = topics;
    rc = MqttClient_Subscribe(&mClient, &mqttObj.subscribe);
    if (rc != MQTT_CODE_SUCCESS) {
        goto exit;
    }
    DEBUGINFO("MQTT Subscribe Success: Topic %s, QoS %d",MQTT_SUB_TOPIC_NAME, MQTT_QOS);


    // memset(&mqtt_attributes, 0x0, sizeof(osThreadAttr_t));
    // mqtt_attributes.name = "mqtt_thread";
    // mqtt_attributes.stack_size = MQTT_THREAD_STACK_SIZE;
    // mqtt_attributes.priority = osPriorityLow;
    // osThreadNew(mqtt_thread, &mqttObj, &mqtt_attributes);

exit:
    if (rc != MQTT_CODE_SUCCESS) {
        DEBUGINFO("MQTT Error %d: %s", rc, MqttClient_ReturnCodeToString(rc));
    }
    return rc;
}
//mqtt解析数据
void Mqtt_ParseData(uint8_t* rbuf,int len)
{
    //解析tcp的数据
    {
        char target_mqtt_str[] = "+WFDATA=";
        // 查找目标前缀在rbuf中的位置
        char *result = strstr((char *)rbuf, target_mqtt_str);
        if (result != NULL) {
            DEBUGINFO("find:%s\n", target_mqtt_str);  
            int sid, data_len, pos;
            // "+WFDATA=%d,%d,%[^\n]" 匹配前缀，然后按逗号分隔读取两个整数，最后读取剩余所有字符的起始位置
            int ret = sscanf(result, "+WFDATA=%d,%d,%n", &sid, &data_len,&pos);
            DEBUGINFO("ret:%d\n",ret);  
            // 检查是否成功读取3个部分
            if (ret == 2 && (len > data_len)) {
                memcpy(mqtt_readbuffer, result + pos, data_len);       
                if(sid == mqtt_socket_id)
                {
                    memcpy(mReadBuf,mqtt_readbuffer,data_len + 1);
                    mqtt_rest2read = data_len;
                    mqtt_ready2read = 1;
                }
            }                
        }            
    }     
    if(mqtt_waitstate != MQTT_WAIT_STATE_IDLE)
    {
        DEBUGINFO("mqtt_waitstate %d\n",mqtt_waitstate);
    }
    switch(mqtt_waitstate)
    {
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
                DEBUGINFO("socket write ok\n");    
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
