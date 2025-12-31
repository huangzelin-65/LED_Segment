/*
 * OtaFlash.c
 * 承担OTA升级中的断点续传，参数获取/更新等操作
 *
 *  Created on: Dec 4, 2025
 *      Author: xusz
 */

#include <stdio.h>
#include "string.h"
#include "stm32h5xx_hal.h"
#include "Calculate.h"
#include "OtaFlash.h"
#include "LogDebugInfo.h"

//保存升级flash的结果
static OF_UPG_RESULT_TypeDef s_ucUpgradeResult = Upgrading;

//当前需要接收包的编号
static uint32_t s_uiUpgPacketNo = 1;//从1开始

//保存升级的APP flash位置
static Partition_TypeDef s_eTargetAppPar = PARTITION_MAX;

//保存运行APP的相关参数，及更新的参数区位置
static Param_And_Area s_struTargetRunningParam = {0};

//保存升级Firmware信息
static OTA_Upg_FW_TypeDef s_struUpgFwParam= {0};

//保存断点续传信息
static OTA_Resume_TypeDef s_struResumeParam= {0};



/**********************************************/






/*****************************************************************************/
// 获取OTA目标分区（当前分区的另一分区）
static Partition_TypeDef of_get_target_partition(void)
{
	// SCB->VTOR 存储当前向量表起始地址（STM32H5 V1.5.0 标准用法）
	uint32_t vector_addr = SCB->VTOR;
	if (vector_addr == APP1_START) {
		return PARTITION_APP2;
	} else if (vector_addr == APP2_START) {
		return PARTITION_APP1;
	}
	return PARTITION_MAX;
}

// 获取OTA使用的参数分区
static BaseType_t of_get_target_running_param(void) {
	//
	Param_And_Area areaParam1= {0}, areaParam2 = {0};
    uint32_t low_crc = 0;
    uint32_t high_crc = 0;

    areaParam1.area = PARTITION_APP1;
    areaParam2.area = PARTITION_APP2;

    // 读取两个运行参数区并校验CRC
    flash_read_data((uint32_t)RUN_PARAM1_PTR, (uint8_t*)&(areaParam1.config), sizeof(Param_Config_TypeDef));
    flash_read_data((uint32_t)RUN_PARAM2_PTR, (uint8_t*)&(areaParam2.config), sizeof(Param_Config_TypeDef));

    //分别计算CRC
    low_crc = crc32_calc((uint32_t)RUN_PARAM1_PTR, sizeof(Param_Config_TypeDef) - 4);
    high_crc = crc32_calc((uint32_t)RUN_PARAM2_PTR, sizeof(Param_Config_TypeDef) - 4);

    //两个CRC都OK
    if((low_crc == areaParam1.config.self_crc) && (high_crc == areaParam2.config.self_crc))
    {
    	//判断哪一个区的数据是旧数据
    	//copy最新参数到s_struTargetRunningParam，但运行参数区记录为旧的，为未来保存做基础
    	if(areaParam1.config.upg_counter < areaParam2.config.upg_counter)
    	{
    		memcpy(&s_struTargetRunningParam, &areaParam2, sizeof(Param_And_Area));
    		s_struTargetRunningParam.area = PARTITION_APP1;
    	}
    	else
    	{
    		memcpy(&s_struTargetRunningParam, &areaParam1, sizeof(Param_And_Area));
    		s_struTargetRunningParam.area = PARTITION_APP2;
    	}
    	return pdTRUE;
    }
    else if ((low_crc == areaParam1.config.self_crc)&&(high_crc != areaParam2.config.self_crc))
    {
    	//copy最新参数到s_struTargetRunningParam，但运行参数区记录为CRC不对的那块区，为未来保存做基础
    	memcpy(&s_struTargetRunningParam.config, &areaParam1.config, sizeof(Param_Config_TypeDef));
    	s_struTargetRunningParam.area = PARTITION_APP2;
        return pdTRUE; // 主区有效，直接返回
    }
    else if ((low_crc != areaParam1.config.self_crc)&&(high_crc == areaParam2.config.self_crc))
	{
    	//当前参数copy并保存到另一个区域
    	memcpy(&s_struTargetRunningParam.config, &areaParam2.config, sizeof(Param_Config_TypeDef));
    	s_struTargetRunningParam.area = PARTITION_APP1;
		return pdTRUE; // 备份区有效
	}
    return pdFALSE;
}

