#ifndef ADAPTOR_INC_ADAPTOR_TCP_H_
#define ADAPTOR_INC_ADAPTOR_TCP_H_

#include "lwip/tcp.h"
#include "lwip/err.h"
#include "lwip/memp.h"
#include "lwip/inet.h"
#include "tcp.h"

typedef enum {
    TCP_MSG_MQTT = 0,//发送心跳包到服务器
    TCP_MSG_SERVER,
} TcpMsgType_t;

typedef struct {
    TcpMsgType_t type;
    char *data;
}TcpMsg_t;

void Tcp_SendMsg(TcpMsgType_t type,char *data);
uint32_t Tcp_ServerSend(struct tcp_pcb *tpcb, const void* buf, uint32_t len);

#endif