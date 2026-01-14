#ifndef _TASK_FLASH_H
#define _TASK_FLASH_H

HAL_StatusTypeDef Flash_Write_Data(uint32_t addr, uint16_t data);
uint16_t Flash_Read_Data(uint32_t addr);
void SlaveAddr_SaveToFlash(uint8_t save_addr);
uint8_t SlaveAddr_ReadFromFlash(void);

#endif
