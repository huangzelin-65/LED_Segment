#ifndef ADAPTOR_INC_ADAPTOR_EEPROM_H_
#define ADAPTOR_INC_ADAPTOR_EEPROM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define EEP_ADD_SCREEN_LOCK_STATUS    		10  // 1bytes 屏幕锁定状态
// #define EEP_ADD_CAR_STATION_STATUS    		10  // 1bytes
#define EEP_ADD_CAR_NUMBER		    		12  // 2bytes 小车序号
#define EEP_ADD_EEPROM_NEED_INIT		    16  // 2bytes EEPROM是否需要初始化标志（初始化后写入{0x51,0x4d}）
#define EEP_ADD_IDCARD_PASSWORD_NUM		  	20 //1 bytes 车厢卡密码位数
#define EEP_ADD_IDCARD_PASSWORD	      	  	21 //60 bytes 车厢卡密码（暂时没使用）
#define EEP_ADD_UVCLEAN_TIME_MINUTES	    100 //1bytes 默认消毒时长设置 
#define EEP_ADD_LAST_UVCLEAN_DATE	      	101 //rtc time+clean time bytes 上次消毒rtc时间 + 消毒时长
#define EEP_ADD_SEND_PASSWORD		      	120 //6 bytes 屏幕上设置的6位数密码
#define EEP_ADD_IS_ENCRYED			      	130 //1 bytes 车厢加密状态（1加密/0未加密）(为0时设置小车为在站状态)
#define EEP_ADD_EN_VIRTUAL_BUTTON			131 //1 bytes 首页是否显示虚拟解锁按钮
#define EEP_ADD_EN_IN_STATION_SENSOR		132 //1 bytes 虚拟进站信号（弃用）
#define EEP_ADD_LAST_CORRECT_DATE	      	150 //CCORRECT time 6 bytes 上一次TP校准时间

// 前向声明
typedef struct M24C64_I2cOps M24C64_I2cOps;

/**
 * @brief EEPROM设备核心结构体
 * 存储设备信息、操作接口、线程安全锁和硬件私有数据
 */
typedef struct {
    uint32_t        total_size;      // 总容量（字节）
    uint16_t        page_size;       // 页大小（字节）
    uint8_t         write_delay_ms;  // 写入等待时间（ms）
    const M24C64_I2cOps* ops;        // 操作函数集
    void*           mutex;           // 线程安全锁（FreeRTOS互斥锁）
    void*           hw_priv;         // 硬件私有数据
} EepromDevice;

/**
 * @brief M24C64 I2C适配层操作函数集
 * 定义所有硬件操作接口，由适配层实现
 */
struct M24C64_I2cOps {
    bool (*init)(EepromDevice* dev);
    bool (*check_conn)(EepromDevice* dev);
    bool (*write_byte)(EepromDevice* dev, uint16_t addr, uint8_t data);
    bool (*read_byte)(EepromDevice* dev, uint16_t addr, uint8_t* data);
    bool (*write_buf)(EepromDevice* dev, uint16_t addr, const uint8_t* data, uint16_t len);
    bool (*read_buf)(EepromDevice* dev, uint16_t addr, uint8_t* data, uint16_t len);
};

// 全局EEPROM设备实例
extern EepromDevice x_eepromDev;

// 适配层初始化与通用操作接口
void v_Eeprom_Data_Init(void);
bool b_Eeprom_Adaptor_Init(void* hi2c);

bool b_Eeprom_Check_Conn(void);

bool b_Eeprom_Write_Byte(uint16_t addr, uint8_t data);
bool b_Eeprom_Read_Byte(uint16_t addr, uint8_t* data);

bool b_Eeprom_Write_Buf(uint16_t addr, const uint8_t* data, uint16_t len);
bool b_Eeprom_Read_Buf(uint16_t addr, uint8_t* data, uint16_t len);

//测试用函数
void vEepromTest(void);

#endif /* ADAPTOR_INC_ADAPTOR_EEPROM_H_ */
