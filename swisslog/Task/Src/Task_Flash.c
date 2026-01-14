#include "main.h"
#include "LogDebugInfo.h"

#define FLASH_SAVE_ADDR  0x0800FC00  //存储从机地址的flash地址
#define SLAVE_ADDR_INIT 0X01
extern  uint8_t seted_slave_addr_current ;

HAL_StatusTypeDef Flash_Write_Data(uint32_t addr, uint16_t data)
{
    HAL_StatusTypeDef status = HAL_OK;
    FLASH_EraseInitTypeDef FlashEraseInitStruct;
    uint32_t PageError = 0;

    // 1. 解锁Flash：STM32的Flash默认上锁，必须先解锁才能擦写
    HAL_FLASH_Unlock();

    // 2. 配置擦除参数：擦除1个页(扇区)，地址为FLASH_SAVE_ADDR
    FlashEraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    FlashEraseInitStruct.PageAddress = addr;
    FlashEraseInitStruct.NbPages = 1;

    // 3. 擦除Flash指定扇区
    if(HAL_FLASHEx_Erase(&FlashEraseInitStruct, &PageError) != HAL_OK)
    {
        status = HAL_ERROR;
    }
    // 4. 写入数据到Flash（按半字写入，2字节，STM32F1必须这样写）
    if(status == HAL_OK)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, data);
    }
    // 5. 上锁Flash：写完必须上锁，防止误操作
    HAL_FLASH_Lock();

    return status;
}

/**
 * @brief  STM32F103 Flash读取函数
 * @param  addr: 要读取的Flash地址
 * @retval 读取到的16位数据
 */
uint16_t Flash_Read_Data(uint32_t addr)
{
    // 直接读取Flash地址中的值，返回16位数据
    return *(uint16_t *)addr;
}

/**
 * @brief  把从机地址seted_slave_addr_current 写入Flash，掉电永久保存
 * @param  save_addr: 要保存的从机地址（比如0x01/0x03/0x05）
 * @retval 无
 */
void SlaveAddr_SaveToFlash(uint8_t save_addr)
{
    // 写入Flash：因为Flash写入最小是2字节，直接强转成uint16_t即可
    Flash_Write_Data(FLASH_SAVE_ADDR, (uint16_t)save_addr);
    // 同时更新RAM中的全局变量
    seted_slave_addr_current = save_addr;
}

/**
 * @brief  上电初始化：从Flash读取从机地址，赋值给全局变量seted_slave_addr_current
 * @retval 读取到的从机地址
 */
uint8_t SlaveAddr_ReadFromFlash(void)
{
    uint16_t flash_data = 0;
    // 从Flash读取数据
    flash_data = Flash_Read_Data(FLASH_SAVE_ADDR);

    // ========= 修改新增 开始 =========
    // 判断：如果Flash是默认值0xFFFF → 从未设置过地址，赋值为默认初始值
    if(flash_data == 0xFFFF)
    {
        seted_slave_addr_current = SLAVE_ADDR_INIT;  //未设置时 = 你的默认值0x01
        SlaveAddr_SaveToFlash(seted_slave_addr_current); //把默认值写入Flash，下次上电直接读取
    }
    else
    // ========= 修改新增 结束 =========
    {
        // 赋值给全局变量（强转回uint8_t，只取低8位）
        seted_slave_addr_current = (uint8_t)flash_data;
    }
    // 返回读取到的地址
    DEBUGINFO("SLAVE_ADDR:%x",seted_slave_addr_current);
    return seted_slave_addr_current;
}