//
static BaseType_t of_get_target_upg_fw_param(void)
{
	uint32_t getCrc=0;

	memset(&s_struUpgFwParam, 0, sizeof(OTA_Upg_FW_TypeDef));

	//从指定的参数区读取升级FW的信息
	if(s_struTargetRunningParam.area == PARTITION_APP1)
	{
		flash_read_data((uint32_t)RESUM_PARAM1_START, (uint8_t*)&s_struUpgFwParam, sizeof(OTA_Upg_FW_TypeDef));
		getCrc = crc32_calc((uint32_t)RESUM_PARAM1_START, sizeof(OTA_Upg_FW_TypeDef) - 4);
	}
	else if(s_struTargetRunningParam.area == PARTITION_APP2)
	{
		flash_read_data((uint32_t)RESUM_PARAM2_START, (uint8_t*)&s_struUpgFwParam, sizeof(OTA_Upg_FW_TypeDef));
		getCrc = crc32_calc((uint32_t)RESUM_PARAM2_START, sizeof(OTA_Upg_FW_TypeDef) - 4);
	}
	else
		return pdFALSE;

	//如果当前参数有效
	if(getCrc == s_struUpgFwParam.crc_32)
	{
		return pdTRUE;
	}
	else return pdFALSE;
}

static BaseType_t of_is_repeate_upgrade(const OTA_Upg_FW_TypeDef * upgInfo)
{
	//写入APP Flash的目标区
	if(s_eTargetAppPar == PARTITION_APP1)
	{
		//对比当前APP的CRC 与 准备升级的CRC，确定是否重复升级
		if(upgInfo->app_crc == s_struTargetRunningParam.config.app2_crc)
		{
			return pdTRUE;//重复升级
		}
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		//对比当前APP的CRC 与 准备升级的CRC，确定是否重复升级
		if(upgInfo->app_crc == s_struTargetRunningParam.config.app1_crc)
		{
			return pdTRUE;//重复升级
		}
	}

	return pdFALSE;
}

static BaseType_t of_compare_upg_fw_info(const OTA_Upg_FW_TypeDef * upgInfo)
{
	if((upgInfo->app_crc == s_struUpgFwParam.app_crc) && (upgInfo->version == s_struUpgFwParam.version))
	{
		return pdTRUE;
	}
	else
		return pdFALSE;
}

// 直接校验目标分区固件（核心：三重校验，与Bootloader一致）
static BaseType_t of_verify_target_partition(void)
{
    uint32_t target_start = (s_eTargetAppPar == PARTITION_APP1) ? APP1_START : APP2_START;
    uint32_t target_size = (s_eTargetAppPar == PARTITION_APP1) ? APP1_SIZE : APP2_SIZE;

    // 1. 校验栈顶地址（STM32H5 SRAM：0x20000000~0x200A0000）
    uint32_t stack_top = *(volatile uint32_t*)target_start;
    if (stack_top < 0x20000000 || stack_top > 0x200A0000)
    {
        return pdFALSE;
    }

    // 2. 校验复位向量地址（必须在目标分区内）
    uint32_t reset_vec = *(volatile uint32_t*)(target_start + 4);
    if (reset_vec < target_start || reset_vec > (target_start + target_size)) {
        return pdFALSE;
    }

    // 3. 校验CRC32（全分区校验）
    uint32_t target_crc = crc32_calc(target_start, target_size);

    // 比较升级包中的CRC与实际APP Flash计算结果
    if(target_crc == s_struUpgFwParam.app_crc)
    	return pdTRUE;
    else
    	return pdFALSE;
}

