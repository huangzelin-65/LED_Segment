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

extern MqttObject mqttObj;//mqtt对象，用于连接客户端
extern MqttNet mNetwork;//网络结构体
extern MqttClient mClient;//mqtt客户端
extern int mqtt_isConnected;

int MqttInit(void);

#endif