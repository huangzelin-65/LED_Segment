#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h"
#include <string.h>
#include "LogDebugInfo.h"
#include "app_freertos.h"
#include "Task_Can.h"
#include "adaptor_can.h"
#include "CAN_Car.h"

#define FILTER_MASK_ALL          (0x0)   // CAN接收所有ID的帧
#define FILTER_MASK_SPECIFIC     (0x7FF) // CAN接收指定ID的帧

#define TX_ID_1          (0x444)
#define RX_ID_1          (0x555)

#define TX_ID_2          (0x555)
#define RX_ID_2          (0x444)

extern osMessageQueueId_t xCAN1_Tx_QueueHandle;
extern osMessageQueueId_t xCAN1_Rx_QueueHandle;
extern osMessageQueueId_t xCAN2_Rx_QueueHandle;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;
extern CarStatus_t CarStatus;

uint8_t txData1[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x55,0xFF};
uint8_t txData2[8] = {0xFF,0x55,0x06,0x05,0x04,0x03,0x02,0x01};

// CAN管理任务
void vCANManagerTask(void *argument)
{
    DEBUGINFO("start");
#ifdef USE_FDCAN1
    CAN_Init(&hfdcan1, 0, FILTER_MASK_ALL, CAN_RX_FIFO0); 
#endif
#ifdef USE_FDCAN2
    CAN_Init(&hfdcan2, 0, FILTER_MASK_ALL, CAN_RX_FIFO1);
#endif

#ifdef CAN_TEST
    osThreadResume(FDCANTxTaskHandle);
    osThreadResume(FDCAN1RxTaskHandle);
    osThreadResume(FDCAN2RxTaskHandle);
#else
    osThreadResume(CANCarTxTaskHandle);
    osThreadResume(CANCarRxTaskHandle);
    // osThreadResume(CANCarHBTaskHandle);

    CAN_Car_Init(CarStatus.usCarID);
    osDelay(pdMS_TO_TICKS(50));

    while (g_CAN_car_ctx.en_online_status != ONLINE_STATUS_ONLINE)
    {
        // 上电发送发现帧
        Car_Send_DiscoverFrame();
        osDelay(pdMS_TO_TICKS(1000));
    }
#endif

    osThreadExit();
}

// CAN小车发送任务
void vCANCarTxTask(void *argument)
{
    DEBUGINFO("start");
    CAN_Send_Msg_t CAN_Send_Msg = {0};

    while (1)
    {
        if(xQueueReceive(xCAN1_Tx_QueueHandle, &CAN_Send_Msg, portMAX_DELAY) == pdPASS)
        {
#ifdef USE_FDCAN1
            CAN_AddMsgToTxFifo(&hfdcan1, CAN_Send_Msg.u32_frame_id, CAN_Send_Msg.u8_data);
#endif
#ifdef USE_FDCAN2
            CAN_AddMsgToTxFifo(&hfdcan2, CAN_Send_Msg.u32_frame_id, CAN_Send_Msg.u8_data);
#endif
        }
        // if(g_CAN_car_ctx.en_online_status == ONLINE_STATUS_ONLINE)
        // {
        //     Car_Send_Status(STATUS_TYPE_RUN);
        //     // Car_Send_Status(STATUS_TYPE_SPEED);
        //     osDelay(pdMS_TO_TICKS(STATUS_REPORT_INTERVAL_MS));
        // }
    }
}

// CAN小车接收任务
void vCANCarRxTask(void *argument)
{
    DEBUGINFO("start");
    CAN_Recv_Msg_t CAN_recv_msg = {0};

    while (1)
    {
        if(xQueueReceive(xCAN1_Rx_QueueHandle, &CAN_recv_msg, portMAX_DELAY) == pdPASS)
        {
            switch (CAN_recv_msg.u32_frame_id) {
                case CAN_ID_AUTH_RESP:
                    Car_Process_AuthRespFrame(CAN_recv_msg.u8_data);
                    break;
                case CAN_ID_MASTER_HEART:
                    Car_Process_MasterHeartFrame(CAN_recv_msg.u8_data);
                    break;
                case CAN_ID_MASTER_CMD:
                    Car_Process_MasterCmdFrame(CAN_recv_msg.u8_data);
                    break;
                default:
                    break;
            }
        }
        osDelay(pdMS_TO_TICKS(10));
    }
}

// CAN小车心跳发送任务
void vCANCarHBTask(void *argument)
{
    DEBUGINFO("start");

    while (1)
    {
        Car_Send_Heartbeat();
        Car_Track_Check();
        // osDelay(pdMS_TO_TICKS(200));
        osDelay(pdMS_TO_TICKS(2000));
    }
}


