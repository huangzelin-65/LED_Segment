#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include "LogDebugInfo.h"
#include "adaptor_eeprom.h"
#include "eeprom_M24C64_i2c.h"
#include "semphr.h"

// 全局设备实例与硬件私有数据
EepromDevice eeprom_dev;
static M24C64_I2cPriv m24c64_i2c_priv;

// 测试数据定义
#define TEST_ADDR_BYTE    0x0000  // 单字节测试地址
#define TEST_ADDR_BUF     0x0100  // 缓冲区测试地址
#define TEST_BUF_LEN      64      // 测试缓冲区长度

void vEepromTest(void) {
  uint8_t test_byte = 0xAA;
  uint8_t test_buf[TEST_BUF_LEN];
  uint8_t read_byte;
  uint8_t read_buf[TEST_BUF_LEN+1];
  
  // 初始化测试缓冲区
  for (uint8_t i = 0; i < TEST_BUF_LEN; i++) {
      test_buf[i] = '0'+i; // 填充0x00, 0x01, ..., 0x3F
  }

  // 写入单字节并打印日志
  if (eeprom_write_byte(TEST_ADDR_BYTE, test_byte)) {
    DEBUGINFO("Write byte success: addr=0x%04X, data=0x%02X\r\n", 
            TEST_ADDR_BYTE, test_byte);
            
    if (eeprom_read_byte(TEST_ADDR_BYTE, &read_byte)) {
      DEBUGINFO("Read byte success: addr=0x%04X, data=0x%02X \r\n", 
            TEST_ADDR_BYTE, read_byte);
    } 
    else {
      DEBUGINFO("Read byte failed!\r\n");
    }
  } 
  else {
      DEBUGINFO("Write byte failed!\r\n");
  }

  // 写入缓冲区（跨页测试，64字节 > 32字节页大小）
  if (eeprom_write_buf(TEST_ADDR_BUF, test_buf, TEST_BUF_LEN)){
    DEBUGINFO("Write buffer success: addr=0x%04X, len=%d\r\n", 
            TEST_ADDR_BUF, TEST_BUF_LEN);

    // 读取缓冲区
    if (eeprom_read_buf(TEST_ADDR_BUF, read_buf, TEST_BUF_LEN)) {
			read_buf[TEST_BUF_LEN]='\0';
      DEBUGINFO("Read buffer success: addr=0x%04X, len=%d, data: %s \r\n", 
             TEST_ADDR_BUF, TEST_BUF_LEN, read_buf);
    } 
    else {
      DEBUGINFO("Read buffer failed!\r\n");
    }
  } 
  else {
    DEBUGINFO("Write buffer failed!\r\n");
  }
}

/**
 * @brief 初始化EEPROM适配层
 * 整合硬件初始化、线程安全配置和操作集绑定
 */
bool eeprom_adaptor_init(void* hi2c) {
    // 1. 初始化硬件私有数据
    if (!M24C64_i2c_init_priv(&m24c64_i2c_priv, (I2C_HandleTypeDef*)hi2c)) {
        return false;
    }

    // 2. 初始化设备实例
    memset(&eeprom_dev, 0, sizeof(EepromDevice));
    eeprom_dev.hw_priv = &m24c64_i2c_priv;
    eeprom_dev.ops = M24C64_i2c_get_ops();

    // 3. 创建线程安全锁
    eeprom_dev.mutex = xSemaphoreCreateMutex();
    if (eeprom_dev.mutex == NULL) {
        return false;
    }

    // 4. 初始化硬件并检查连接
    return (eeprom_dev.ops->init(&eeprom_dev) && eeprom_dev.ops->check_conn(&eeprom_dev));
}

// ------------------------------ 带线程安全的操作接口 ------------------------------

bool eeprom_check_conn(void) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->check_conn(&eeprom_dev);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool eeprom_write_byte(uint16_t addr, uint8_t data) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->write_byte(&eeprom_dev, addr, data);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool eeprom_read_byte(uint16_t addr, uint8_t* data) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->read_byte(&eeprom_dev, addr, data);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool eeprom_write_buf(uint16_t addr, const uint8_t* data, uint16_t len) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->write_buf(&eeprom_dev, addr, data, len);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool eeprom_read_buf(uint16_t addr, uint8_t* data, uint16_t len) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->read_buf(&eeprom_dev, addr, data, len);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}
