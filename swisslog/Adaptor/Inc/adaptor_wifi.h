#ifndef ADAPTOR_INC_ADAPTOR_WIFI_H_
#define ADAPTOR_INC_ADAPTOR_WIFI_H_
#include <stdbool.h>
// 缓冲区大小
#define WIFI_TX_BUF_SIZE   1024
#define WIFI_RX_BUF_SIZE   (1024 * 2)

/* 配置参数定义 */

#define WIFI_WLAN  "192.168.1.1"
#define WIFI_MASK  "255.255.255.0" 
#define WIFI_DHCP  1
#define WIFI_SSID "SwisslogHC"
#define WIFI_PSW  "SwisslogHC" 
#define WIFI_CHECK_IP      "192.168.10"//验证WIFI是否已经连接到理由，注意：此处需要根据实际情况更改
typedef enum
{
    WIFI_IDLE = 0x00,
    WIFI_AT,
    WIFI_TPMODE_EXIT_1,
    WIFI_TPMODE_EXIT_2,
    WIFI_CHECK_CONNET, 
    WIFI_SET_CONNECT, 
    WIFI_TO_MQTT,   
    WIFI_END,
} WifiState_t;

typedef enum
{
    WIFI_OK = 0x00,
    WIFI_ERROR,
} WifiResult_t;

typedef struct 
{
    uint16_t last_read_id;
    uint16_t size;
    uint8_t *rx_buffer;
}WifiParseData_t;

HAL_StatusTypeDef Wifi_SendATCmd(const char *cmd,int32_t timeout_ms);
void Wifi_ReceiveInit(void);
void Wifi_ParseDataStart(uint8_t *rx_buffer,uint16_t last_read_id,uint16_t size);
void Wifi_ReceiveData(uint16_t Size);
void Wifi_ConnectStart(void);
void Wifi_ConnectProcess(void);
void Wifi_ConnectAck(uint8_t* rbuf,int len);
bool Wifi_IsConnected(void);
void vSendToWifiTX(uint8_t * CmdDataArr,uint8_t len); //发送数据到WIFI TX






#endif /* ADAPTOR_INC_ADAPTOR_WIFI_H_ */