static BaseType_t of_wr_app_upgrade_data(const U_Of_UpgPkg_TypeDef * upgPkg, uint32_t *no)
{
	if(upgPkg->Upg.cur_pkg == 0)
	{
		*no = 1;//从第1包开始
		return pdFALSE;
	}

	//写APP的flash
	uint32_t secno = (upgPkg->Upg.cur_pkg -1)/ (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH);

	//防止越界
	if(127 < secno)
	{
		*no = 1;//从第1包开始
		return pdFALSE;
	}

	//当取%为0时，说明需要写入新的sector
	uint32_t remainpkg = (upgPkg->Upg.cur_pkg-1) % (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH);

	//计算sector和packet的地址
	uint32_t secAddr = secno * FLASH_SECTOR_SIZE;
	uint32_t pkgAddr = remainpkg * UPG_PACKET_FIFO_LENGTH;

	//分区计算sector和packet的地址
	if(s_eTargetAppPar == PARTITION_APP1)
	{
		pkgAddr = APP1_START + secAddr + pkgAddr;
		secAddr = APP1_START + secAddr;
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		pkgAddr = APP2_START + secAddr + pkgAddr;
		secAddr = APP2_START + secAddr;
	}
	else
	{
		*no = 1;
		return pdFALSE;
	}

	//当前sector的第一个packet，擦除整个sector
	if(0 == remainpkg)
	{
		//擦除flash sector失败
		if (!flash_erase_sectors(secAddr, FLASH_SECTOR_SIZE))
		{
			*no = 1;//从第一包开始，同时返回错误
			return pdFALSE;
		}
	}

	//写入当前包(APP数据)
	if (!flash_write_data(pkgAddr, (uint8_t*)upgPkg->Upg.data, upgPkg->Upg.length))
	{
		//当前sector全部重来,所以包编号为当前sector的第一个
		*no = secno * (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH) + 1;
		return pdFALSE;
	}

	//读回来, 比较当前数据段的CRC
	if(upgPkg->Upg.data_crc != crc32_calc(pkgAddr, upgPkg->Upg.length))
	{
		//当前sector全部重来,所以包编号为当前sector的第一个
		*no = secno * (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH) + 1;
		return pdFALSE;//返回错误
	}

	//写成功
	*no = upgPkg->Upg.cur_pkg + 1;
	return pdTRUE;
}

//不擦除，然后保存断点续传参数
static BaseType_t of_save_resume_info(OTA_Resume_TypeDef* rsmP, uint32_t pkgNum, Partition_TypeDef partion)
{
	if(pkgNum == 0) return pdFALSE;
	//1-pkgNum
	uint32_t remainPkg = pkgNum % (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH);
	if(0 != remainPkg) return pdTRUE;

	//准备断点续传参数
	//OTA_Resume_TypeDef rsmP;
	rsmP->sector_no = pkgNum / (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH);
	rsmP->sector_stat = SECCTOR_ALL_BIT_OK;
	rsmP->Reserved[0]=rsmP->Reserved[1]=rsmP->Reserved[2]=0;

	//计算存储APP的crc32
	if(s_eTargetAppPar == PARTITION_APP1)
	{
		rsmP->data_crc = crc32_calc(APP1_START, (rsmP->sector_no) * FLASH_SECTOR_SIZE);
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		rsmP->data_crc = crc32_calc(APP2_START, (rsmP->sector_no) * FLASH_SECTOR_SIZE);
	}
	else
		return pdFALSE;

	rsmP->sector_no -= 1;

	//计算结构体自身的CRC
	rsmP->crc_32 = crc32_calc((uint32_t)rsmP, sizeof(OTA_Resume_TypeDef) - 4);

	//计算断点续传参数的存储位置
	uint32_t secAddr = sizeof(OTA_Upg_FW_TypeDef) + rsmP->sector_no * sizeof(OTA_Resume_TypeDef);
	if(partion == PARTITION_APP1)
	{
		//写入升级续传信息
		if (!flash_write_data(RESUM_PARAM1_START + secAddr, (uint8_t*)rsmP, sizeof(OTA_Resume_TypeDef))) {
			return pdFALSE;
		}
	}
	else if(partion == PARTITION_APP2)
	{
		//写入升级续传信息
		if (!flash_write_data(RESUM_PARAM2_START + secAddr, (uint8_t*)rsmP, sizeof(OTA_Resume_TypeDef))) {
			return pdFALSE;
		}
	}
	else return pdFALSE;

	return pdTRUE;
}

//擦除FW及断点续传参数区域
static BaseType_t of_erase_fw_resume_area(Partition_TypeDef partion)
{
	if(partion == PARTITION_APP1)
	{
		if (!flash_erase_sectors(RESUM_PARAM1_START, PARAM_AREA_SIZE)) {
			return pdFALSE;
		}
	}
	else if(partion == PARTITION_APP2)
	{
		if (!flash_erase_sectors(RESUM_PARAM2_START, PARAM_AREA_SIZE)) {
			return pdFALSE;
		}
	}
	else return pdFALSE;

	return pdTRUE;
}