/* =========================================仅测试使用======================================== */
void vFDCANTxTask(void *argument)
{
    int swit = 1;

    DEBUGINFO("start");

    osDelay(pdMS_TO_TICKS(20));
    
    // 测试CAN接口时短接CAN1和CAN2，互相收发
    while (1)
    {
        if(swit)
            CAN_AddMsgToTxFifo(&hfdcan1, 0x555, txData1); // A
            // CAN_AddMsgToTxFifo(&hfdcan1, 0x111, txData1); // B
        else
            // CAN_AddMsgToTxFifo(&hfdcan2, 0x555, txData2); // A
            CAN_AddMsgToTxFifo(&hfdcan2, 0x111, txData2); // B
        
        swit = !swit;

        osDelay(pdMS_TO_TICKS(1000));
    }
    
}

void vFDCAN2RxTask(void *argument)
{
    DEBUGINFO("start");
    CAN_Recv_Msg_t recv_msg;

    while (1)
    {
        // 阻塞等待队列数据（portMAX_DELAY：永久等待）
        if(xQueueReceive(xCAN1_Rx_QueueHandle, &recv_msg, portMAX_DELAY) == pdPASS)
        {
        // 打印接收信息（可替换为自定义解析逻辑）
        DEBUGINFO("FDCAN%d Recv <- ID:0x%03lX, Len:%d, "
            "Data:0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n",
                recv_msg.u8_can_id, recv_msg.u32_frame_id, recv_msg.u8_len, 
                recv_msg.u8_data[0],recv_msg.u8_data[1],recv_msg.u8_data[2],recv_msg.u8_data[3],recv_msg.u8_data[4],recv_msg.u8_data[5],recv_msg.u8_data[6],recv_msg.u8_data[7]);
        }
    }
}

void vFDCAN1RxTask(void *argument)
{
    DEBUGINFO("start");
    CAN_Recv_Msg_t recv_msg;

    while (1)
    {
        // 阻塞等待队列数据（portMAX_DELAY：永久等待）
        if(xQueueReceive(xCAN2_Rx_QueueHandle, &recv_msg, portMAX_DELAY) == pdPASS)
        {
        // 打印接收信息（可替换为自定义解析逻辑）
        DEBUGINFO("FDCAN%d Recv <- ID:0x%03lX, Len:%d, "
            "Data:0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\r\n",
                recv_msg.u8_can_id, recv_msg.u32_frame_id, recv_msg.u8_len, 
                recv_msg.u8_data[0],recv_msg.u8_data[1],recv_msg.u8_data[2],recv_msg.u8_data[3],recv_msg.u8_data[4],recv_msg.u8_data[5],recv_msg.u8_data[6],recv_msg.u8_data[7]);
        }
    }
}
/* =========================================仅测试使用======================================== */


// FIFO0 接收队列回调函数
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0U)
    {
        FDCAN_RxHeaderTypeDef RxHeader = {0};
        uint8_t RxData[8] = {0};
        CAN_Recv_Msg_t recv_msg = {0};
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        // 1. 读取FIFO0中的报文（HAL库已确保中断标志有效）
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
        {
            Error_Handler();
        }
        // 2. 区分FDCAN外设，标记报文来源
        if(hfdcan->Instance == FDCAN1)
        {
            recv_msg.u8_can_id = 1;
        }
        else if(hfdcan->Instance == FDCAN2)
        {
            recv_msg.u8_can_id = 2;
        }

        // 3. 填充报文信息
        recv_msg.u32_frame_id = RxHeader.Identifier;       // 标准ID
        recv_msg.u8_len = RxHeader.DataLength; 
        memcpy(recv_msg.u8_data, RxData, recv_msg.u8_len); // 数据域

        // 4. 发送到FreeRTOS队列（中断安全版本）
        if(xCAN1_Rx_QueueHandle != NULL)
        {
            xQueueSendFromISR(xCAN1_Rx_QueueHandle, &recv_msg, &xHigherPriorityTaskWoken);
        }
        
        // 5. 若高优先级任务被唤醒，触发任务调度（中断上下文）
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

// FIFO1 接收队列回调函数
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != 0U)
    {
        FDCAN_RxHeaderTypeDef RxHeader = {0};
            uint8_t RxData[8] = {0};
            CAN_Recv_Msg_t recv_msg = {0};
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &RxHeader, RxData) != HAL_OK)
        {
            Error_Handler();
        }
        // 2. 区分FDCAN外设，标记报文来源
        if(hfdcan->Instance == FDCAN1)
        {
            recv_msg.u8_can_id = 1;
        }
        else if(hfdcan->Instance == FDCAN2)
        {
            recv_msg.u8_can_id = 2;
        }

        // 3. 填充报文信息
        recv_msg.u32_frame_id = RxHeader.Identifier;       // 标准ID
        recv_msg.u8_len = RxHeader.DataLength;
        memcpy(recv_msg.u8_data, RxData, recv_msg.u8_len); // 数据域

        // 4. 发送到FreeRTOS队列（中断安全版本）
        if(xCAN1_Rx_QueueHandle != NULL)
        {
            xQueueSendFromISR(xCAN1_Rx_QueueHandle, &recv_msg, &xHigherPriorityTaskWoken);
        }
        
        // 5. 若高优先级任务被唤醒，触发任务调度（中断上下文）
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

