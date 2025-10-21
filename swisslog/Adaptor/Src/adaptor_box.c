#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adaptor_box.h"
#include "LogDebugInfo.h"

extern CarStatus_t CarStatus;
extern osMessageQueueId_t xBox_Ctrl_QueueHandle;

/*************************** NumDisp *****************************/
extern UART_HandleTypeDef huart11;

void NumDisp_Init(void)
{
  GPIO_WRITE(NUMDISP_485_CTRL, GPIO_PIN_SET); // 使能485的TX发送：NUMDISP_485_CTRL
}

void vSendToNumDisp(uint8_t * CmdDataArr,uint8_t len)
{
  taskENTER_CRITICAL(); // 进入临界区
  HAL_UART_Transmit(&huart11, CmdDataArr, len,HAL_MAX_DELAY);  // 启动发送
  taskEXIT_CRITICAL(); // 退出临界区
}

/*************************** Elock *****************************/

int countTimes = 0;

/**
锁操作：0：解锁，需要手动锁上
锁状态GPIO读取：0：锁上状态，1：解锁状态
锁状态判定：0：没上锁，1：上锁
return 0:成功 1:失败
*/
uint8_t ELock_unLock(void)
{
  countTimes++;
  int count = 0; 

  //低电平解锁
  GPIO_WRITE(ELOCK_EN1, GPIO_PIN_RESET);
  GPIO_WRITE(ELOCK_EN2, GPIO_PIN_RESET);

  //解锁是0, 两个状态都为0才算解锁成功，跳出循环
  while(ELOCK1_LEVEL || ELOCK2_LEVEL)
  {
    count++;
    if(count>5){
      break;
    }
    osDelay(10);
  }
  GPIO_WRITE(ELOCK_EN1, GPIO_PIN_SET);
  GPIO_WRITE(ELOCK_EN2, GPIO_PIN_SET);

  osDelay(10);

  if(ELOCK1_LEVEL || ELOCK2_LEVEL)
  {
    DEBUGINFO("unlock fail\r\n");
    return 1;
  }
  else
  {
    DEBUGINFO("unlock success\r\n");
    return 0;
  }
}

//检测车厢电子锁状态
void vBoxELockStatusCheck(void)
{

  volatile uint8_t BOX_ELOCK1_value;
  volatile uint8_t BOX_ELOCK2_value;
  eBoxCtrlType box_msg;

  //锁上是低电平，解锁是高电平
  // 读BOX_ELOCK1电平
  BOX_ELOCK1_value = GPIO_READ(ELOCK1_STATUS);
  // 读BOX_ELOCK2电平
  BOX_ELOCK2_value = GPIO_READ(ELOCK2_STATUS);

  if(BOX_ELOCK1_value == GPIO_PIN_RESET)
  {
    CarStatus.xBoxELockStatus1 = Locked;
    DEBUGINFO("xBoxELockStatus1 = Locked\r\n");
  } else {
    CarStatus.xBoxELockStatus1 = UnLock;
    DEBUGINFO("xBoxELockStatus1 = UnLock\r\n");
  }

  if(BOX_ELOCK2_value == GPIO_PIN_RESET)
  {
    CarStatus.xBoxELockStatus2 = Locked;
    DEBUGINFO("xBoxELockStatus2 = Locked\r\n");
  } else {
    CarStatus.xBoxELockStatus2 = UnLock;
    DEBUGINFO("xBoxELockStatus2 = UnLock\r\n");
  }

  if((CarStatus.xBoxELockStatus1 == Locked) || (CarStatus.xBoxELockStatus2 == Locked))
  {
    // 锁上
    CarStatus.xBoxLocked = Locked;
    DEBUGINFO("xBoxLocked = Locked\r\n");
  } else {
    // 解锁
    CarStatus.xBoxLocked = UnLock;
    DEBUGINFO("xBoxLocked = UnLock\r\n");
  }

  box_msg = UpdateBoxLockStatus;

    //发送电子锁事件
  if(osMessageQueuePut(xBox_Ctrl_QueueHandle, &box_msg, 0, pdMS_TO_TICKS(100)) != osOK)
  {
    DEBUGINFO("send box_msg error\r\n");
  }
}

/*************************** 紫外线灯 *****************************/
void vUV_Clean_enable(void)
{
  GPIO_WRITE(UV_CLEAN_EN, GPIO_PIN_RESET);
}

void vUV_Clean_disable(void)
{
  GPIO_WRITE(UV_CLEAN_EN, GPIO_PIN_SET);
}

/*************************** RGB灯 *****************************/
void vRGB_LED(uint8_t ucColor)
{
  //默认低电平，高电平有效
  switch (ucColor)
  {
    case WHITE:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_SET);
      //DEBUGINFO("BOX LED WHITE\r\n");
      break;

    case RED:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      //DEBUGINFO("BOX LED RED\r\n");
      break;
    
    case GREEN:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      //DEBUGINFO("BOX LED GREEN\r\n");
      break;

    case BLUE:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_SET);
      //DEBUGINFO("BOX LED BLUE\r\n");
      break;

    case YELLOW:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      //DEBUGINFO("BOX LED YELLOW\r\n");
      break;
    
    case LED_OFF:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      //DEBUGINFO("BOX LED OFF\r\n");
      break;

    default:
      break;
  }

}

/*************************** 车厢按键灯 *****************************/
// void Button_LED_En(uint8_t Val)
// {
// 	if(Val==0)
// 		GPIO_WRITE(LED_BUTTON, GPIO_PIN_RESET);
// 	else
// 		GPIO_WRITE(LED_BUTTON, GPIO_PIN_SET);
// }