#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "LogDebugInfo.h"
#include "NumDisplay.h"
#include "adaptor_box.h"

void vBoxCtrlTask(void *argument)
{
  //初始化数码管显示
  NumDisp_Init();

  //设置数码管显示的数字
  NumDisp_SetNumber((uint16_t)666);

  osDelay(1000);
  
  NumDisp_BlueShan();

  ELock_unLock();

  vSterilamp_enable();
  osDelay(2000);
  vSterilamp_disable();

  

  while(1)
  {
    // vRGB_LED(RED);
    // osDelay(2000);
    // vRGB_LED(GREEN);
    // osDelay(2000);
    // vRGB_LED(BLUE);
    // osDelay(2000);
    // vRGB_LED(YELLOW);
    // osDelay(2000);
    // vRGB_LED(WHITE);
    // osDelay(2000);
    // vRGB_LED(LED_OFF);
    osDelay(2000);
  }
}