/*
 * Task_Rfid.h
 *
 *  Created on: Jun 10, 2025
 *      Author: e3lijia25d
 */

#ifndef INC_TASK_RFID_H_
#define INC_TASK_RFID_H_

char* pcGetRfidCardNum(char *data, u32 RfidDataLen);
void vGetCarPosition(char *data);
void vGetTagPosType(char *data);
void vGetTagSpeed(char *data)
void vRfidTask(void *argument);

#endif /* INC_TASK_RFID_H_ */
