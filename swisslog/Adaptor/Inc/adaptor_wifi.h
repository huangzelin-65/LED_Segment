#ifndef ADAPTOR_INC_ADAPTOR_WIFI_H_
#define ADAPTOR_INC_ADAPTOR_WIFI_H_

// 缓冲区大小
#define WIFI_TX_BUF_SIZE   128
#define WIFI_RX_BUF_SIZE   128

/* 配置参数定义 */
#define WIFI_STATIC_IP     "192.168.1.10"
#define WIFI_GATEWAY       "192.168.1.1"
#define WIFI_NETMASK       "255.255.1.0"
#define WIFI_SSID          "DianKong"  
#define WIFI_PASSWORD      "12345678"  
#define WIFI_SERVER_IP     "192.168.1.108" 
#define WIFI_SERVER_PORT   "9100"

// 序号+PLC编号 所用字节数
#define CMD_BASE_COUNT 6

// 数据帧结构体（包含数据和长度）
typedef struct {
    uint8_t data[WIFI_TX_BUF_SIZE];
    uint16_t len;
} Wifi_Tx_Frame_t;

typedef struct {
    uint8_t data[WIFI_RX_BUF_SIZE];
    uint16_t len;
} Wifi_Rx_Frame_t;


// 函数声明
void vWifi_Start_GPDMA_Receive(void);
void vWifi_Stop_GPDMA_Receive(void);
void vWifi_RxEventCallback(uint16_t Size);
void vSendToWifiTX(uint8_t * CmdDataArr,uint8_t len); //发送数据到WIFI TX

HAL_StatusTypeDef at_send_command(const char *cmd, const char *expect, uint32_t timeout_ms);
HAL_StatusTypeDef wb502a_connect_ap(void);
HAL_StatusTypeDef wb502a_check_ip(const char *ip_addr);



#endif /* ADAPTOR_INC_ADAPTOR_WIFI_H_ */
