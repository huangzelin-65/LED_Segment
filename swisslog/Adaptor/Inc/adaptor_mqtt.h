#ifndef ADAPTOR_INC_ADAPTOR_MQTT_H_
#define ADAPTOR_INC_ADAPTOR_MQTT_H_
#include <stdbool.h>
#include "wolfmqtt/mqtt_client.h"

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
    MQTT_MSG_HEARTBEAT,
    MQTT_MSG_ROBOT_EVENT,
    MQTT_MSG_SUBSCRIBE,
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

extern MqttObject mqttObj;//mqtt对象，用于连接客户端
extern MqttNet mNetwork;//网络结构体
extern MqttClient mClient;//mqtt客户端
extern int mqtt_isConnected;


int MqttInit(const char *client_id);
void Mqtt_SendMsg(MqttMsgType_t msg,char *data);
void Mqtt_ParseData(uint8_t* rbuf,int len);
void Mqtt_PublishMsg(char *pub_topic, char *pub_buf, uint16_t data_len, uint8_t qos, uint8_t retain);
int Mqtt_SubscribeMsg(MqttTopic *topics,int count);
int Mqtt_SubscribeTopicInit(void);
void Mqtt_SetMsgCb(MqttClient *client,MqttMsgCb msg_cb);
#endif