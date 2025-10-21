#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "task.h"
#include "app_freertos.h"


extern osMessageQueueId_t xSensor_QueueHandle;
extern CarStatus_t CarStatus;
extern CarToServerData_t CarToServerData;

/**
 * 传感器状态检查函数
 *
 * 该函数负责读取并检查各个传感器的状态，包括前碰撞、后碰撞、前近距离和后近距离传感器。
 * 根据传感器状态，更新CarStatus结构体中的相应状态标志。
 * 如果任何一个传感器触发或释放，发送SensorEvent消息到motion_Queue。
 */
void vSensorStatusCheck(void)
{
  volatile uint8_t FC_H_value;
  volatile uint8_t FC_L_value;
  volatile uint8_t RC_H_value;
  volatile uint8_t RC_L_value;
  volatile uint8_t FP_H_value;
  volatile uint8_t FP_L_value;
  volatile uint8_t RP_H_value;
  volatile uint8_t RP_L_value;
  uint8_t sensor_msg;

  // 读FC_H电平
  FC_H_value = GPIO_READ(FC_H);
  // 读FC_L电平
  FC_L_value = GPIO_READ(FC_L);
  // 读RC_H电平
  RC_H_value = GPIO_READ(RC_H);
  // 读RC_L电平
  RC_L_value = GPIO_READ(RC_L);

  // 读FP_H电平
  FP_H_value = GPIO_READ(FP_H);
  // 读FP_L电平
  FP_L_value = GPIO_READ(FP_L);
  // 读RP_H电平
  RP_H_value = GPIO_READ(RP_H);
  // 读RP_L电平
  RP_L_value = GPIO_READ(RP_L);

  DEBUGINFO("FC_H = %d, FC_L = %d, RC_H = %d, RC_L = %d,\r\n",FC_H_value,FC_L_value,RC_H_value,RC_L_value);
  DEBUGINFO("FP_H = %d, FP_L = %d, RP_H = %d, RP_L = %d,\r\n",FP_H_value,FP_L_value,RP_H_value,RP_L_value);

  if ((FC_H_value == GPIO_PIN_RESET)&&(FC_L_value == GPIO_PIN_SET)) //FC Trigger
  {
    CarStatus.FrontCrashStatus = SensorTrigger;
    CarToServerData.wSta |= 1 << 4;
  }
  if ((FC_H_value == GPIO_PIN_SET)&&(FC_L_value == GPIO_PIN_RESET)) //FC Release
  {
    CarStatus.FrontCrashStatus = SensorRelease;
    CarToServerData.wSta &= ~(1 << 4);
  }
  if ((RC_H_value == GPIO_PIN_RESET)&&(RC_L_value == GPIO_PIN_SET)) //RC Trigger
  {
    CarStatus.RearCrashStatus = SensorTrigger;
    CarToServerData.wSta |= 1 << 5;
  }
  if ((RC_H_value == GPIO_PIN_SET)&&(RC_L_value == GPIO_PIN_RESET)) //RC Release
  {
    CarStatus.RearCrashStatus = SensorRelease;
    CarToServerData.wSta &= ~(1 << 5);
  }
  if ((FP_H_value == GPIO_PIN_RESET)&&(FP_L_value == GPIO_PIN_SET)) //FP Trigger
  {
    CarStatus.FrontProxStatus = SensorTrigger;
    CarToServerData.wSta |= 1 << 2;
  }
  if ((FP_H_value == GPIO_PIN_SET)&&(FP_L_value == GPIO_PIN_RESET)) //FP Release
  {
    CarStatus.FrontProxStatus = SensorRelease;
    CarToServerData.wSta &= ~(1 << 2);
  }
  if ((RP_H_value == GPIO_PIN_RESET)&&(RP_L_value == GPIO_PIN_SET)) //RP Trigger
  {
    CarStatus.RearProxStatus = SensorTrigger;
    CarToServerData.wSta |= 1 << 3;
  }
  if ((RP_H_value == GPIO_PIN_SET)&&(RP_L_value == GPIO_PIN_RESET)) //RP Release
  {
    CarStatus.RearProxStatus = SensorRelease;
    CarToServerData.wSta &= ~(1 << 3);
  }

  // 发送消息到xSensor_Queue
  sensor_msg = SensorEvent;//碰撞触发
  //DEBUGINFO("send Event to Sensor_Queue\r\n");
  if(osMessageQueuePut(xSensor_QueueHandle, &sensor_msg, 0, pdMS_TO_TICKS(100)) != osOK)
  {
      DEBUGINFO("send motion error 1\r\n");
  }

}


