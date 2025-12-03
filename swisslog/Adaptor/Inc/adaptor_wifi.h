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

#define WIFI_ARRAY_LEN 30  //保存解析信息的长度
// #define WIFI_USE_MALLOC   //开启之后，中断里malloc消息长度，线程中解析（虽然malloc内存很小，但存在风险）

typedef enum
{
    WIFI_IDLE = 0x00,
    WIFI_AT,
    WIFI_TPMODE_EXIT_1,
    WIFI_TPMODE_EXIT_2,
    WIFI_CHECK_CONNET, 
    WIFI_SET_CONNECT,    
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


typedef struct 
{ 
    int rssi;
    WifiResult_t connect_state;//wifi是否成功连接到热点
    char ip[36];    
}WifiStatus_t;

typedef enum
{
    WIFI_POWER_OFF = 0,
    WIFI_POWER_ON,
} WifiPower_t;

extern WifiStatus_t wifi_status;
extern WifiParseData_t wifi_parse_array[WIFI_ARRAY_LEN];

HAL_StatusTypeDef Wifi_SendATCmd(const char *cmd,int32_t timeout_ms);
void Wifi_ReceiveInit(void);
void Wifi_ParseDataStart(uint8_t *rx_buffer,uint16_t last_read_id,uint16_t size);
void Wifi_ReceiveData(uint16_t Size);
void Wifi_ConnectStart(void);
void Wifi_ConnectProcess(void);
void Wifi_ConnectAck(uint8_t* rbuf,int len);
bool Wifi_IsConnected(void);
bool Wifi_IsChanged(void);
void Wifi_Init(void);
void Wifi_SetPower(WifiPower_t power);
void Wifi_OtaProcess(void);



#endif /* ADAPTOR_INC_ADAPTOR_WIFI_H_ */
