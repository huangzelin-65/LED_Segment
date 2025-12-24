#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h"
#include "CAN_Car.h"
#include "adaptor_can.h"
#include "LogDebugInfo.h"
#include "string.h"

/* ===================== 全局变量定义 ===================== */
extern FDCAN_HandleTypeDef hfdcan1;
extern osMessageQueueId_t xCAN1_Rx_QueueHandle;
extern osMessageQueueId_t xCAN1_Tx_QueueHandle;
SemaphoreHandle_t g_car_mutex;
CANCarCtx_t g_CAN_car_ctx = {0};

/* ===================== 核心函数声明 ===================== */
void Car_Send_DiscoverFrame(void);
void Car_Process_AuthRespFrame(uint8_t *pu8_rx_data);
void Car_Process_MasterHeartFrame(uint8_t *pu8_rx_data);
void Car_Process_MasterCmdFrame(uint8_t *pu8_rx_data);
void Car_Send_Heartbeat(void);
void Car_Send_Status(uint8_t u8_status_type);
void Car_Send_CmdAck(uint16_t u16_master_id, uint8_t u8_temp_id, uint8_t u8_cmd_type, uint8_t u8_cmd_seq, uint8_t u8_ack_code);
void Car_Track_Check(void);

/* ===================== 工具函数 ===================== */
// 主设备ID编码（16位→高低字节）
MasterID_Encode_t Encode_MasterID(uint16_t u16_master_id) {
    MasterID_Encode_t st_encode;
    st_encode.u8_high_byte = (u16_master_id >> 8) & 0xFF;
    st_encode.u8_low_byte = u16_master_id & 0xFF;
    return st_encode;
}

// 主设备ID解码（高低字节→16位）
uint16_t Decode_MasterID(uint8_t u8_high_byte, uint8_t u8_low_byte) {
    return ((uint16_t)u8_high_byte << 8) | u8_low_byte;
}

// 校验位计算（Byte0~6异或和）
uint8_t Calc_CheckSum(uint8_t *pu8_data, uint8_t u8_len) {
    uint8_t u8_check_sum = 0;
    for (uint8_t u8_i = 0; u8_i < u8_len; u8_i++) {
        u8_check_sum ^= pu8_data[u8_i];
    }
    return u8_check_sum;
}

// 获取系统时间戳（ms）
uint32_t Get_SysTick_MS(void) {
    return xTaskGetTickCount();
}

