/*
 * OtaFlash.h
 * 承担OTA升级中的断点续传，参数获取/更新等操作
 *
 *  Created on: Dec 4, 2025
 *      Author: xusz
 */

#ifndef OTA_FLASH_OPERATION_H_
#define OTA_FLASH_OPERATION_H_

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "param.h"
#include "Flash.h"



#define UPG_PACKET_FIFO_LENGTH	512//必须设置能被8K整除的数

//与服务器通讯的head
typedef enum {
  CMD_HEAD_IDLE,
  CMD_HEAD_UPG_INFO,
  CMD_HEAD_FRAME_INFO,
  CMD_HEAD_FRAME,
  CMD_HEAD_UPG_DONE,
} OTA_CMD_HEAD_TypeDef;

//是继续升级还是停止升级的enum
typedef enum
{
	CONTINUE,
	STOPPED,
}OF_UPG_CONTINUE_TypeDef;

typedef enum
{
	Fail,
	Success,
	Upgrading
}OF_UPG_RESULT_TypeDef;
/**********************************************************/
//初始化
//同步初始化升级中flash的所有参数
BaseType_t of_init_target_param(void);


/**********************************************************/

/*
 * of_get_first_packet_number
 * 升级刚启动时，接收到上位机 "升级信息"
 *
 * 传入升级数据，同时带回需要服务器发送升级文件的packet信息
 * return, pdFALSE,运行失败，pdTRUE运行成功
 *
 * pkg[in], 两个升级文件的信息
 * upgInfo[out], 返回给服务器的升级包信息
 */
#pragma pack(push, 1)
typedef struct {
	OTA_CMD_HEAD_TypeDef head;//命令头
	uint32_t  version;		//续传文件版本号
	uint32_t  app_crc;		//续传文件crc
	uint32_t pkg_length;     // 每个包的长度
	uint32_t pkg_No;		//需要的包编号
	OF_UPG_CONTINUE_TypeDef isStop;	//是继续传输数据还是停止
} STRU_Of_Get_UpgPkg_Info_TypeDef;

typedef struct {
	OTA_CMD_HEAD_TypeDef head;//命令头
	uint32_t  version;		//升级文件版本号
	uint32_t  app_crc;		//升级文件crc
	uint8_t result;     // 升级结果: 1.成功; 0.失败
} STRU_Of_Upg_Result_TypeDef;

#pragma pack(pop)

//接收到的第一个数据包格式
typedef union
{
	uint8_t Buf[16];
	struct
	{
		uint32_t ver1;
		uint32_t crc1;
		uint32_t ver2;
		uint32_t crc2;
	}UpgInfo;
}U_Of_UpgFistPkg_TypeDef;

BaseType_t of_get_first_packet_number(const U_Of_UpgFistPkg_TypeDef *pkg, STRU_Of_Get_UpgPkg_Info_TypeDef * upgInfo);




/**********************************************************/
//接收到上位机 “数据包”  升级过程中

/*
 * of_get_next_packet_number
 * 传入升级数据，同时带回需要服务器发送的下一个packet编号
 * return, pdFALSE,运行失败，pdTRUE运行成功
 *
 * pkg[in], 升级基本信息 + 当前包数据
 * upgInfo[out], 返回给服务器的升级包信息
 */

/*升级packet信息*/
typedef union
{
	uint8_t buf[536];
	struct {
		//OTA_Upg_FW_TypeDef upginfo;
		uint32_t  version;		//续传文件版本号
		uint32_t  app_crc;		//续传文件crc
		uint32_t  total_pkg;	//总包数

		uint32_t  cur_pkg;		//当前包编号
		uint8_t	  data[512];		//当前包的数据
		uint32_t  length;		//当前包数据长度
		uint32_t  data_crc;	//当前包数据(data)CRC
	}Upg;
}U_Of_UpgPkg_TypeDef;

BaseType_t of_get_next_packet_number(const U_Of_UpgPkg_TypeDef *pkg, STRU_Of_Get_UpgPkg_Info_TypeDef * upgInfo);



/**********************************************************/
//接收到上位机 "升级完成"
//得到Flash这边的升级结果
BaseType_t of_get_upgrade_result(STRU_Of_Upg_Result_TypeDef *rlt);

#if 0
//得到APP运行的位置
static Partition_TypeDef of_get_target_partition(void);
//得到目标运行参数
static BaseType_t of_get_target_running_param(void);
//得到升级文件信息
static BaseType_t of_get_target_upg_fw_param(void);

//判断是否重复升级(比较升级信息与正在运行app的CRC)
static BaseType_t of_is_repeate_upgrade(const OTA_Upg_FW_TypeDef * upgInfo);
//比较升级文件信息是否与断点续传文件信息相同？
static BaseType_t of_compare_upg_fw_info(OTA_Upg_FW_TypeDef * upgInfo);
//得到断点续传参数
static BaseType_t of_get_resume_param(OTA_Resume_TypeDef* param);
//判断断点续传参数是否有效？
static BaseType_t resume_param_is_valid(const OTA_Resume_TypeDef* param);



//擦除并保存升级信息和断点续传信息
static BaseType_t of_save_fw_resume_info(const OTA_Upg_FW_TypeDef* param, const OTA_Resume_TypeDef* rsmP, Partition_TypeDef partion);
//擦除并保存升级信息
static BaseType_t of_save_fw_info(const OTA_Upg_FW_TypeDef* param, Partition_TypeDef partion);
//仅写入断点续传参数(注意未擦除)
static BaseType_t of_save_resume_info(uint32_t pkgNum, Partition_TypeDef partion);
//擦除FW及断点续传参数区域
static BaseType_t of_erase_fw_resume_area(Partition_TypeDef partion);


//擦除和写升级APP数据
static BaseType_t of_wr_app_upgrade_data(const OTA_Upg_Pkg_TypeDef * upgPkg, uint32_t *no);
//更新运行参数(升级最后一个packet完成)
static BaseType_t of_save_running_param(Param_And_Area* param);

static BaseType_t of_verify_target_partition(void);
#endif


#endif /* OTA_FLASH_OPERATION_H_ */


