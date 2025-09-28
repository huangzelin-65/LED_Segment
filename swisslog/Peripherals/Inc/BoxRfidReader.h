/*
 * BoxRFIDReader.h
 *
 *  Created on: Sep 25, 2025
 *      Author: e3lijia25d
 */

#ifndef PERIPHERALS_INC_BOXRFIDREADER_H_
#define PERIPHERALS_INC_BOXRFIDREADER_H_

#define RFID_PASD_A				0x60
#define RFID_PASD_B				0x61

//#define RFCARD_MANUAL_READ	1 

//#define RFID_Can()		GPIO_ReadInputDataBit(RFID_Can_PORT,RFID_Can_PIN)


uint8_t M5_Halt(void);
uint8_t M5_Auth(uint8_t mode,uint8_t block,uint8_t *BufferPassword) ;
uint8_t M5_SelectCard(void);
uint8_t M5_Anticoll(void);
uint8_t M5_FindCard(uint8_t mode);
uint8_t M5_Beep(uint16_t time);
uint8_t M5_ReadWriteMode(void);
uint8_t M5_ReadOnlyMode8(uint8_t block,uint8_t passwd[6],uint8_t bitNum);

uint8_t M5_Easy_Read(uint8_t block,uint8_t pswdType,uint8_t *BufferPassword,uint8_t *readBuf);
uint8_t M5_Easy_Write(uint8_t block,uint8_t pswdType,uint8_t *BufferPassword,uint8_t *writeBuf);
uint8_t M5_Easy_ChangePswd(uint8_t block,uint8_t pswdType,uint8_t *oldPassword,uint8_t *newPasswd); 
uint8_t M5_WaitCard(uint8_t *readBuf);


void vBoxRfid_ReceiveDataHandler(uint8_t* ucReceiveData,uint32_t ucReceiveLen);

#endif /* PERIPHERALS_INC_BOXRFIDREADER_H_ */
