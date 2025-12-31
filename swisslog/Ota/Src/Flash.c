#include "Param.h"
#include "Flash.h"
#include <stdio.h>
#include <string.h>
#include "app_freertos.h"
//#include "queue.h"
#include "semphr.h"

void   * hFlashMutex;

void flash_operation_init()
{
	hFlashMutex = xSemaphoreCreateMutex();
}

// Flash解锁（支持Bank1+Bank2，2MB）
uint8_t flash_unlock(void) {
    // 解锁
	xSemaphoreTake(hFlashMutex, portMAX_DELAY);
	HAL_StatusTypeDef rlt = HAL_FLASH_Unlock();
	xSemaphoreGive(hFlashMutex);

    if (rlt != HAL_OK) {
        return 0;
    }
    return 1;
}

// Flash上锁（同时锁Bank1+Bank2）
uint8_t flash_lock(void) {
    // 上锁
	xSemaphoreTake(hFlashMutex, portMAX_DELAY);
	HAL_StatusTypeDef rlt = HAL_FLASH_Lock();
	xSemaphoreGive(hFlashMutex);

    if (rlt != HAL_OK) {
        return 0;
    }
    return 1;
}

// 按扇区擦除Flash（自动适配Bank1/Bank2，计算扇区数）
uint8_t flash_erase_sectors(uint32_t start_addr, uint32_t total_len) {
    if (!flash_unlock()) {
        return 0;
    }

    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t sector_error = 0;
    uint32_t start_sector, total_sectors;

    // 计算起始扇区
    // Bank1：0x08000000~0x08100000（1MB）→ 扇区0~127（1MB/8KB=128个扇区）
    // Bank2：0x08100000~0x08200000（1MB）→ 扇区128~255（1MB/8KB=128个扇区）
    if(start_addr < APP1_START)//boot loader area
    {
    	return 0;
    }
    if (start_addr < 0x08100000)
    { // Bank1（0x08000000~0x08100000）
        start_sector = (start_addr - FLASH_BASE) / FLASH_SECTOR_SIZE;
    }
    else
    { // Bank2（0x08100000~0x08200000）
        start_sector = (start_addr - 0x08100000) / FLASH_SECTOR_SIZE;
    }

    // 计算总扇区数（向上取整）
    total_sectors = (total_len + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE;

    // 配置擦除参数
    erase_init.TypeErase   = FLASH_TYPEERASE_SECTORS;
    erase_init.Banks       = (start_addr < 0x08100000) ? FLASH_BANK_1 : FLASH_BANK_2;
    erase_init.Sector      = start_sector;
    erase_init.NbSectors   = total_sectors;

    // 执行擦除
    xSemaphoreTake(hFlashMutex, portMAX_DELAY);
    HAL_StatusTypeDef rlt = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    xSemaphoreGive(hFlashMutex);

    if (rlt != HAL_OK) {
        flash_lock();
        return 0; // 擦除失败
    }

    flash_lock();
    return 1;
}


// Flash写入（64位（QuadWord）对齐，不足补0，支持跨Bank写入）
uint8_t flash_write_data(uint32_t start_addr, uint8_t* data, uint32_t data_len) {
    if (data == NULL || data_len == 0) {
        return 0;
    }

    if (!flash_unlock()) {
        return 0;
    }

    uint32_t write_addr = start_addr;
    uint32_t write_len = data_len;
    uint32_t i = 0;
    uint8_t wBuf[16];

    // 按64位（QUADWORD）写入
    while (i < write_len) {
    	memset(wBuf, 0, 16);

        // 拼接4字节数据（不足补0）
    	for (uint8_t j = 0; j < 16 && i < write_len; j++)
		{
			wBuf[j] = data[i++];
		}

    	// 写入数据（自动适配Bank1/Bank2）
        xSemaphoreTake(hFlashMutex, portMAX_DELAY);
        HAL_StatusTypeDef rlt = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, write_addr, (uint32_t)wBuf);
        xSemaphoreGive(hFlashMutex);

		if ( rlt != HAL_OK) {
			flash_lock();
			return 0; // 写入失败
		}

        // 验证写入数据
        /*uint32_t read_data = *(volatile uint32_t*)write_addr;
        if (read_data != write_data) {
            flash_lock();
            return 0; // 写入校验失败
        }*/

        write_addr += 16;
    }

    flash_lock();
    return 1;
}

// Flash读取（支持跨Bank读取）
uint8_t flash_read_data(uint32_t start_addr, uint8_t* data, uint32_t data_len) {
    if (data == NULL || data_len == 0) {
        return 0;
    }

    xSemaphoreTake(hFlashMutex, portMAX_DELAY);
    volatile uint8_t* read_addr = (volatile uint8_t*)start_addr;
	for (uint32_t i = 0; i < data_len; i++) {
		data[i] = read_addr[i];
	}
	xSemaphoreGive(hFlashMutex);

    return 1;
}

//app:表示哪个app分区只读，param:表示哪个数据区只读
//boot+app1:0x08000000-0x080FFFFF  //sector 0-7
//boot+app2:0x08000000-0x0801FFFF,0x08100000-0x081DFFFF  //sector0, sector8-21
//param1:0x081F8000 ~ 0x081FBFFF   //sector28
//param2:0x081FC000 ~ 0x081FFFFF   //sector29
//bank1:sector 0-11  bank2:sector 12-29
uint8_t H563_Flash_Set_Readonly(Partition_TypeDef app, Partition_TypeDef param)
{
	if(PARTITION_MAX <= app || PARTITION_MAX <= param) return 0;

	FLASH_OBProgramInitTypeDef ob_config = {0};
	uint32_t wrp_sector_mask1 = 0, wrp_sector_mask2 = 0;

	ob_config.OptionType = OPTIONBYTE_WRP;
	ob_config.WRPState = OB_WRPSTATE_ENABLE;

	if(app == PARTITION_APP1)
	{
		wrp_sector_mask1 = 0x000000FF;
	}
	else if(app == PARTITION_APP2)
	{
		wrp_sector_mask1 = 0x00000F01;
		wrp_sector_mask2 = 0x003FF000;
	}

	if(param == PARTITION_APP1)
	{
		wrp_sector_mask2 |= 0x10000000;
	}
	else if(param == PARTITION_APP2)
	{
		wrp_sector_mask2 |= 0x20000000;
	}

	// 第一步：保护Bank1扇区0~1（掩码0x00000003）
	if(HAL_OK != HAL_FLASH_Unlock()) return 0;
	if(HAL_OK != HAL_FLASH_OB_Unlock()) return 0;

	ob_config.WRPSector = wrp_sector_mask1;  // 自定义保护扇区掩码
	ob_config.Banks = FLASH_BANK_1;
	if(HAL_OK != HAL_FLASHEx_OBProgram(&ob_config)) goto error;

	// 第二步：保护Bank2扇区12~13（掩码0x00003000）
	ob_config.WRPSector = wrp_sector_mask2;  // 自定义保护扇区掩码
	ob_config.Banks = FLASH_BANK_2;
	if(HAL_OK != HAL_FLASHEx_OBProgram(&ob_config)) goto error;

	// 生效并锁定
	HAL_FLASH_OB_Launch();
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();
	return 1;

error:
	HAL_FLASH_OB_Lock();
	HAL_FLASH_Lock();
	return 0;
}
