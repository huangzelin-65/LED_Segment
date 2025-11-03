#ifndef ADAPTOR_INC_ADAPTOR_MQTT_H_
#define ADAPTOR_INC_ADAPTOR_MQTT_H_
#include <stdbool.h>

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

int MqttInit(void);

#endif