/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Robot.h"
#include "Project_Config.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
/*!< STM32F10x Standard Peripheral Library old types (maintained for legacy purpose) */
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t sc8;   /*!< Read Only */

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  /*!< Read Only */
typedef __I int16_t vsc16;  /*!< Read Only */
typedef __I int8_t vsc8;   /*!< Read Only */

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  /*!< Read Only */
typedef __I uint16_t vuc16;  /*!< Read Only */
typedef __I uint8_t vuc8;   /*!< Read Only */


//----------中断类型枚举----------
typedef enum {
	FrontCrashSensor,       //前碰撞传感器
	RearCrashSensor,        //后碰撞传感器
	FrontProxSensor,        //前距离传感器
	RearProxSensor,         //后距离传感器
    ToggleSwitch,           //拨动开关
    BoxELock,               //车厢电子锁
    ResetButton,            //复位按钮
    LowVoltageDetect,       //低电压检测
    ServiceJoystick,            //维修模式
    SensorDebounce,         //传感器防抖
    ToggleDebounce,         //拨动开关防抖
    BoxELockDebounce,       //车厢电子锁防抖
    ServiceJoystickDebounce,    //维修模式防抖
}eInterruptType;

//----------小车在站状态枚举----------
typedef enum {
  	InStation = 0,
   	OutStation = 1,
}CarStationStatus;

//----------小车动作类型枚举----------
typedef enum {
  SensorEvent,          //小车传感器变化
  SensorTrigger,        //小车传感器触发
  SensorRelease,        //小车传感器释放
  SensorError,          //小车传感器异常
  ToggleFront,          //拨动开关拨向前
  ToggleBack,           //拨动开关拨向后
  ToggleStop,           //拨动开关拨向停止
  ServiceFront,         //维修模式拨向前
  ServiceBack,          //维修模式拨向后
  ServiceStop,          //维修模式拨向停止
  ResetButtonTrigger,   //小车复位按钮触发
  Reset,                //小车复位
}eSensorMotiontType; 


//----------小车速度枚举----------
typedef enum
{
    ZeroSpeed = 0,
    LowSpeed = 1,
    NormalSpeed = 2,
    HighSpeed = 3,
}eSpeedType;


//----------小车方向枚举----------
typedef enum
{
    NoDirection = 0,
    Forward = 1,
    Backward = 2,
}eDirectionType;


//----------小车自动模式枚举----------
typedef enum
{
    Manual = 1,
    Auto = 2,
}eAutoModeType;

//----------小车运动状态枚举----------
typedef enum
{
    CarStop = 0,
    CarReadyToRun = 1,
    CarRunning = 2,
}eCarRunningStatusType;

//----------电机使能状态枚举----------
typedef enum
{
    MotorDisable = 0,
    MotorEnable = 1,
}eMotorStatusType;

//----------电机停止原因枚举----------
typedef enum
{
    NoStopReason,       // 无停止原因
    ByCommand,          // 通过命令停止
    BySensorError,      // 传感器异常停止
    BySensorTrigger,    // 传感器触发停止
    ByToggleStop,       // 拨动开关停止
    ByServiceMode,      // 维修模式停止
    ByStopTag,          // 停止标签停止
    ByBoxUnlock,        // 车厢未上锁停止
    ByInstation,        // 进站停止
    ByReset,            // 复位导致停止
    ByMotorError,       // 电机异常停止
    ByInit,             // 初始化时停止
}eMotorStopReasonType;

//----------LED灯颜色枚举----------
typedef enum
{
    WHITE = 0,
    RED = 1,
    GREEN = 2,
    BLUE = 3,
    YELLOW = 4,
    LED_OFF = 5,
}eLedColorType;

//----------锁状态枚举----------
typedef enum
{
    UnLock = 0,
    Locked = 1,
}eLockStatusType;

//----------车厢控制枚举----------
typedef enum
{
    BoxElockOps,           // 车厢电子锁操作(解锁)
    RfidLoginTimeout,      // 车厢RFID登录超时
    UpdateStationStatus,   // 更新车厢在站状态
    UpdateBoxLockStatus,   // 更新车厢电子锁状态
    UpdateUVCleanStatus,   // 更新车厢紫外线清洁状态
}eBoxCtrlType;

