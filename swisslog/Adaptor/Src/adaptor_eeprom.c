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

static uint8_t Check_Fisrt_Boot()
{
	uint8_t bootTime[2] = {0,0};
	bEeprom_Check_Conn();
	bEeprom_Read_Buf(EEP_ADD_EEPROM_NEED_INIT,bootTime,2);
	if(bootTime[0] == 0x51 && bootTime[1] == 0x4d)
		return 1;
	return 0;
}

static void Update_Fisrt_Boot(void){
	uint8_t bootTime[2] = {0x51,0x4d};

	bEeprom_Check_Conn();
	bEeprom_Write_Buf(EEP_ADD_EEPROM_NEED_INIT,bootTime,2);
}

void vEeprom_Data_Init(void)
{
	uint8_t temp[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	//uint8_t temp2[8]={0x01,0x01,0x01,0x01};
	uint8_t defaultUv = 10;

	if(0==Check_Fisrt_Boot()){
    DEBUGINFO("Fisrt Boot\r\n");
		//write card password
		bEeprom_Write_Byte(EEP_ADD_IS_ENCRYED,temp[0]); 
		bEeprom_Write_Byte(EEP_ADD_CAR_STATION_STATUS,temp[0]); 	
		bEeprom_Write_Byte(EEP_ADD_UVCLEAN_TIME_MINUTES,defaultUv); 	
		bEeprom_Write_Byte(EEP_ADD_IDCARD_PASSWORD_NUM,temp[0]); 
		bEeprom_Write_Byte(EEP_ADD_EN_VIRTUAL_BUTTON,temp[0]); 
		bEeprom_Write_Byte(EEP_ADD_EN_IN_STATION_SENSOR,temp[0]); 

    bEeprom_Write_Buf(EEP_ADD_SEND_PASSWORD,temp,6); 
		bEeprom_Write_Buf(EEP_ADD_LAST_UVCLEAN_DATE,temp,7); 
		bEeprom_Write_Buf(EEP_ADD_CAR_NUMBER,temp,2); 
		// EEP_ADD_IDCARD_PASSWORD	      	  	21 //60 bytes
		//write what you want for initialize
		Update_Fisrt_Boot();
	}else{
    DEBUGINFO("Not Fisrt Boot\r\n");
  }
}

/**
 * @brief 初始化EEPROM适配层
 * 整合硬件初始化、线程安全配置和操作集绑定
 */
bool bEeprom_Adaptor_Init(void* hi2c) {
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

bool bEeprom_Check_Conn(void) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->check_conn(&eeprom_dev);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool bEeprom_Write_Byte(uint16_t addr, uint8_t data) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->write_byte(&eeprom_dev, addr, data);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool bEeprom_Read_Byte(uint16_t addr, uint8_t* data) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->read_byte(&eeprom_dev, addr, data);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool bEeprom_Write_Buf(uint16_t addr, const uint8_t* data, uint16_t len) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->write_buf(&eeprom_dev, addr, data, len);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}

bool bEeprom_Read_Buf(uint16_t addr, uint8_t* data, uint16_t len) {
    if (xSemaphoreTake(eeprom_dev.mutex, portMAX_DELAY) != pdPASS) return false;
    bool result = eeprom_dev.ops->read_buf(&eeprom_dev, addr, data, len);
    xSemaphoreGive(eeprom_dev.mutex);
    return result;
}


// ------------------------------ 测试函数 ------------------------------
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
  if (bEeprom_Write_Byte(TEST_ADDR_BYTE, test_byte)) {
    DEBUGINFO("Write byte success: addr=0x%04X, data=0x%02X\r\n", 
            TEST_ADDR_BYTE, test_byte);
            
    if (bEeprom_Read_Byte(TEST_ADDR_BYTE, &read_byte)) {
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
  if (bEeprom_Write_Buf(TEST_ADDR_BUF, test_buf, TEST_BUF_LEN)){
    DEBUGINFO("Write buffer success: addr=0x%04X, len=%d\r\n", 
            TEST_ADDR_BUF, TEST_BUF_LEN);

    // 读取缓冲区
    if (bEeprom_Read_Buf(TEST_ADDR_BUF, read_buf, TEST_BUF_LEN)) {
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