//擦除，然后保存升级参数和断点续传参数
static BaseType_t of_save_fw_resume_info(const OTA_Upg_FW_TypeDef* param, const OTA_Resume_TypeDef* rsmP, Partition_TypeDef partion)
{
	if (param == NULL || rsmP==NULL) {
		return pdFALSE;
	}

	uint32_t secAddr = 0x00000000UL;

	//擦除FW及断点续传参数区域
	if(pdTRUE != of_erase_fw_resume_area(partion))
		return pdFALSE;

	if(partion == PARTITION_APP1)
	{
		//写入升级FW信息
		if (!flash_write_data(RESUM_PARAM1_START, (uint8_t*)param, sizeof(OTA_Upg_FW_TypeDef))) {
			return pdFALSE;
		}
		//写入升级续传信息到指定位置
		secAddr = sizeof(OTA_Upg_FW_TypeDef) + rsmP->sector_no * sizeof(OTA_Resume_TypeDef);
		if (!flash_write_data(RESUM_PARAM1_START + secAddr, (uint8_t*)rsmP, sizeof(OTA_Resume_TypeDef))) {
			return pdFALSE;
		}
	}
	else if(partion == PARTITION_APP2)
	{
		//写入升级FW信息
		if (!flash_write_data(RESUM_PARAM2_START, (uint8_t*)param, sizeof(OTA_Upg_FW_TypeDef))) {
			return pdFALSE;
		}
		//写入升级续传信息
		secAddr = sizeof(OTA_Upg_FW_TypeDef) + rsmP->sector_no * sizeof(OTA_Resume_TypeDef);
		if (!flash_write_data(RESUM_PARAM2_START + secAddr, (uint8_t*)rsmP, sizeof(OTA_Resume_TypeDef))) {
			return pdFALSE;
		}
	}
	else return pdFALSE;

	return pdTRUE;
}

//擦除，然后仅仅保存升级参数
static BaseType_t of_save_fw_info(OTA_Upg_FW_TypeDef* param, Partition_TypeDef partion)
{
	if (param == NULL) {
		return pdFALSE;
	}

	// 更新自身CRC
	param->crc_32 = crc32_calc((uint32_t)param, sizeof(OTA_Upg_FW_TypeDef) - 4);

	//擦除FW及断点续传参数区域
	if(pdTRUE != of_erase_fw_resume_area(partion))
		return pdFALSE;

	//写入升级FW 参数
	if(partion == PARTITION_APP1)
	{
		if (!flash_write_data(RESUM_PARAM1_START, (uint8_t*)param, sizeof(OTA_Upg_FW_TypeDef))) {
			return pdFALSE;
		}
	}
	else if(partion == PARTITION_APP2)
	{
		if (!flash_write_data(RESUM_PARAM2_START, (uint8_t*)param, sizeof(OTA_Upg_FW_TypeDef))) {
			return pdFALSE;
		}
	}
	else return pdFALSE;

	return pdTRUE;
}

// 保存运行参数
static BaseType_t of_save_running_param(Param_And_Area* param)
{
    if (param == NULL) {
        return pdFALSE;
    }

    //更新APP flash存储目标区域CRC
	if(s_eTargetAppPar == PARTITION_APP1)
	{
		param->config.active_part = PARTITION_APP1;
		param->config.app1_crc = s_struUpgFwParam.app_crc;//update app1 crc
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		param->config.active_part = PARTITION_APP2;
		param->config.app2_crc = s_struUpgFwParam.app_crc;//update app2 CRC
	}
	else
		return pdFALSE;

	param->config.upg_counter = param->config.upg_counter + 1;//update counter

	//更新自身CRC
	param->config.self_crc = crc32_calc((uint32_t)&param->config, sizeof(Param_Config_TypeDef) - 4);

	//更新运行参数区
    if(param->area == PARTITION_APP1)
    {
    	//erase running parameters area
		if (!flash_erase_sectors(RUN_PARAM1_START, PARAM_AREA_SIZE)) {
			return pdFALSE;
		}
		//write running parameters area
		if (!flash_write_data(RUN_PARAM1_START, (uint8_t*)&param->config, sizeof(Param_Config_TypeDef))) {
			return pdFALSE;
		}
    }
    else if(param->area == PARTITION_APP2)
    {
    	//erase running parameters area
		if (!flash_erase_sectors(RUN_PARAM2_START, PARAM_AREA_SIZE)) {
			return pdFALSE;
		}
		//write running parameters area
		if (!flash_write_data(RUN_PARAM2_START, (uint8_t*)&param->config, sizeof(Param_Config_TypeDef))) {
			return pdFALSE;
		}
    }
    else return pdFALSE;

    return pdTRUE;
}

