#ifndef ADAPTOR_INC_ADAPTOR_BOX_H_
#define ADAPTOR_INC_ADAPTOR_BOX_H_

/*************************** NumDisp *****************************/
void vSendToNumDisp(uint8_t * CmdDataArr,uint8_t len);



/*************************** Elock *****************************/
uint8_t ELock_unLock(void);



/*************************** 紫外线灯 *****************************/
void vSterilamp_enable(void);
void vSterilamp_disable(void);


/*************************** RGB LED *****************************/
void vRGB_LED(uint8_t ucColor); //RED,GREEN,BLUE

#endif /* ADAPTOR_INC_ADAPTOR_BOX_H_ */
