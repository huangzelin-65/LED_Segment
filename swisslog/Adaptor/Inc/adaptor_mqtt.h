#ifndef ADAPTOR_INC_ADAPTOR_MQTT_H_
#define ADAPTOR_INC_ADAPTOR_MQTT_H_
#include <stdbool.h>
#include "wolfmqtt/mqtt_client.h"

#define MQTT_SUBSCRIBE_COUNT     2
#define MQTT_SUBSCRIBE_LENGTH    64
#define MQTT_NAME_LENGTH         30
#define MQTT_PSW_LENGTH          30
#define MQTT_SN_LENGTH           50
#define MQTT_ID_LENGTH           7
#define MQTT_UUID_ID_LENGTH      ((2*3*32/8)+1) //3个32bit的数值，16进制上传，24个字符,最后一位添加结束符

#define MQTT_REGISTER_NAME      "hcms_def"    //静默注册时使用的账户
#define MQTT_REGISTER_PSW       "Abc@123456"  //静默注册时使用的账户密码

#define MQTT_SUB_ACTION        "tk/v1/slhc/tkv-%s/instantactions"
#define MQTT_SUB_CONN_ACK      "tk/v1/slhc/tkv-%s/connection/ack"  

#define MQTT_REGISTER_PUB_TOPIC "bcss/v1/slhc/register"
#define MQTT_REGISTER_SUB_TOPIC "bcss/v1/slhc/register/response"

typedef enum
{
    MQTT_OK = 0,
    MQTT_ERROR,
}MqttResult_t;

typedef enum
{
    MQTT_WAIT_STATE_IDLE = 0,
    MQTT_WAIT_STATE_SOCKET_CHECK,
    MQTT_WAIT_STATE_SOCKET_OPEN,
    MQTT_WAIT_STATE_TPMODE,
    MQTT_WAIT_STATE_WRITE,
    MQTT_WAIT_STATE_READ,
    MQTT_WAIT_STATE_SOCKET_CLOSE,
}MqttWaitState_t;

typedef enum
{
    MQTT_MSG_START = 0,//启动mqtt服务
    MQTT_MSG_INIT,
    MQTT_MSG_HEARTBEAT,
    MQTT_MSG_ROBOT_EVENT,
    MQTT_MSG_SUBSCRIBE,
    MQTT_MSG_ONLINE,
    MQTT_MSG_OFFLINE, 
    MQTT_MSG_REGISTER,   
}MqttMsgType_t;

typedef struct 
{
    MqttMsgType_t type;
    char *data;
}MqttMsgdata_t;

// 1：未连接
// 2：连接中
// 3：连接成功
// 4：连接失败
// 5：地址解析错误
typedef enum
{
    SOCKET_DISCONNECT = 1,
    SOCKET_CONNECTING,
    SOCKET_CONNECTED,
    SOCKET_FAIL,
    SOCKET_ERROR_ADDR
}Mqtt_SocketStatus;

typedef struct 
{
    int id;
    int type;
    int status;
    int host;
    int port;
    char ip_addr[16];    
}MqttSocket_t;

typedef struct 
{
    char *data;//接收到的数据指针
    int len;//数据的长度
    int rest_len;//剩余长度
}MqttReceiveData_t;

typedef enum {
    MQTT_NOTIFY_SUBSCRIBE = 0x01,
    MQTT_NOTIFY_ONLINE = 0x02,
    MQTT_NOTIFY_OFFLINE = 0x04,
    MQTT_NOTIFY_INIT = 0x08,
} MqttNotify_t;

typedef struct {
    char *topic_name;
    char *data;
}MqttRcMsg_t;

typedef struct {
	char name[MQTT_NAME_LENGTH];
	char pwd[MQTT_PSW_LENGTH]; 
    char sn[MQTT_SN_LENGTH];//作为client id使用 
    char id[MQTT_ID_LENGTH];//从sn中获取
    char uuid[MQTT_UUID_ID_LENGTH];//16进制表示的uid 
    char sub_topic[MQTT_SUBSCRIBE_COUNT][MQTT_SUBSCRIBE_LENGTH];//订阅话题
    uint8_t Register;//代表需要注册     
}MqttInfo_t;


extern MqttNet mNetwork;//网络结构体
extern MqttClient mClient;//mqtt客户端
extern int mqtt_isConnected;
extern int MqttReadReady;
extern MqttInfo_t mqtt_info;

int MqttInit(void);
void Mqtt_SendMsg(MqttMsgType_t msg,char *data);
void Mqtt_ParseData(uint8_t* rbuf,int len);
void Mqtt_PublishMsg(char *pub_topic, char *pub_buf, uint16_t data_len, uint8_t qos, uint8_t retain);
int Mqtt_SubscribeMsg(MqttTopic *topics,int count);
int Mqtt_SubscribeTopicInit(void);
void Mqtt_SetMsgCb(MqttClient *client,MqttMsgCb msg_cb);
void Mqtt_ListInit(void);
int Mqtt_GetListSize(void);
void Mqtt_PopListTail(void);
MqttReceiveData_t* Mqtt_GetListTail(void);
void Mqtt_Notify(uint32_t value);
void Mqtt_Restart(void);
int Mqtt_IsConnected(void);
#endif