//----------小车状态结构体------------------------------
typedef struct
{
    eDirectionType xSetDirection;			    // 小车设置方向记录 0:Stop/1.Forward/2:Backward    
    eDirectionType xRealDirection;		        // 小车实际方向记录 0:Stop/1.Forward/2:Backward  
    eSpeedType xSetSpeed;					    // 小车设置速度记录 0:停止/1:低速/2:额定速度/3:高速  
    eSpeedType xRealSpeed;				        // 小车实际速度记录 0:停止/1:低速/2:额定速度/3:高速
    eAutoModeType xAutoMode;                    // 自动模式标志 1:Manual/2:Auto
    eCarRunningStatusType xIsCarRunning;        // 小车运行状态 0:CarStop/1:CarReadyToRun/2:CarRunning
    eMotorStatusType xMotorEnable;              // 电机使能标志 0:MotorDisable/1:MotorEnable
    uint16_t usCarID;	  					    // 小车车号
    uint16_t usDestination;				        // 小车目的地
    CarStationStatus xStationStatus;            // 小车当前在站状态 0：InStation 1：OutStation
    eMotorStopReasonType xMotorStopReason;      // 电机停止原因

    eSensorMotiontType ToggleSwtichPosition;	// 拨码开关位置 ToggleStop:停车/ToggleFront:前进/ToggleBack:后退
    eSensorMotiontType ServiceJoystickPosition; // 维修控杆位置 ServiceStop:停车/ServiceFront:前进/ServiceBack:后退
    eSensorMotiontType FrontProxStatus;			// 前距离传感器状态 SensorRelease:未检测/SensorTrigger:检测到
    eSensorMotiontType FrontCrashStatus;		// 前碰撞开关状态 SensorRelease:未检测/SensorTrigger:检测到
    eSensorMotiontType RearProxStatus;			// 后距离传感器状态 SensorRelease:未检测/SensorTrigger:检测到
    eSensorMotiontType RearCrashStatus;   		// 后碰撞开关状态 SensorRelease:未检测/SensorTrigger:检测到
    eLockStatusType xBoxELockStatus1;           // 车厢电子锁1状态 UnLock:未锁上/Locked:已锁上
    eLockStatusType xBoxELockStatus2;           // 车厢电子锁2状态 UnLock:未锁上/Locked:已锁上
    eLockStatusType xBoxLocked;		            // 车厢是否锁上 UnLock:未锁上/Locked:已锁上
    eLockStatusType xScreenLockStatus;          // 车厢屏幕是否锁上 UnLock:未锁上/Locked:已锁上
    uint32_t dwCurPos;                          // 小车当前位置地址编号0-99999
    uint32_t dwPrevPos;                         // 小车上一位置地址编号
    uint8_t u8_uvTimeRemain;                     // 剩余消毒时间
    uint8_t MotorStatus;   				        // 马达运行状态 0:正常/1:异常
    uint8_t MotorOverloadNum;                   // 电机过载次数
    uint16_t SumAverage;				        // 电流方均值
    uint16_t MaxSpeed;					        // 最大速度
    uint16_t MaxCurrent;				        // 最大电流
} CarStatus_t;



#pragma pack(push, 1)  // 1字节对齐

typedef struct {
    uint16_t wTypeId;
    uint16_t wLength;
} CIPHeader;

typedef struct {
    uint32_t dwConnId;
    uint16_t wSeq;
    uint8_t  bSpare[2];
} CipSeqAddrData;

