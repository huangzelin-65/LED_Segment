#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h"
#include <string.h>
#include "LogDebugInfo.h"
#include "Task_Can.h"
#include "adaptor_can.h"

#define TX_ID_1          (0x444)
#define RX_ID_1          (0x555)

#define TX_ID_2          (0x555)
#define RX_ID_2          (0x444)

extern osMessageQueueId_t xCAN1_Rx_QueueHandle;
extern osMessageQueueId_t xCAN2_Rx_QueueHandle;

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

uint8_t txData1[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x55,0xFF};
uint8_t txData2[8] = {0xFF,0x55,0x06,0x05,0x04,0x03,0x02,0x01};

void vFDCANTxTask(void *argument)
{
    int swit = 1;

    DEBUGINFO("start");
    // CAN_Init(&hfdcan1, 0x111, CAN_RX_FIFO0); // A
    CAN_Init(&hfdcan1, 0x555, CAN_RX_FIFO0); // B
    // CAN_Init(&hfdcan2, 0x111, CAN_RX_FIFO1);  // A
    // CAN_Init(&hfdcan2, 0x555, CAN_RX_FIFO1); // B

    osDelay(pdMS_TO_TICKS(20));
    
    while (1)
    {
        // if(swit)
            // CAN_AddMsgToTxFifo(&hfdcan1, 0x555, txData1); // A
            // CAN_AddMsgToTxFifo(&hfdcan1, 0x111, txData1); // B
        // else
            // CAN_AddMsgToTxFifo(&hfdcan2, 0x555, txData2); // A
            // CAN_AddMsgToTxFifo(&hfdcan2, 0x111, txData2); // B
        
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
                recv_msg.can_id, recv_msg.std_id, recv_msg.len, 
                recv_msg.data[0],recv_msg.data[1],recv_msg.data[2],recv_msg.data[3],recv_msg.data[4],recv_msg.data[5],recv_msg.data[6],recv_msg.data[7]);
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
                recv_msg.can_id, recv_msg.std_id, recv_msg.len, 
                recv_msg.data[0],recv_msg.data[1],recv_msg.data[2],recv_msg.data[3],recv_msg.data[4],recv_msg.data[5],recv_msg.data[6],recv_msg.data[7]);
        }
    }
}

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
            recv_msg.can_id = 1;
        }
        else if(hfdcan->Instance == FDCAN2)
        {
            recv_msg.can_id = 2;
        }

        // 3. 填充报文信息
        recv_msg.std_id = RxHeader.Identifier;       // 标准ID
        recv_msg.len = RxHeader.DataLength; 
        memcpy(recv_msg.data, RxData, recv_msg.len); // 数据域

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
            recv_msg.can_id = 1;
        }
        else if(hfdcan->Instance == FDCAN2)
        {
            recv_msg.can_id = 2;
        }

        // 3. 填充报文信息
        recv_msg.std_id = RxHeader.Identifier;       // 标准ID
        recv_msg.len = RxHeader.DataLength;
        memcpy(recv_msg.data, RxData, recv_msg.len); // 数据域

        // 4. 发送到FreeRTOS队列（中断安全版本）
        if(xCAN1_Rx_QueueHandle != NULL)
        {
            xQueueSendFromISR(xCAN1_Rx_QueueHandle, &recv_msg, &xHigherPriorityTaskWoken);
        }
        
        // 5. 若高优先级任务被唤醒，触发任务调度（中断上下文）
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

