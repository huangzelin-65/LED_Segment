#ifndef PERIPHERALS_INC_CAN_CAR_H_
#define PERIPHERALS_INC_CAN_CAR_H_


/* 头文件包含 */
#include "stm32h5xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* ===================== 核心宏定义 ===================== */
// CAN基础配置
#define CAN_BAUDRATE_33K               33000
#define CAN_MAX_RETRANSMIT             3       // 最大重传次数
#define CAN_FRAME_LEN                  8       // 固定8字节数据帧

// 设备ID范围
#define MASTER_ID_MIN                  1
#define MASTER_ID_MAX                  4095    // 16位主设备ID（支持400+）
#define CAR_FIXED_ID_MIN               1
#define CAR_FIXED_ID_MAX               300
#define CAR_TEMP_ID_MIN                1
#define CAR_TEMP_ID_MAX                10      // 单主最大并发10个小车

// 时限配置（ms）
#define AUTH_TIMEOUT_MS                200
#define HEARTBEAT_TIMEOUT_MS           900
#define CMD_ACK_TIMEOUT_MS             200
#define CMD_RETRY_CNT                  2       // 指令重试次数
#define STATUS_REPORT_INTERVAL_MS      1000    // 状态定时上报间隔

// CAN帧ID定义
#define CAN_ID_CAR_DISCOVER            0x001   // 小车发现帧（小→主）
#define CAN_ID_AUTH_RESP               0x002   // 认证响应帧（主→小）
#define CAN_ID_MASTER_HEART            0x003   // 主心跳帧（主→小）
#define CAN_ID_SLAVE_HEART             0x004   // 小车心跳帧（小→主）
#define CAN_ID_MASTER_CMD              0x005   // 主设备指令帧（主→小）
#define CAN_ID_CMD_ACK                 0x006   // 指令确认帧（小→主）
#define CAN_ID_CAR_STATUS              0x007   // 小车状态帧（小→主）

// 指令类型（Byte3）
#define CMD_TYPE_START_STOP            0x01    // 小车启停
#define CMD_TYPE_SPEED_ADJ             0x02    // 速度调节
#define CMD_TYPE_STATUS_REQUERY        0x03    // 状态重询
#define CMD_TYPE_FAULT_RESET           0x04    // 故障复位

// 状态类型（Byte3）
#define STATUS_TYPE_RUN                0x01    // 运行状态
#define STATUS_TYPE_SPEED              0x02    // 速度状态
#define STATUS_TYPE_FAULT              0x03    // 故障状态
#define STATUS_TYPE_POSITION           0x04    // 位置状态

// 指令确认码（Byte5）
#define ACK_CODE_SUCCESS               0x00    // 执行成功
#define ACK_CODE_PARSE_FAIL            0x01    // 解析失败
#define ACK_CODE_EXEC_FAIL             0x02    // 执行失败

/* ===================== 枚举定义 ===================== */
// 认证状态
typedef enum {
    AUTH_STATUS_UNKNOWN = 0,
    AUTH_STATUS_PENDING,
    AUTH_STATUS_SUCCESS,
    AUTH_STATUS_FAILED
} AuthStatus_t;

// 在线状态
typedef enum {
    ONLINE_STATUS_OFFLINE = 0,
    ONLINE_STATUS_ONLINE,
    ONLINE_STATUS_LOST
} OnlineStatus_t;

// 小车轨道状态
typedef enum {
    TRACK_STATUS_STABLE = 0,
    TRACK_STATUS_BOUNDARY,
    TRACK_STATUS_SWITCHING
} TrackStatus_t;

/* ===================== 结构体定义 ===================== */

// 主设备ID编解码结构体
typedef struct {
    uint8_t u8_high_byte;
    uint8_t u8_low_byte;
} MasterID_Encode_t;

// 主设备指令结构体
typedef struct {
    uint16_t u16_master_id;    // 主设备ID（16位）
    uint8_t u8_car_temp_id;    // 小车临时ID
    uint8_t u8_cmd_type;       // 指令类型
    uint8_t u8_cmd_seq;        // 指令序列号
    uint16_t u16_cmd_param;    // 指令参数
} MasterCmd_t;


// 主设备管理结构体（主设备端）
typedef struct {
    uint16_t u16_self_master_id;                 // 本机主设备ID
    uint8_t u8_temp_id_used[CAR_TEMP_ID_MAX+1];  // 临时ID占用状态（1~10）
    OnlineStatus_t en_car_online[CAR_TEMP_ID_MAX+1]; // 小车在线状态
    uint8_t u8_car_fixed_id[CAR_TEMP_ID_MAX+1];  // 临时ID→固有ID映射
    uint32_t u32_last_car_heart[CAR_TEMP_ID_MAX+1]; // 最后接收小车心跳时间
    uint8_t u8_cmd_seq;        // 指令序列号自增
} MasterManager_t;

// 小车上下文结构体（小车端）
typedef struct {
    uint16_t u16_fixed_id;             // 固有编号（1~300）
    uint16_t u16_cur_master_id;        // 当前关联主设备ID
    uint8_t u8_temp_id;                // 临时ID（0=未分配）
    AuthStatus_t en_auth_status;       // 认证状态
    OnlineStatus_t en_online_status;   // 在线状态
    TrackStatus_t en_track_status;     // 轨道状态
    uint32_t u32_last_master_heart;    // 最后接收主心跳时间
    uint8_t u8_status_seq;             // 状态序列号自增
    // 小车硬件状态
    uint8_t u8_run_status;             // 0=停止，1=运行，2=故障
    uint8_t u8_speed_level;            // 速度等级（1~10）
    uint16_t u16_fault_code;           // 故障码（0=无故障）
} CANCarCtx_t;



/* ===================== 全局声明 ===================== */

// FreeRTOS队列/信号量（全局）
extern SemaphoreHandle_t g_car_mutex;       // 小车上下文互斥量

// 设备上下文（全局）
extern CANCarCtx_t g_CAN_car_ctx;                  // 小车端

/* ===================== 公共函数声明 ===================== */
// 单线CAN主设备初始化（33Kbps）
void CAN_Car_Init(uint16_t u16_fixed_id);

// 主设备ID编解码
MasterID_Encode_t Encode_MasterID(uint16_t u16_master_id);
uint16_t Decode_MasterID(uint8_t u8_high_byte, uint8_t u8_low_byte);

// 校验位计算（Byte0~6异或和）
uint8_t Calc_CheckSum(uint8_t *pu8_data, uint8_t u8_len);

// 获取系统时间戳（ms）
uint32_t Get_SysTick_MS(void);

/* ===================== 核心函数声明 ===================== */
void Car_Send_DiscoverFrame(void);
void Car_Process_AuthRespFrame(uint8_t *pu8_rx_data);
void Car_Process_MasterHeartFrame(uint8_t *pu8_rx_data);
void Car_Process_MasterCmdFrame(uint8_t *pu8_rx_data);
void Car_Send_Heartbeat(void);
void Car_Send_Status(uint8_t u8_status_type);
void Car_Send_CmdAck(uint16_t u16_master_id, uint8_t u8_temp_id, uint8_t u8_cmd_type, uint8_t u8_cmd_seq, uint8_t u8_ack_code);
void Car_Track_Check(void);

#endif /* PERIPHERALS_INC_CAN_CAR_H_ */