typedef struct {
    uint16_t wSeq;         // 序号
    uint32_t dwPlcNum;     // PLC编号（抓包分析该字段都是0x00000001猜测是PLC的编号）
    uint16_t wHeatBeat;    // 心跳信号
    uint16_t wAlm;         // 报警信号
    uint8_t wCtrl;        // 控制信号
    uint16_t wYear;        // 年
    uint8_t  bMonth;       // 月
    uint8_t  bDay;         // 日
    uint8_t  bHour;        // 时
    uint8_t  bMinite;      // 分
    uint8_t  bSecond;      // 秒
    uint8_t  bSterStHSV;   //消毒开始时间设定（H）
    uint8_t  bSterStMSV;   //消毒开始时间设定（M）
    uint8_t  bSterCycleSV; //消毒周期设定（H）
    uint32_t dwDestPos;    // 小车任务目标位置地址编号0-99999
    uint32_t dwDestPra;    // 小车任务密码：6位数字密码
    uint16_t wSterTimSV;   //小车消毒时长（min）
    uint8_t  bInitSTID;    // 起点站点编号
    uint8_t  bDestSTID;    // 目的地站点号
    uint8_t  bSpare28;     // 备用
    uint8_t  bType;        // 小车类型：1=洁车 2=污车
    eAutoModeType  xAutoMode;        // 自动模式标志 1:Manual/2:Auto
    eDirectionType  xDirection;      // 小车运行方向 1=正转 2=反转
    eSpeedType xSetSpeed;            // 小车速度设置 0:停止/1:低速/2:额定速度/3:高速  
    eMotorStatusType xMotorEnable;   // 电机使能标志 0:MotorDisable/1:MotorEnable
    CarStationStatus xStationStatus; // 小车当前在站状态 0：InStation 1：OutStation
    eLockStatusType xScreenLockStatus; // 车厢屏幕是否锁上 UnLock:未锁上/Locked:已锁上
} ServerToCarData_t;

typedef struct {
    uint16_t wSeq;          // 序号
    uint16_t wHeatBeat;     // 通讯心跳
    uint16_t wAlm;          // 报警信息
    uint16_t wSta;          // 状态信息
    uint8_t bID;            // 小车编号
    uint8_t ucSpdMode;       // 小车速度模式：1=·速度1，2=速度2，3=速度3
    uint8_t bRTimM;         // 小车运行总时间（分钟）
    uint16_t wRTimH;        // 小车运行总时间（小时）
    uint32_t dwCurPos;      // 小车当前位置地址编号0-99999
    uint32_t dwPrevPos;     // 小车上一位置地址编号
    uint16_t wAmps;         // 小车消电流(A)
    uint16_t wSterRTim;     // 小车消毒运行时间(S)
    uint8_t bSpare24;       // 备用
    uint8_t bSpare25;       // 备用
    uint16_t wTemperature;  // 小车温度（℃）
    uint16_t wErrCode;      // 小车驱动器错误代码
    uint8_t ucPosType;      // 小车位置类型
    uint8_t xDirection;    // 小车运行方向1=正转 2=反转，手动运行时不修改此变量
    uint32_t dwVersion;     // 小车固件版本号
    uint8_t  bSpare[4];     // 备用36~39
} CarToServerData_t;

#pragma pack(pop)