static BaseType_t resume_param_is_valid(const OTA_Resume_TypeDef* param)
{
	//状态位不对
	if(param->sector_stat != SECCTOR_ALL_BIT_OK) return pdFALSE;

	//sector编号不对
	if(127 < param->sector_no) return pdFALSE;

	//自己的CRC不对
	if(param->crc_32 != crc32_calc((uint32_t)param, sizeof(OTA_Resume_TypeDef) - 4)) return pdFALSE;

	//检查Flash中数据的CRC
	uint32_t crc=0;
	if(s_eTargetAppPar == PARTITION_APP1)
	{
		crc = crc32_calc(APP1_START, (1+param->sector_no) * FLASH_SECTOR_SIZE);
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		crc = crc32_calc(APP2_START, (1+param->sector_no) * FLASH_SECTOR_SIZE);
	}

	//参数sector_crc有效
	if(crc == param->data_crc)
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}

static BaseType_t of_get_resume_param(OTA_Resume_TypeDef* param)
{
	OTA_Resume_TypeDef temp={0};

	if(param == NULL) return pdFALSE;

	//uint8_t getParam = 0;

	//清零
	memset(param, 0, sizeof(OTA_Resume_TypeDef));

	//循环读取参数区保存的续传数据，并判断
	for(int8_t i=127; 0<=i; i--)
	{
		uint32_t offset = sizeof(OTA_Upg_FW_TypeDef) + i * sizeof(OTA_Resume_TypeDef);

		if(s_struTargetRunningParam.area == PARTITION_APP1)
		{
			//读取参数区保存的续传数据
			if (!flash_read_data((RESUM_PARAM1_START + offset), (uint8_t*)&temp, sizeof(OTA_Resume_TypeDef))) {
				return pdFALSE;
			}
		}
		else if(s_struTargetRunningParam.area == PARTITION_APP2)
		{
			//读取参数区保存的续传数据
			if (!flash_read_data((RESUM_PARAM2_START + offset), (uint8_t*)&temp, sizeof(OTA_Resume_TypeDef))) {
				return pdFALSE;
			}
		}
		else
			return pdFALSE;

		//检验参数是否有效
		if(pdTRUE == resume_param_is_valid(&temp))
		{
			//保存参数
			memcpy(param, &temp, sizeof(OTA_Resume_TypeDef));
			return pdTRUE;
		}
	}

	return pdFALSE;
}

//同步初始化OTA升级中flash的所有参数
BaseType_t of_init_target_param(void)
{
	flash_operation_init();

	//获取升级时读写的是那个分区
	s_eTargetAppPar = of_get_target_partition();
	if(s_eTargetAppPar == PARTITION_MAX)
	{
		printf("Get target app area error!\r\n");
		return pdFALSE;
	}
	else if(s_eTargetAppPar == PARTITION_APP1)
	{
		printf("APP B Running!\n");
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		printf("APP A Running!\n");
	}

	//获取到需更新参数区的运行参数
	if(pdTRUE != of_get_target_running_param())
	{
		printf("Get target running parameter fail!\n");
		return pdFALSE;
	}

	//得到参数存储区中的升级FW参数s_struUpgFwParam, 如果没有，返回0
	if(pdTRUE != of_get_target_upg_fw_param())
	{
		//没有断点续传参数，准备全新升级
		printf("Get target upgrade parameter fail!\n");
		return pdFALSE;
	}

	return pdTRUE;
}

