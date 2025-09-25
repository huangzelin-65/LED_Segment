#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adaptor_box.h"
#include "LogDebugInfo.h"


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
功能:去解锁
return 0:成功 1:失败
*/
uint8_t ELock_unLock(void)
{
  countTimes++;
  int count = 0; 

  //低电平解锁
  GPIO_WRITE(ELOCK_EN1, GPIO_PIN_RESET);
  GPIO_WRITE(ELOCK_EN2, GPIO_PIN_RESET);

  //锁上是低电平，解锁是高电平,两个都为高电平才算解锁成功，跳出循环
  while(!(GPIO_READ(ELOCK1_STATUS) && GPIO_READ(ELOCK2_STATUS)))
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

  if(GPIO_READ(ELOCK1_STATUS) && GPIO_READ(ELOCK2_STATUS))
  {
    DEBUGINFO("unlock success\r\n");
    return 0;
  }
  else
  {
    DEBUGINFO("unlock fail\r\n");
    return 1;
  }

}

/*************************** 紫外线灯 *****************************/
void vSterilamp_enable(void)
{
  GPIO_WRITE(Sterilamp_EN, GPIO_PIN_RESET);
}

void vSterilamp_disable(void)
{
  GPIO_WRITE(Sterilamp_EN, GPIO_PIN_SET);
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
      DEBUGINFO("BOX LED WHITE\r\n");
      break;

    case RED:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      DEBUGINFO("BOX LED RED\r\n");
      break;
    
    case GREEN:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      DEBUGINFO("BOX LED GREEN\r\n");
      break;

    case BLUE:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_SET);
      DEBUGINFO("BOX LED BLUE\r\n");
      break;

    case YELLOW:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_SET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      DEBUGINFO("BOX LED YELLOW\r\n");
      break;
    
    case LED_OFF:
      GPIO_WRITE(LED_BOX_R, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_G, GPIO_PIN_RESET);
      GPIO_WRITE(LED_BOX_B, GPIO_PIN_RESET);
      DEBUGINFO("BOX LED OFF\r\n");
      break;

    default:
      break;
  }

}