extern ServerToCarData_t ServerToCarData;
extern CarToServerData_t CarToServerData;
extern CarStatus_t CarStatus;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define HMI_TX_Pin GPIO_PIN_3
#define HMI_TX_GPIO_Port GPIOE
#define HMI_RX_Pin GPIO_PIN_2
#define HMI_RX_GPIO_Port GPIOE
#define Rfid_Box_TX_Pin GPIO_PIN_1
#define Rfid_Box_TX_GPIO_Port GPIOE
#define Rfid_Box_RX_Pin GPIO_PIN_0
#define Rfid_Box_RX_GPIO_Port GPIOE
#define EEPROM_SCL_Pin GPIO_PIN_8
#define EEPROM_SCL_GPIO_Port GPIOB
#define FDCAN2_RX_Pin GPIO_PIN_5
#define FDCAN2_RX_GPIO_Port GPIOB
#define LOW_VOLTAGE_DET_Pin GPIO_PIN_14
#define LOW_VOLTAGE_DET_GPIO_Port GPIOG
#define LOW_VOLTAGE_DET_EXTI_IRQn EXTI14_IRQn
#define Encoder_S2_4_Pin GPIO_PIN_4
#define Encoder_S2_4_GPIO_Port GPIOB
#define Encoder_S2_8_Pin GPIO_PIN_3
#define Encoder_S2_8_GPIO_Port GPIOB
#define Encoder_S3_4_Pin GPIO_PIN_15
#define Encoder_S3_4_GPIO_Port GPIOA
#define BOX_5V_Pin GPIO_PIN_4
#define BOX_5V_GPIO_Port GPIOE
#define ELOCK_EN1_Pin GPIO_PIN_5
#define ELOCK_EN1_GPIO_Port GPIOE
#define ELOCK2_STATUS_Pin GPIO_PIN_6
#define ELOCK2_STATUS_GPIO_Port GPIOE
#define ELOCK2_STATUS_EXTI_IRQn EXTI6_IRQn
#define EEPROM_SDA_Pin GPIO_PIN_9
#define EEPROM_SDA_GPIO_Port GPIOB
#define BEEP_PWM_Pin GPIO_PIN_7
#define BEEP_PWM_GPIO_Port GPIOB
#define FDCAN2_TX_Pin GPIO_PIN_6
#define FDCAN2_TX_GPIO_Port GPIOB
#define NUMDISP_485_CTRL_Pin GPIO_PIN_13
#define NUMDISP_485_CTRL_GPIO_Port GPIOC
#define UV_CLEAN_EN_Pin GPIO_PIN_8
#define UV_CLEAN_EN_GPIO_Port GPIOI
#define FDCAN1_RX_Pin GPIO_PIN_9
#define FDCAN1_RX_GPIO_Port GPIOI
#define Encoder_S3_2_Pin GPIO_PIN_15
#define Encoder_S3_2_GPIO_Port GPIOH
#define USART1_RX_Pin GPIO_PIN_10
#define USART1_RX_GPIO_Port GPIOA
#define ELOCK_EN2_Pin GPIO_PIN_11
#define ELOCK_EN2_GPIO_Port GPIOI
#define FDCAN1_TX_Pin GPIO_PIN_13
#define FDCAN1_TX_GPIO_Port GPIOH
#define USART1_TX_Pin GPIO_PIN_9
#define USART1_TX_GPIO_Port GPIOA
#define Wifi_RX_Pin GPIO_PIN_7
#define Wifi_RX_GPIO_Port GPIOC
#define LED_RESET_Pin GPIO_PIN_2
#define LED_RESET_GPIO_Port GPIOF
#define TOGGLE_FRONT_Pin GPIO_PIN_4
#define TOGGLE_FRONT_GPIO_Port GPIOH
#define TOGGLE_FRONT_EXTI_IRQn EXTI4_IRQn
#define Wifi_TX_Pin GPIO_PIN_6
#define Wifi_TX_GPIO_Port GPIOC
#define NUMDISP_TX_Pin GPIO_PIN_3
#define NUMDISP_TX_GPIO_Port GPIOF
#define NUMDISP_RX_Pin GPIO_PIN_4
#define NUMDISP_RX_GPIO_Port GPIOF
#define LED_RED_Pin GPIO_PIN_7
#define LED_RED_GPIO_Port GPIOG
#define SERVICE_FRONT_Pin GPIO_PIN_5
#define SERVICE_FRONT_GPIO_Port GPIOF
#define SERVICE_FRONT_EXTI_IRQn EXTI5_IRQn
#define FC_L_Pin GPIO_PIN_12
#define FC_L_GPIO_Port GPIOH
#define FC_L_EXTI_IRQn EXTI12_IRQn
#define FC_H_Pin GPIO_PIN_5
#define FC_H_GPIO_Port GPIOG
#define LED_YELLOW_Pin GPIO_PIN_4
#define LED_YELLOW_GPIO_Port GPIOG
#define Encoder_S2_2_Pin GPIO_PIN_3
#define Encoder_S2_2_GPIO_Port GPIOG
#define FP_H_Pin GPIO_PIN_11
#define FP_H_GPIO_Port GPIOH
#define FP_H_EXTI_IRQn EXTI11_IRQn
#define RC_H_Pin GPIO_PIN_10
#define RC_H_GPIO_Port GPIOH
#define RC_H_EXTI_IRQn EXTI10_IRQn
#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOD
#define Encoder_S2_1_Pin GPIO_PIN_2
#define Encoder_S2_1_GPIO_Port GPIOG
#define ELOCK1_STATUS_Pin GPIO_PIN_2
#define ELOCK1_STATUS_GPIO_Port GPIOC
#define ELOCK1_STATUS_EXTI_IRQn EXTI2_IRQn
#define MOTOR_485_CTRL_Pin GPIO_PIN_3
#define MOTOR_485_CTRL_GPIO_Port GPIOC
#define Encoder_S3_8_Pin GPIO_PIN_2
#define Encoder_S3_8_GPIO_Port GPIOB
#define TOGGLE_BACK_Pin GPIO_PIN_1
#define TOGGLE_BACK_GPIO_Port GPIOG
#define TOGGLE_BACK_EXTI_IRQn EXTI1_IRQn
#define RC_L_Pin GPIO_PIN_9
#define RC_L_GPIO_Port GPIOH
#define RC_L_EXTI_IRQn EXTI9_IRQn
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOD
#define SERVICE_BACK_Pin GPIO_PIN_0
#define SERVICE_BACK_GPIO_Port GPIOA
#define SERVICE_BACK_EXTI_IRQn EXTI0_IRQn
#define RP_H_Pin GPIO_PIN_13
#define RP_H_GPIO_Port GPIOF
#define RP_H_EXTI_IRQn EXTI13_IRQn
#define FP_L_Pin GPIO_PIN_0
#define FP_L_GPIO_Port GPIOG
#define Rfid_Car_Power_Pin GPIO_PIN_13
#define Rfid_Car_Power_GPIO_Port GPIOE
#define Encoder_S1_1_Pin GPIO_PIN_11
#define Encoder_S1_1_GPIO_Port GPIOD
#define LED_GREEN_Pin GPIO_PIN_10
#define LED_GREEN_GPIO_Port GPIOD
#define LED_BOX_B_Pin GPIO_PIN_12
#define LED_BOX_B_GPIO_Port GPIOF
#define RESET_Pin GPIO_PIN_15
#define RESET_GPIO_Port GPIOF
#define RESET_EXTI_IRQn EXTI15_IRQn
#define Motor_TX_Pin GPIO_PIN_8
#define Motor_TX_GPIO_Port GPIOE
#define Wifi_Power_Pin GPIO_PIN_9
#define Wifi_Power_GPIO_Port GPIOE
#define Rfid_Car_RX_Pin GPIO_PIN_12
#define Rfid_Car_RX_GPIO_Port GPIOB
#define Rfid_Car_TX_Pin GPIO_PIN_13
#define Rfid_Car_TX_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_9
#define LED4_GPIO_Port GPIOD
#define LED3_Pin GPIO_PIN_8
#define LED3_GPIO_Port GPIOD
#define RP_L_Pin GPIO_PIN_3
#define RP_L_GPIO_Port GPIOA
#define RP_L_EXTI_IRQn EXTI3_IRQn
#define LED_BOX_R_Pin GPIO_PIN_1
#define LED_BOX_R_GPIO_Port GPIOB
#define LED_BOX_G_Pin GPIO_PIN_0
#define LED_BOX_G_GPIO_Port GPIOB
#define Encoder_S1_2_Pin GPIO_PIN_14
#define Encoder_S1_2_GPIO_Port GPIOF
#define Motor_RX_Pin GPIO_PIN_7
#define Motor_RX_GPIO_Port GPIOE
#define LED5_Pin GPIO_PIN_10
#define LED5_GPIO_Port GPIOE
#define Encoder_S3_1_Pin GPIO_PIN_14
#define Encoder_S3_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define MotorOverloadMaxNum 2      //电机最大过载次数(每一分钟减一次) ljj
#define DebounceTime 20      //GPIO中断消抖时间(ms)
#define ResetDuration 1000    //reset按钮持续按下时间(ms)


//读gpio电平的宏
#define GPIO_READ(pin) \
    HAL_GPIO_ReadPin(pin##_GPIO_Port, pin##_Pin)

//写gpio电平的宏
#define GPIO_WRITE(pin, state) \
    HAL_GPIO_WritePin(pin##_GPIO_Port, pin##_Pin, state)

#define CIP_HEADER_TOTAL_SIZE (sizeof(uint16_t) + sizeof(CIPHeader) + sizeof(CipSeqAddrData) + sizeof(CIPHeader))
#define CIP_CAR_TO_PLC_DATA_SIZE (CIP_HEADER_TOTAL_SIZE + sizeof(CarToServerData_t))
#define CIP_PLC_TO_CAR_DATA_SIZE (CIP_HEADER_TOTAL_SIZE + sizeof(ServerToCarData_t))
#define CAR_TO_PLC_DATA_SIZE sizeof(CarToServerData_t) //先不加协议头
#define PLC_TO_CAR_DATA_SIZE sizeof(ServerToCarData_t) //先不加协议头
#define TEMP_SIZE 12



/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