//传入升级信息，同时带回需要服务器发送的下一个packet编号
BaseType_t of_get_first_packet_number(const U_Of_UpgFistPkg_TypeDef *pkg, STRU_Of_Get_UpgPkg_Info_TypeDef * upgInfo)
{
	s_ucUpgradeResult = Upgrading;
	upgInfo->head = CMD_HEAD_FRAME_INFO;
	if(PARTITION_MAX <= s_eTargetAppPar)
	{
		printf("there isn't upgrade flash!\n");
		upgInfo->isStop = STOPPED;
		return pdFALSE;
	}
	//把收到的数据转换成版本号和CRC
	OTA_Upg_FW_TypeDef temp;
	temp.Reserved = 0;
	if(s_eTargetAppPar == PARTITION_APP1)//文件需要写入APP1的flash
	{
		temp.version = (pkg->UpgInfo.ver1 < pkg->UpgInfo.ver2) ? pkg->UpgInfo.ver1: pkg->UpgInfo.ver2;
		temp.app_crc = (pkg->UpgInfo.ver1 < pkg->UpgInfo.ver2) ? pkg->UpgInfo.crc1: pkg->UpgInfo.crc2;
	}
	else if(s_eTargetAppPar == PARTITION_APP2)
	{
		temp.version = (pkg->UpgInfo.ver1 < pkg->UpgInfo.ver2) ? pkg->UpgInfo.ver2: pkg->UpgInfo.ver1;
		temp.app_crc = (pkg->UpgInfo.ver1 < pkg->UpgInfo.ver2) ? pkg->UpgInfo.crc2: pkg->UpgInfo.crc1;
	}

	upgInfo->version = temp.version;
	upgInfo->app_crc = temp.app_crc;

	upgInfo->pkg_length = UPG_PACKET_FIFO_LENGTH;
	//upgInfo->total_pkg = 1792;

	//判断当前运行的APP chksum是否与即将升级的一致？
	//防止重复升级,重复升级直接返回升级失败
	if(pdTRUE == of_is_repeate_upgrade(&temp))
	{
		printf("repeated upgrade!\n");
		upgInfo->isStop = STOPPED;
		upgInfo->pkg_No = 1;
		return pdFALSE;
	}

	//判断当前需要升级的信息是否与断点续传的信息相同
	if(pdTRUE != of_compare_upg_fw_info(&temp))
	{
		goto UPG_FROM_ZERO;
	}

	//获取断点续传sector信息
	memset(&s_struResumeParam, 0, sizeof(OTA_Resume_TypeDef));
	if(pdTRUE != of_get_resume_param(&s_struResumeParam))
	{
		goto UPG_FROM_ZERO;
	}

	//保存fw和断点续传信息
	if(pdTRUE != of_save_fw_resume_info(&s_struUpgFwParam, &s_struResumeParam, s_struTargetRunningParam.area))
	{
		printf("save upgrade and resume info fail\n");
		goto UPG_FROM_ZERO;
	}

	//通过sector回算下一个packet编号
	s_uiUpgPacketNo = (1 + s_struResumeParam.sector_no) * (FLASH_SECTOR_SIZE/UPG_PACKET_FIFO_LENGTH) +1;

	//此处无法考虑升级包个数是否大于包总数
	upgInfo->isStop = CONTINUE;
	upgInfo->pkg_No = s_uiUpgPacketNo;
	//printf("upgrading start packet is %lu\n", s_uiUpgPacketNo);
	return pdTRUE;

UPG_FROM_ZERO:
	upgInfo->isStop = CONTINUE;
	upgInfo->pkg_No = s_uiUpgPacketNo = 1;//从第1包开始

	//擦除UPG_FW以及续传数据
	if(pdTRUE != of_save_fw_info(&temp, s_struTargetRunningParam.area))
	{
		upgInfo->isStop = STOPPED;
		printf("save upgrade firmware info fail\n");
		return pdFALSE;
	}

	//读回到全局变量中
	if(pdTRUE != of_get_target_upg_fw_param())
	{
		printf("Get target upgrade parameter fail!\n");
	}

	//printf("start upgrade,param area %d\n",s_struTargetRunningParam.area);
	return pdTRUE;
}


