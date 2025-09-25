#include "main.h"
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "task.h"
#include "stm32h5xx_hal_i2c.h"
#include <string.h>
#include <stdlib.h>
#include "LogDebugInfo.h"
#include "eeprom_M24C64_i2c.h"


// ------------------------------ 内部函数声明 ------------------------------
static bool M24C64_i2c_init(EepromDevice* dev);
static bool M24C64_i2c_check_conn(EepromDevice* dev);
static bool M24C64_i2c_write_byte(EepromDevice* dev, uint16_t addr, uint8_t data);
static bool M24C64_i2c_read_byte(EepromDevice* dev, uint16_t addr, uint8_t* data);
static bool M24C64_i2c_write_buf(EepromDevice* dev, uint16_t addr, const uint8_t* data, uint16_t len);
static bool M24C64_i2c_read_buf(EepromDevice* dev, uint16_t addr, uint8_t* data, uint16_t len);

// ------------------------------ 操作函数集定义 ------------------------------
static const M24C64_I2cOps m24c64_i2c_ops = {
    .init        = M24C64_i2c_init,
    .check_conn  = M24C64_i2c_check_conn,
    .write_byte  = M24C64_i2c_write_byte,
    .read_byte   = M24C64_i2c_read_byte,
    .write_buf   = M24C64_i2c_write_buf,
    .read_buf    = M24C64_i2c_read_buf
};

// ------------------------------ 对外接口实现 ------------------------------

/**
 * @brief 获取M24C64操作函数集
 */
const M24C64_I2cOps* M24C64_i2c_get_ops(void) {
    return &m24c64_i2c_ops;
}

/**
 * @brief 初始化M24C64硬件私有数据
 */
bool M24C64_i2c_init_priv(M24C64_I2cPriv* priv, I2C_HandleTypeDef* hi2c) {
    if (!priv || !hi2c) return false;
    
    priv->hi2c = hi2c;
    return (hi2c->State != HAL_I2C_STATE_RESET);
}

// ------------------------------ 硬件操作实现 ------------------------------

/**
 * @brief 初始化M24C64设备
 */
static bool M24C64_i2c_init(EepromDevice* dev) {
    if (!dev || !dev->hw_priv) return false;
    
    // 从M24C64硬件参数初始化设备信息
    dev->total_size = M24C64_TOTAL_SIZE;
    dev->page_size = M24C64_PAGE_SIZE;
    dev->write_delay_ms = M24C64_WRITE_DELAY;
    
    return true;
}

/**
 * @brief 检查M24C64连接状态
 */
static bool M24C64_i2c_check_conn(EepromDevice* dev) {
    if (!dev || !dev->hw_priv) return false;
    
    M24C64_I2cPriv* priv = (M24C64_I2cPriv*)dev->hw_priv;
    
    // 调用HAL库检查I2C设备是否在线
    return (HAL_I2C_IsDeviceReady(
        priv->hi2c, 
        M24C64_I2C_ADDR, 
        3,          // 重试3次
        100         // 超时100ms
    ) == HAL_OK);
}

/**
 * @brief 写入单字节并验证
 */
static bool M24C64_i2c_write_byte(EepromDevice* dev, uint16_t addr, uint8_t data) {
    // 参数校验
    if (!dev || !dev->hw_priv || addr >= dev->total_size) return false;
    
    M24C64_I2cPriv* priv = (M24C64_I2cPriv*)dev->hw_priv;
    HAL_StatusTypeDef status;

    // 写入数据
    status = HAL_I2C_Mem_Write(
        priv->hi2c,
        M24C64_I2C_ADDR,
        addr,
        I2C_MEMADD_SIZE_16BIT,  // M24C64使用16位地址
        &data,
        1,
        100
    );
    if (status != HAL_OK) return false;

    // 等待写入完成
    HAL_Delay(dev->write_delay_ms);

    // 数据验证
    uint8_t read_back;
    return (M24C64_i2c_read_byte(dev, addr, &read_back) && read_back == data);
}

/**
 * @brief 读取单字节
 */
static bool M24C64_i2c_read_byte(EepromDevice* dev, uint16_t addr, uint8_t* data) {
    if (!dev || !dev->hw_priv || !data || addr >= dev->total_size) return false;
    
    M24C64_I2cPriv* priv = (M24C64_I2cPriv*)dev->hw_priv;
    
    return (HAL_I2C_Mem_Read(
        priv->hi2c,
        M24C64_I2C_ADDR,
        addr,
        I2C_MEMADD_SIZE_16BIT,
        data,
        1,
        100
    ) == HAL_OK);
}

/**
 * @brief 写入缓冲区（处理跨页）
 */
static bool M24C64_i2c_write_buf(EepromDevice* dev, uint16_t addr, const uint8_t* data, uint16_t len) {
    // 参数校验
    if (!dev || !dev->hw_priv || !data || len == 0 || (addr + len) > dev->total_size) {
        return false;
    }
    
    M24C64_I2cPriv* priv = (M24C64_I2cPriv*)dev->hw_priv;
    uint16_t bytes_written = 0;

    // 分批次写入（处理M24C64页边界）
    while (bytes_written < len) {
        // 计算当前页可写入长度
        uint16_t page_remaining = dev->page_size - (addr % dev->page_size);
        uint16_t write_len = (len - bytes_written) < page_remaining ? 
                            (len - bytes_written) : page_remaining;

        // 写入一页数据
        if (HAL_I2C_Mem_Write(
            priv->hi2c,
            M24C64_I2C_ADDR,
            addr,
            I2C_MEMADD_SIZE_16BIT,
			(uint8_t*)&data[bytes_written],  // 此处添加显式转换
            write_len,
            100
        ) != HAL_OK) {
            return false;
        }

        // 等待写入完成
        HAL_Delay(dev->write_delay_ms);

        // 验证数据
        uint8_t verify_buf[write_len];
        if (HAL_I2C_Mem_Read(
            priv->hi2c,
            M24C64_I2C_ADDR,
            addr,
            I2C_MEMADD_SIZE_16BIT,
            verify_buf,
            write_len,
            100
        ) != HAL_OK || memcmp(verify_buf, &data[bytes_written], write_len) != 0) {
            return false;
        }

        // 移动到下一页
        bytes_written += write_len;
        addr += write_len;
    }

    return true;
}

/**
 * @brief 读取缓冲区
 */
static bool M24C64_i2c_read_buf(EepromDevice* dev, uint16_t addr, uint8_t* data, uint16_t len) {
    if (!dev || !dev->hw_priv || !data || len == 0 || (addr + len) > dev->total_size) {
        return false;
    }
    
    M24C64_I2cPriv* priv = (M24C64_I2cPriv*)dev->hw_priv;
    
    return (HAL_I2C_Mem_Read(
        priv->hi2c,
        M24C64_I2C_ADDR,
        addr,
        I2C_MEMADD_SIZE_16BIT,
        data,
        len,
        100
    ) == HAL_OK);
}