/* ===================== 核心帧处理函数 ===================== */
// 发送小车发现帧（0x001）
void Car_Send_DiscoverFrame(void) {
    DEBUGINFO("Frame 0x001 start");
    CAN_Send_Msg_t CAN_Send_Msg = {0};

    // 配置小车发现帧的id
    CAN_Send_Msg.u32_frame_id = CAN_ID_CAR_DISCOVER;

    // 填充固有编号
    CAN_Send_Msg.u8_data[0] = (g_CAN_car_ctx.u16_fixed_id >> 8) & 0xFF;
    CAN_Send_Msg.u8_data[1] = g_CAN_car_ctx.u16_fixed_id & 0xFF;
    CAN_Send_Msg.u8_data[7] = Calc_CheckSum(CAN_Send_Msg.u8_data, 7);


    // 放入发送队列
    if(xQueueSend(xCAN1_Tx_QueueHandle, &CAN_Send_Msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        // 发送失败
        DEBUGINFO("CAN Send DiscoverFrame Fail");
    }
}

// 处理认证响应帧（0x002）
void Car_Process_AuthRespFrame(uint8_t *pu8_rx_data) {
    DEBUGINFO("Frame 0x002 start");
    if (pu8_rx_data[7] != Calc_CheckSum(pu8_rx_data, 7)) {
        return;
    }

    // 校验固有编号
    uint16_t u16_car_fixed_id = ((uint16_t)pu8_rx_data[0] << 8) | pu8_rx_data[1];
    if (u16_car_fixed_id != g_CAN_car_ctx.u16_fixed_id) {
        return;
    }

    xSemaphoreTake(g_car_mutex, portMAX_DELAY);
    // 临时ID有效则认证成功
    if (pu8_rx_data[2] >= CAR_TEMP_ID_MIN && pu8_rx_data[2] <= CAR_TEMP_ID_MAX) {
        g_CAN_car_ctx.u8_temp_id = pu8_rx_data[2];
        g_CAN_car_ctx.u16_cur_master_id = Decode_MasterID(pu8_rx_data[3], pu8_rx_data[4]);
        g_CAN_car_ctx.en_auth_status = AUTH_STATUS_SUCCESS;
        g_CAN_car_ctx.en_online_status = ONLINE_STATUS_ONLINE;
        DEBUGINFO("Auth Success !!");
        DEBUGINFO("temp ID: %d, Master ID: %d", g_CAN_car_ctx.u8_temp_id, g_CAN_car_ctx.u16_cur_master_id);
    } else {
        g_CAN_car_ctx.en_auth_status = AUTH_STATUS_FAILED;
    }
    xSemaphoreGive(g_car_mutex);
}

// 处理主设备心跳帧（0x003）
void Car_Process_MasterHeartFrame(uint8_t *pu8_rx_data) {
    DEBUGINFO("Frame 0x003 start");
    if (pu8_rx_data[7] != Calc_CheckSum(pu8_rx_data, 7)) {
        return;
    }

    uint16_t u16_master_id = Decode_MasterID(pu8_rx_data[0], pu8_rx_data[1]);
    if (u16_master_id != g_CAN_car_ctx.u16_cur_master_id) {
        return;
    }

    xSemaphoreTake(g_car_mutex, portMAX_DELAY);
    g_CAN_car_ctx.u32_last_master_heart = Get_SysTick_MS();
    g_CAN_car_ctx.en_online_status = ONLINE_STATUS_ONLINE;
    xSemaphoreGive(g_car_mutex);
}

// 发送小车心跳帧（0x004）
void Car_Send_Heartbeat(void) {
    DEBUGINFO("Frame 0x004 start");
    xSemaphoreTake(g_car_mutex, portMAX_DELAY);
    if (g_CAN_car_ctx.en_online_status != ONLINE_STATUS_ONLINE) {
        xSemaphoreGive(g_car_mutex);
        return;
    }

    CAN_Send_Msg_t CAN_Send_Msg = {0};
    MasterID_Encode_t st_master_id_encode = Encode_MasterID(g_CAN_car_ctx.u16_cur_master_id);
    uint32_t u32_tick = Get_SysTick_MS();

    // 配置小车心跳帧的id
    CAN_Send_Msg.u32_frame_id = CAN_ID_SLAVE_HEART;

    CAN_Send_Msg.u8_data[0] = (g_CAN_car_ctx.u16_fixed_id >> 8) & 0xFF;
    CAN_Send_Msg.u8_data[1] = g_CAN_car_ctx.u16_fixed_id & 0xFF;
    CAN_Send_Msg.u8_data[2] = g_CAN_car_ctx.u8_temp_id;
    CAN_Send_Msg.u8_data[3] = st_master_id_encode.u8_high_byte;
    CAN_Send_Msg.u8_data[4] = st_master_id_encode.u8_low_byte;
    CAN_Send_Msg.u8_data[5] = (u32_tick >> 8) & 0xFF;
    CAN_Send_Msg.u8_data[6] = u32_tick & 0xFF;
    CAN_Send_Msg.u8_data[7] = Calc_CheckSum(CAN_Send_Msg.u8_data, 7);

    // 放入发送队列
    if(xQueueSend(xCAN1_Tx_QueueHandle, &CAN_Send_Msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        // 发送失败
        DEBUGINFO("CAN Send Heartbeat Fail");
    }
    xSemaphoreGive(g_car_mutex);
}

// 处理主设备指令帧（0x005）
void Car_Process_MasterCmdFrame(uint8_t *pu8_rx_data) {
    DEBUGINFO("Frame 0x005 start");
    if (pu8_rx_data[7] != Calc_CheckSum(pu8_rx_data, 7)) {
        Car_Send_CmdAck(Decode_MasterID(pu8_rx_data[0], pu8_rx_data[1]), pu8_rx_data[2], pu8_rx_data[3], pu8_rx_data[4], ACK_CODE_PARSE_FAIL);
        return;
    }

    uint16_t u16_master_id = Decode_MasterID(pu8_rx_data[0], pu8_rx_data[1]);
    uint8_t u8_temp_id = pu8_rx_data[2];
    
    xSemaphoreTake(g_car_mutex, portMAX_DELAY);
    // 仅处理当前关联主设备+自身临时ID
    if (u16_master_id != g_CAN_car_ctx.u16_cur_master_id || u8_temp_id != g_CAN_car_ctx.u8_temp_id) {
        xSemaphoreGive(g_car_mutex);
        return;
    }

    uint8_t u8_ack_code = ACK_CODE_SUCCESS;
    switch (pu8_rx_data[3]) {
        case CMD_TYPE_START_STOP:
            if (pu8_rx_data[5] == 1) {
                g_CAN_car_ctx.u8_run_status = 1; // 启动
            } else if (pu8_rx_data[5] == 0) {
                g_CAN_car_ctx.u8_run_status = 0; // 停止
            } else {
                u8_ack_code = ACK_CODE_EXEC_FAIL;
            }
            break;
        case CMD_TYPE_SPEED_ADJ:
            if (pu8_rx_data[5] >= 1 && pu8_rx_data[5] <= 10) {
                g_CAN_car_ctx.u8_speed_level = pu8_rx_data[5];
            } else {
                u8_ack_code = ACK_CODE_EXEC_FAIL;
            }
            break;
        case CMD_TYPE_STATUS_REQUERY:
            Car_Send_Status(STATUS_TYPE_RUN); // 立即上报运行状态
            break;
        default:
            u8_ack_code = ACK_CODE_PARSE_FAIL;
            break;
    }
    xSemaphoreGive(g_car_mutex);

    // 发送确认帧
    Car_Send_CmdAck(u16_master_id, u8_temp_id, pu8_rx_data[3], pu8_rx_data[4], u8_ack_code);
}

// 发送指令确认帧（0x006）
void Car_Send_CmdAck(uint16_t u16_master_id, 
                        uint8_t u8_temp_id, 
                        uint8_t u8_cmd_type, 
                        uint8_t u8_cmd_seq, 
                        uint8_t u8_ack_code) 
{
    DEBUGINFO("Frame 0x006 start");
    CAN_Send_Msg_t CAN_Send_Msg = {0};
    MasterID_Encode_t st_master_id_encode = Encode_MasterID(u16_master_id);

    // 配置指令确认帧的id
    CAN_Send_Msg.u32_frame_id = CAN_ID_CMD_ACK;

    CAN_Send_Msg.u8_data[0] = st_master_id_encode.u8_high_byte;
    CAN_Send_Msg.u8_data[1] = st_master_id_encode.u8_low_byte;
    CAN_Send_Msg.u8_data[2] = u8_temp_id;
    CAN_Send_Msg.u8_data[3] = u8_cmd_type;
    CAN_Send_Msg.u8_data[4] = u8_cmd_seq;
    CAN_Send_Msg.u8_data[5] = u8_ack_code;
    CAN_Send_Msg.u8_data[7] = Calc_CheckSum(CAN_Send_Msg.u8_data, 7);

    // 放入发送队列
    if(xQueueSend(xCAN1_Tx_QueueHandle, &CAN_Send_Msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        // 发送失败
        DEBUGINFO("Car Send CmdAck Fail");
    }
}

// 发送小车状态帧（0x007）
void Car_Send_Status(uint8_t u8_status_type) {
    DEBUGINFO("Frame 0x007 start");
    xSemaphoreTake(g_car_mutex, portMAX_DELAY);
    if (g_CAN_car_ctx.en_online_status != ONLINE_STATUS_ONLINE) {
        xSemaphoreGive(g_car_mutex);
        return;
    }

    CAN_Send_Msg_t CAN_Send_Msg = {0};
    MasterID_Encode_t st_master_id_encode = Encode_MasterID(g_CAN_car_ctx.u16_cur_master_id);

    // 配置小车状态帧的id
    CAN_Send_Msg.u32_frame_id = CAN_ID_CAR_STATUS;

    // 填充固定字段
    CAN_Send_Msg.u8_data[0] = st_master_id_encode.u8_high_byte;
    CAN_Send_Msg.u8_data[1] = st_master_id_encode.u8_low_byte;
    CAN_Send_Msg.u8_data[2] = g_CAN_car_ctx.u8_temp_id;
    CAN_Send_Msg.u8_data[3] = u8_status_type;
    CAN_Send_Msg.u8_data[4] = g_CAN_car_ctx.u8_status_seq++;
    if (g_CAN_car_ctx.u8_status_seq > 255) g_CAN_car_ctx.u8_status_seq = 0;

    // 填充状态值
    switch (u8_status_type) {
        case STATUS_TYPE_RUN:
            CAN_Send_Msg.u8_data[5] = g_CAN_car_ctx.u8_run_status;
            break;
        case STATUS_TYPE_SPEED:
            CAN_Send_Msg.u8_data[5] = g_CAN_car_ctx.u8_speed_level;
            break;
        case STATUS_TYPE_FAULT:
            CAN_Send_Msg.u8_data[5] = (g_CAN_car_ctx.u16_fault_code >> 8) & 0xFF;
            CAN_Send_Msg.u8_data[6] = g_CAN_car_ctx.u16_fault_code & 0xFF;
            break;
        default:
            xSemaphoreGive(g_car_mutex);
            return;
    }

    CAN_Send_Msg.u8_data[7] = Calc_CheckSum(CAN_Send_Msg.u8_data, 7);
    // 放入发送队列
    if(xQueueSend(xCAN1_Tx_QueueHandle, &CAN_Send_Msg, pdMS_TO_TICKS(100)) != pdTRUE) {
        // 发送失败
        DEBUGINFO("Car Send Status Fail");
    }
    xSemaphoreGive(g_car_mutex);
}

// 轨道状态检查（基于主心跳超时）
void Car_Track_Check(void) {
    DEBUGINFO("start");
    xSemaphoreTake(g_car_mutex, portMAX_DELAY);
    if (g_CAN_car_ctx.en_online_status == ONLINE_STATUS_ONLINE) {
        if (Get_SysTick_MS() - g_CAN_car_ctx.u32_last_master_heart > HEARTBEAT_TIMEOUT_MS) {
            // 主心跳超时，标记轨道切换
            g_CAN_car_ctx.en_online_status = ONLINE_STATUS_LOST;
            g_CAN_car_ctx.en_track_status = TRACK_STATUS_SWITCHING;
            g_CAN_car_ctx.u8_temp_id = 0; // 清空临时ID
            // 进入新轨道后重新发送发现帧
            Car_Send_DiscoverFrame();
        }
    }
    xSemaphoreGive(g_car_mutex);
}

/* ===================== CAN初始化（复用33Kbps配置） ===================== */
// 小车初始化
void CAN_Car_Init(uint16_t u16_fixed_id) {
    DEBUGINFO("start");
    // 初始化小车上下文
    g_CAN_car_ctx.u16_fixed_id = u16_fixed_id;
    g_CAN_car_ctx.en_auth_status = AUTH_STATUS_PENDING;
    g_CAN_car_ctx.en_track_status = TRACK_STATUS_STABLE;

    // 初始化CAN和FreeRTOS资源
    g_car_mutex = xSemaphoreCreateMutex();

}