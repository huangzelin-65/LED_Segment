/*
 * Flash.h
 *
 *  Created on: Nov 11, 2025
 *      Author: e3lijia25d
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "stdint.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_flash.h" // 新增：包含Flash核心头文件（定义FLASH_STATE_LOCKED等）
#include "stm32h5xx_hal_flash_ex.h" // 补充：包含Flash扩展功能（如HAL_FLASHEx_Erase）

// STM32H563 Flash配置（Bank1/2，2MB Flash对应Bank1+Bank2）
#define FLASH_BANK          FLASH_BANK_1 // 主分区使用Bank1，超过1MB自动扩展到Bank2
#define FLASH_VOLTAGE_RANGE FLASH_VOLTAGE_RANGE_3 // 3.0~3.6V

//extern void* hFlashMutex;

// Flash操作函数声明
uint8_t flash_unlock(void);
uint8_t flash_lock(void);
uint8_t flash_erase_sectors(uint32_t start_addr, uint32_t total_len);
uint8_t flash_write_data(uint32_t start_addr, uint8_t* data, uint32_t data_len);
uint8_t flash_read_data(uint32_t start_addr, uint8_t* data, uint32_t data_len);

//app:表示哪个app分区只读，param:表示哪个数据区只读
//boot+app1:0x08000000-0x080FFFFF  //sector 0-7
//boot+app2:0x08000000-0x0801FFFF,0x08100000-0x081DFFFF  //sector0, sector8-21
//param1:0x081F8000 ~ 0x081FBFFF   //sector28
//param2:0x081FC000 ~ 0x081FFFFF   //sector29
//bank1:sector 0-11  bank2:sector 12-29
uint8_t H563_Flash_Set_Readonly(Partition_TypeDef app, Partition_TypeDef param);

#endif /* INC_FLASH_H_ */
