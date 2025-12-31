/*
 * param.h
 *
 *  Created on: Nov 11, 2025
 *      Author: xusz
 */

#ifndef INC_PARAM_H_
#define INC_PARAM_H_

#include "stdint.h"
#include "stm32h5xx_hal.h"

// 分区枚举
typedef enum {
    PARTITION_APP1 = 0,  // 分区0x08020000~0x080FFFFF（896K）0x081F8000~0x081FBFFF
    PARTITION_APP2 = 1,  // 分区0x08100000~0x081E0000（896K）0x081FC000~0x081FFFFF
    PARTITION_MAX
} Partition_TypeDef;

/************************************************************************************
 * 参数区数据
 * 前8K: Param_Config_TypeDef (运行文件信息)
 * upg_counter    active_part    app1_crc    app2_crc    self_crc  (只有更新此数据才擦除flash)
 *
 * 后8K: OTA_Upg_FW_TypeDef (升级文件信息) + N * OTA_Resume_TypeDef(断点续传信息)
 * version          app_crc       Reserved    CRC32
 * sector_no        sector_stat   sector_crc  CRC32  (secure_no顺序增加)
 * sector_no        sector_stat   sector_crc  CRC32
 * sector_no        sector_stat   sector_crc  CRC32
 * sector_no        sector_stat   sector_crc  CRC32
 * ..........
 *************************************************************************************/
//断点续传每个sector状态
//每个sector 8K, 如果每个packet = 512, 一共需要16个flag
#define SECCTOR_ALL_BIT_OK	0xFFFF//16bit全部写完成
typedef struct {
	uint32_t sector_no;		//sector编号, 0-127
	uint32_t sector_stat; 	//sector断点续传状态(每个bit对应1个packet状态)
	uint32_t total_pkg;     // OTA总包数
	uint32_t data_crc;      //前面sector(包括sector_no) APP区的CRC
	uint32_t Reserved[3];    //对齐16byte整数倍
    uint32_t crc_32;     	//结构体CRC
} OTA_Resume_TypeDef;

/*升级firmware信息*/
typedef struct {
	uint32_t  version;		//续传文件版本号
	uint32_t  app_crc;		//续传文件crc
    uint32_t  Reserved;		//对齐16byte整数倍
    uint32_t  crc_32;     	// 结构体CRC
} OTA_Upg_FW_TypeDef;

// 参数区配置（8KB+8KB参数区）
typedef struct {
	uint32_t upg_counter;//升级计数器(通过比较这个值，可以获取最新param是哪一组)
    Partition_TypeDef active_part;  // 当前运行APP的分区(注意跟参数存储area的区别)
    uint32_t app1_crc;              // App1的CRC32（全分区校验）
    uint32_t app2_crc;              // App2的CRC32（全分区校验）
    uint32_t self_crc;              // 本结构体CRC32（偏移0~sizeof-4）
} Param_Config_TypeDef;

typedef struct{
	Param_Config_TypeDef config;//区域中的配置数据
	Partition_TypeDef area;//获取到参数是哪个区，方便回存
}Param_And_Area;

// 2MB Flash分区地址定义（STM32H563：0x08000000~0x08200000）
#define FLASH_TOTAL_SIZE    0x200000UL   // 2MB = 2097152字节
#define BOOTLOADER_SIZE     0x20000UL     // 128KB Boot loader

#define APP1_START          (FLASH_BASE + BOOTLOADER_SIZE) // 0x08020000
#define APP1_SIZE           0xE0000UL   // 896KB App1（0x08020000~0x080FFFFF）

#define APP2_START          (APP1_START + APP1_SIZE) // 0x08100000
#define APP2_SIZE           0xE0000UL   // 896KB App2（0x08100000~0x081DFFFF）

#define PARAM_AREA_SIZE     0x2000UL
#define RUN_PARAM1_START    0x081F8000UL
#define RESUM_PARAM1_START  (RUN_PARAM1_START + PARAM_AREA_SIZE) // 0x081FA000UL
#define RUN_PARAM2_START    (RESUM_PARAM1_START + PARAM_AREA_SIZE) //0x081FC000UL
#define RESUM_PARAM2_START  (RUN_PARAM2_START + PARAM_AREA_SIZE) // 0x081FE000UL


// 参数区指针（强制映射到Flash地址）
#define RUN_PARAM1_PTR    ((Param_Config_TypeDef*)RUN_PARAM1_START)
#define RUN_PARAM2_PTR    ((Param_Config_TypeDef*)RUN_PARAM2_START)

//#define RESUME_PARAM1_PTR    ((Param_Config_TypeDef*)RESUM_PARAM1_START)
//#define RESUME_PARAM2_PTR    ((Param_Config_TypeDef*)RESUM_PARAM2_START)

// CRC32校验
#define CRC_NONE 0xFFFFFFFF





#endif /* INC_PARAM_H_ */


