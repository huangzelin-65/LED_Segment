#ifndef ADAPTOR_INC_ADAPTOR_EEPROM_H_
#define ADAPTOR_INC_ADAPTOR_EEPROM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

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
extern EepromDevice eeprom_dev;

// 适配层初始化与通用操作接口
bool eeprom_adaptor_init(void* hi2c);
bool eeprom_check_conn(void);
bool eeprom_write_byte(uint16_t addr, uint8_t data);
bool eeprom_read_byte(uint16_t addr, uint8_t* data);
bool eeprom_write_buf(uint16_t addr, const uint8_t* data, uint16_t len);
bool eeprom_read_buf(uint16_t addr, uint8_t* data, uint16_t len);

//测试用函数
void vEepromTest(void);

#endif /* ADAPTOR_INC_ADAPTOR_EEPROM_H_ */
