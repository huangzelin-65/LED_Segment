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
    MQTT_MSG_RECIEVE,
    MQTT_MSG_SUBSCRIBE,
}MqttMsgType_t;

typedef struct 
{
    MqttMsgType_t type;
    char *data;
}MqttMsgdata_t;

extern MqttObject mqttObj;//mqtt对象，用于连接客户端
extern MqttNet mNetwork;//网络结构体
extern MqttClient mClient;//mqtt客户端
extern int mqtt_isConnected;


int MqttInit(void);
void Mqtt_SendMsg(MqttMsgType_t msg,char *data);
void Mqtt_ParseData(uint8_t* rbuf,int len);
void Mqtt_PublishMsg(char *pub_topic, char *pub_buf, uint16_t data_len, uint8_t qos, uint8_t retain);
int Mqtt_SubscribeMsg(MqttTopic *topics,int count);
int Mqtt_SubscribeTopicInit(void);
void Mqtt_SetMsgCb(MqttClient *client,MqttMsgCb msg_cb);
#endif