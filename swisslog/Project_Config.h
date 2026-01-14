/*
 * Project_Config.h
 *
 *  Created on: 2025年12月13日
 *      Author: e3lijia25d
 */

#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

// #define UART1_TEST

#define BOX_CTRL
#define USE_WIFI

/********************************* CAN配置 ****************************************/
// #define USE_CAN
#ifdef USE_CAN
    #define USE_FDCAN1
    // #define USE_FDCAN2
    // #define CAN_TEST
#endif /* USE_CAN */


/********************************* mqtt配置 ****************************************/
#define MQTT_ENABLE       //开启此宏，使能mqtt功能  
#define MQTT_USE_WIFI     //开启此宏，mqtt通过WiFi模块tcp功能实现
// #define MQTT_USE_ETH      //开启此宏，mqtt通过有线网口实现
#define ROBOT            //开启此宏，响应与服务器交互事件
// #define REGISTER        //注册流程用

/********************************* RFID配置 ****************************************/
// #define ZHONGNENG_RFID 	//开启此宏，开启读取众能RFID模块
#ifdef  ZHONGNENG_RFID
    #define ZN_RFID_ENCRYPT     // 读有密码的卡
#endif /* ZHONGNENG_RFID */

#endif /* PROJECT_CONFIG_H_ */
