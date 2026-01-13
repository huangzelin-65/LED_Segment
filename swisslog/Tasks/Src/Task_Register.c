#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdio.h>
#include "Task_Register.h"
#include "LogDebugInfo.h"
#include "adaptor_mqtt.h"
#include "adaptor_wifi.h"
#include "adaptor_register.h"
#include "Common.h"
#include <stdbool.h>
#include "queue.h"
#include "Robot.h"
#include "Encoder.h"
#include "semphr.h"
#include <limits.h>
#include "Register.h"
#include "StringEdit.h"
#include "RegisterInfo.h"
#include "ChipInfo.h"
#include "Calculate.h"

//处理注册消息，获取工程注册信息或已经获取到的注册信息,存入mqtt登录信息中
void vRegisterManagerTask(void *argument)
{
    uint32_t ulNotificationValue;
    BaseType_t xResult;      
    DEBUGINFO("vRegisterManagerTask\r\n");
    //测试用
    // bEraseRegisterArea();
    // memset(&g_register_information,0x0,sizeof(Stru_Field_Register_Typedef));
    // bReadFieldRegisterInfo(&g_register_information);    
    while(1)
    {
      // 等待通知，超时时间为永远等待
      // 清除方式: 收到通知后清除通知值
      xResult = xTaskNotifyWait(0x00,    // 进入函数前不清除任何位
                                ULONG_MAX,// 退出函数时清除所有位
                                &ulNotificationValue, 
                                portMAX_DELAY);
      
      if (xResult == pdPASS) 
      {
        DEBUGINFO("ulNotificationValue:%lx\n",ulNotificationValue);
        if(ulNotificationValue & REGISTER_NOTIFY_GET_INFO)
        {
            DEBUGINFO("REGISTER_NOTIFY_GET_INFO\n");  
            memset(&mqtt_info,0,sizeof(MqttInfo_t));
            memset(&g_register_information,0,sizeof(Stru_Field_Register_Typedef));
            //初始化uuid
            uint32_t uid[3];
            Chip_GetUId(uid);                    
            uid_to_uuid(uid,mqtt_info.uuid,MQTT_UUID_ID_LENGTH);
            DEBUGINFO("uuid:%s\n",mqtt_info.uuid);

            bool rc = bReadFieldRegisterInfo(&g_register_information);
            if(rc == true)//正常登录
            {
                //这里已经获取正常账号和密码
                memcpy(mqtt_info.name,g_register_information.name,MQTT_NAME_LENGTH);
                memcpy(mqtt_info.pwd,g_register_information.pwd,MQTT_PSW_LENGTH);
                memcpy(mqtt_info.sn,g_register_information.sn,MQTT_SN_LENGTH);
                g_register_state.to_register = 0;//不需要静默注册
                DEBUGINFO("name:%s pwd:%s sn:%s\n", g_register_information.name,g_register_information.pwd,g_register_information.sn);
                //从sn中提取ID信息
                if (extract_last_numbers((char *)g_register_information.sn, mqtt_info.id, sizeof(mqtt_info.id))) {                            
                    DEBUGINFO("id:%s\n", mqtt_info.id);

                    //测试用
                    snprintf(mqtt_info.id, 7, "%d", usEncoder_Read_Number());  //"%06d"

                    Mqtt_Notify(MQTT_NOTIFY_INIT);
                }                        
            }
            else
            {
                //以下是测试用，模拟工程注册，后需要删除
                {
                    bool rc = bInitProdRegisterInfo(mqtt_info.uuid, MQTT_UUID_ID_LENGTH);
                    if(rc == true)
                    {
                        DEBUGINFO("bInitProdRegisterInfo success\n"); 
                    }
                }
                
                //校验产品
                bool rc = bReadProdRegisterInfo(mqtt_info.uuid, MQTT_UUID_ID_LENGTH);
                if(rc == true)//校验成功，产品需静默注册
                {                            
                    strcpy(mqtt_info.name, MQTT_REGISTER_NAME);//使用默认名称
                    strcpy(mqtt_info.pwd, MQTT_REGISTER_PSW); //使用默认密码
                    strcpy(mqtt_info.sn, mqtt_info.uuid);//为了满足多台机器同时静默升级且不冲突，此处用uuid作为client id 登录
                    g_register_state.to_register = 1;  //需要执行静默注册


                    //测试用
                    // strcpy(mqtt_info.name, MQTT_TEST_NAME);
                    // strcpy(mqtt_info.pwd, MQTT_TEST_PSW);
                    // snprintf(mqtt_info.id, 7, "%d", usEncoder_Read_Number());  

                    Mqtt_Notify(MQTT_NOTIFY_INIT);                       
                }
                else
                {
                    //产品无法正常使用
                }
            }            
        }        

      }
    }
}
//从服务器中获取注册信息（主要获取流水号）
void vRegisterHandleTask(void *argument)
{
    int register_info_cnt = 0;     
    DEBUGINFO("vRegisterHandleTask\r\n");
    while(1)
    {
        if(Mqtt_IsConnected())
        {
            if(waitforperiod(&register_info_cnt,5))
            {
                if(g_register_state.to_register)
                {
                    if(g_register_state.register_cnt++ > 5)
                    {
                        DEBUGINFO("register fail\n");
                        g_register_state.to_register = 0;
                        bIncFieldRegisterErrTimes();//记录注册失败次数
                    }
                    else
                    {
                        Register_Event();
                    }
                }
            }
        }
        osDelay(1000);
    }
}