/**
 * 拨动开关状态检查函数
 *
 * 该函数用于检查MOVE_FRONT和MOVE_BACK电平的状态，根据状态设置motion_msg和CarStatus.ToggleSwtichPosition的值。
 *
 * @note 该函数会发送拨动开关时事件到motion_QueueHandle队列中。
 *
 * @return 无返回值
 */
void vToggleSwitchStatusCheck(void)
{
  volatile uint8_t TOGGLE_FRONT_value;
  volatile uint8_t TOGGLE_BACK_value;
  uint8_t sensor_msg;

  //低电平有效
  // 读MOVE_FRONT电平
  TOGGLE_FRONT_value = GPIO_READ(TOGGLE_FRONT);
  // 读MOVE_BACK电平
  TOGGLE_BACK_value = GPIO_READ(TOGGLE_BACK);

  //拨动开关拨向前
  if((TOGGLE_FRONT_value == GPIO_PIN_RESET)&&(TOGGLE_BACK_value == GPIO_PIN_SET))
  {
    sensor_msg = ToggleFront;
    CarStatus.ToggleSwtichPosition = ToggleFront;
    DEBUGINFO("motion_msg = ToggleFront\r\n");
  }
  //拨动开关拨向后
  else if((TOGGLE_FRONT_value == GPIO_PIN_SET)&&(TOGGLE_BACK_value == GPIO_PIN_RESET))
  {
    sensor_msg = ToggleBack;
    CarStatus.ToggleSwtichPosition = ToggleBack;
    DEBUGINFO("motion_msg = ToggleBack\r\n");
  }
  //拨动开关拨向停止(中间挡位)
  else if(TOGGLE_FRONT_value == GPIO_PIN_SET && TOGGLE_BACK_value == GPIO_PIN_SET)
  {
    sensor_msg = ToggleStop;
    CarStatus.ToggleSwtichPosition = ToggleStop;
    DEBUGINFO("motion_msg = ToggleStop\r\n");
  }

  //发送拨动开关事件
  if(osMessageQueuePut(xSensor_QueueHandle, &sensor_msg, 0, pdMS_TO_TICKS(100)) != osOK)
  {
    DEBUGINFO("send motion error\r\n");
  }
}


/**
 * 重置LED状态检查函数
 *
 * 该函数检查RESET引脚的状态，并根据状态设置LED_RESET引脚的状态。
 *
 * @note 该函数会打印LED_RESET的状态信息到控制台。
 */
void vResetLedStatusCheck(void)
{
  if (GPIO_READ(RESET) == GPIO_PIN_RESET)
  {
    GPIO_WRITE(LED_RESET, GPIO_PIN_SET); // 使能LED_RESET
    DEBUGINFO("LED_RESET on\r\n");
    //DEBUGINFO("reset trigger\r\n");
    //osTimerStart(xResetButtonTimerHandle, pdMS_TO_TICKS(ResetDuration));
  }
  else
  {
    GPIO_WRITE(LED_RESET, GPIO_PIN_RESET); // 关闭LED_RESET
    DEBUGINFO("LED_RESET off\r\n");
  }
}

/******************************* GPIO防抖回调 *************************/
//传感器防抖回调
void vSensorDebounceCallback(void *argument)
{
  uint8_t msg;
  msg = SensorDebounce;
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
}

//拨动开关防抖回调
void vToggleSwitchCallback(void *argument)
{
  uint8_t msg;
  msg = ToggleDebounce;
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
}

//车厢电子锁防抖回调
void vBoxELockDebounceCallback(void *argument)
{
  uint8_t msg;
  msg = BoxELockDebounce;
  osMessageQueuePut(xInterrupt_QueueHandle, &msg, 0, 0);
}

//reset按钮防抖回调
void vResetButtonCallback(void *argument)
{
  u8 sensor_msg;
  if(GPIO_READ(RESET) == GPIO_PIN_RESET)
  {
    sensor_msg = Reset;
    if(osMessageQueuePut(xSensor_QueueHandle, &sensor_msg, 0, pdMS_TO_TICKS(100)) != osOK)
    {
      DEBUGINFO("send msg error\r\n");
    }
  }
}