//传入升级数据，同时带回需要服务器发送的下一个packet编号
//BaseType_t of_get_next_packet_number(const OTA_Upg_Pkg_TypeDef * upgPkg, uint32_t *no, OF_UPG_CONTINUE_TypeDef *isStop);
BaseType_t of_get_next_packet_number(const U_Of_UpgPkg_TypeDef *pkg, STRU_Of_Get_UpgPkg_Info_TypeDef * upgInfo)
{
	OTA_Upg_FW_TypeDef UpgFw;
	upgInfo->head = CMD_HEAD_FRAME_INFO;
	upgInfo->version = UpgFw.version = pkg->Upg.version;
	upgInfo->app_crc = UpgFw.app_crc = pkg->Upg.app_crc;
	upgInfo->pkg_length = UPG_PACKET_FIFO_LENGTH;
	UpgFw.Reserved=0;

	s_ucUpgradeResult = Upgrading;
	//升级文件信息比较
	if(pdTRUE != of_compare_upg_fw_info(&UpgFw))
	{
		//升级包信息有误，非断点续传，全新升级
		upgInfo->pkg_No = s_uiUpgPacketNo = 1;//从第1包开始

		memcpy(&s_struUpgFwParam, &UpgFw, sizeof(OTA_Upg_FW_TypeDef));

		//擦除UPG_FW以及续传数据
		if(pdTRUE != of_save_fw_info(&s_struUpgFwParam, s_struTargetRunningParam.area))
		{
			goto UPG_ERR_STOP;
		}
		else
		{
			upgInfo->isStop = CONTINUE;
			return pdTRUE;
		}
	}

	//升级包编号比较(不相等意味着不是需要的packet)
	if(pkg->Upg.cur_pkg != s_uiUpgPacketNo)
	{
		//再次请求s_uiUpgPacketNo包
		upgInfo->pkg_No = s_uiUpgPacketNo;
		upgInfo->isStop = CONTINUE;
		return pdTRUE;
	}

	//写APP Flash
	if(pdTRUE != of_wr_app_upgrade_data(pkg, &s_uiUpgPacketNo))
	{
		//写APP flash失败，停止升级
		goto UPG_ERR_STOP;
	}

	s_struResumeParam.total_pkg = pkg->Upg.total_pkg;

	//如果是sector的最后一个packet，需要更新断点续传参数
	if(pdTRUE != of_save_resume_info(&s_struResumeParam, pkg->Upg.cur_pkg, s_struTargetRunningParam.area) )
	{
		//写入断点续传参数失败，停止升级
		goto UPG_ERR_STOP;
	}

	//判断是否为最后一个packet
	if(pkg->Upg.total_pkg < s_uiUpgPacketNo )
	{
		upgInfo->isStop = STOPPED;

		//检查计算出的断点续传CRC是否与升级文件CRC一致
		if(s_struResumeParam.data_crc != s_struUpgFwParam.app_crc)
		{
			s_ucUpgradeResult = Fail;
			of_erase_fw_resume_area(s_struTargetRunningParam.area);//
			printf("Resume.data_crc != upgFw.app_crc\n");
			//写入断点续传参数失败，停止升级
			goto UPG_ERR_STOP;
		}

		//再次验证升级区最后的参数
		if(of_verify_target_partition() != pdTRUE)
		{
			s_ucUpgradeResult = Fail;
			of_erase_fw_resume_area(s_struTargetRunningParam.area);//
			printf("target app verify failed!\n");
			//APP不正确，不能运行
			goto UPG_ERR_STOP;
		}

		//擦除断点续传参数，更新运行参数
		if(of_erase_fw_resume_area(s_struTargetRunningParam.area) != pdTRUE)
		{
			s_ucUpgradeResult = Fail;
			printf("Erase resume area failed!\n");
			//APP不正确，不能运行
			goto UPG_ERR_STOP;
		}

		//of_update_target_running_param(&s_struTargetRunningParam.config);
		if(of_save_running_param(&s_struTargetRunningParam) != pdTRUE)
		{
			s_ucUpgradeResult = Fail;
			printf("Save Running failed!\n");
			//APP运行参数保存失败
			goto UPG_ERR_STOP;
		}
		s_ucUpgradeResult = Success;
	}
	else
	{
		//s_uiUpgPacketNo++;
		upgInfo->pkg_No = s_uiUpgPacketNo;
		upgInfo->isStop = CONTINUE;
	}
	return pdTRUE;

UPG_ERR_STOP:
	upgInfo->pkg_No = s_uiUpgPacketNo = 1;//从第1包开始
	upgInfo->isStop = STOPPED;
	return pdFALSE;
}

BaseType_t of_get_upgrade_result(STRU_Of_Upg_Result_TypeDef *rlt)
{
	if(s_ucUpgradeResult == Upgrading) return pdFALSE;

	rlt->head = CMD_HEAD_UPG_DONE;
	rlt->app_crc = s_struUpgFwParam.app_crc;
	rlt->version = s_struUpgFwParam.version;
	rlt->result = s_ucUpgradeResult;

	return pdTRUE;
}



