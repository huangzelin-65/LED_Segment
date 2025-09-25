#ifndef PERIPHERALS_INC_EEPROM_M24C64_I2C_H_
#define PERIPHERALS_INC_EEPROM_M24C64_I2C_H_


#include "adaptor_eeprom.h"


// M24C64硬件参数（数据手册定义）
#define M24C64_I2C_ADDR     0xA0    // I2C地址（A0-A2接地）
#define M24C64_TOTAL_SIZE   8192    // 总容量：8KB
#define M24C64_PAGE_SIZE    32      // 页大小：32字节
#define M24C64_WRITE_DELAY  5       // 写入延迟：5ms

/**
 * @brief M24C64硬件私有数据
 * 存储I2C句柄等硬件资源
 */
typedef struct {
    I2C_HandleTypeDef* hi2c;  // STM32 I2C句柄
} M24C64_I2cPriv;

// M24C64适配层对外接口
const M24C64_I2cOps* M24C64_i2c_get_ops(void);
bool M24C64_i2c_init_priv(M24C64_I2cPriv* priv, I2C_HandleTypeDef* hi2c);



#endif /* PERIPHERALS_INC_EEPROM_M24C64_I2C_H_ */
