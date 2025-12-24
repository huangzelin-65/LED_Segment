#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include "LogDebugInfo.h"
#include "Task_Can.h"
#include "adaptor_can.h"
#include "CAN_Car.h"


extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;


// CAN1 配置接收过滤器
void CAN_FilterConfig(FDCAN_HandleTypeDef *hfdcan, 
                        uint16_t u16_Rx_id, 
                        uint16_t u16_MaskID,
                        eFifoType xFifo)
{
    DEBUGINFO("u16_Rx_id = 0x%lX, u16_MaskID = 0x%lX, FIFO = %d",u16_Rx_id, u16_MaskID);

    FDCAN_FilterTypeDef sFilterConfig;
    sFilterConfig.IdType       = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex  = 0U;
    sFilterConfig.FilterType   = FDCAN_FILTER_MASK;

    // 选择使用的FIFO
    if(CAN_RX_FIFO0 == xFifo)
    {
        sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    } else {
        sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
    }

    sFilterConfig.FilterID1    = u16_Rx_id;
    sFilterConfig.FilterID2    = u16_MaskID; 

    if (HAL_FDCAN_ConfigFilter(hfdcan, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    // 选择使用的FIFO
    if(CAN_RX_FIFO0 == xFifo)
    {
        if (HAL_FDCAN_ConfigGlobalFilter(hfdcan,
                                    FDCAN_ACCEPT_IN_RX_FIFO0, FDCAN_REJECT,
                                    FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
        {
            DEBUGINFO("HAL_FDCAN_ConfigGlobalFilter Fail !!");
            osDelay(pdMS_TO_TICKS(100));
            Error_Handler();
        } 
    } else {
        if (HAL_FDCAN_ConfigGlobalFilter(hfdcan,
                                    FDCAN_ACCEPT_IN_RX_FIFO1, FDCAN_REJECT,
                                    FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK)
        {
            DEBUGINFO("HAL_FDCAN_ConfigGlobalFilter Fail !!");
            osDelay(pdMS_TO_TICKS(100));
            Error_Handler();
        } 
    }

}


void CAN_RxFifoNotify_Activate(FDCAN_HandleTypeDef *hfdcan, 
                                eFifoType xFifo)
{
    DEBUGINFO("start");
    if(CAN_RX_FIFO0 == xFifo)
    {
        // 启用 FIFO0 接收中断
        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U) != HAL_OK)
        {
            DEBUGINFO("HAL_FDCAN_ActivateNotification Fail !!");
            osDelay(pdMS_TO_TICKS(100));
            Error_Handler();
        }
    } else {
        // 启用 FIFO1 接收中断
        if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0U) != HAL_OK)
        {
            DEBUGINFO("HAL_FDCAN_ActivateNotification Fail !!");
            osDelay(pdMS_TO_TICKS(100));
            Error_Handler();
        }
    }
}


void CAN_Start(FDCAN_HandleTypeDef *hfdcan)
{
    DEBUGINFO("Start");
    if (HAL_FDCAN_Start(hfdcan) != HAL_OK)
    {
        DEBUGINFO("Can_Start Fail !!");
        osDelay(pdMS_TO_TICKS(100));
        Error_Handler();
    }
}


void CAN_AddMsgToTxFifo(FDCAN_HandleTypeDef *hfdcan, 
                        uint16_t CAN_Tx_ID, 
                        uint8_t *pucCAN_Tx_Data)
{
    DEBUGINFO("start");
    FDCAN_TxHeaderTypeDef txHeader;

    txHeader.Identifier          = CAN_Tx_ID;
    txHeader.IdType              = FDCAN_STANDARD_ID;
    txHeader.TxFrameType         = FDCAN_DATA_FRAME;
    txHeader.DataLength          = FDCAN_DLC_BYTES_8;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch       = FDCAN_BRS_OFF;
    txHeader.FDFormat            = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker       = 0U;
    if (HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, &txHeader, pucCAN_Tx_Data) != HAL_OK)
    {
        DEBUGINFO("HAL_FDCAN_AddMessageToTxFifoQ Fail !!");
        osDelay(pdMS_TO_TICKS(100));
        Error_Handler();
    }
}


void CAN_Init(FDCAN_HandleTypeDef *hfdcan, 
                        uint16_t u16_Rx_id, 
                        uint16_t u16_MaskID, 
                        eFifoType xFifo)
{
    DEBUGINFO("start");
    CAN_FilterConfig(hfdcan, u16_Rx_id, u16_MaskID, xFifo);
    CAN_RxFifoNotify_Activate(hfdcan, xFifo);
    CAN_Start(hfdcan);
}
