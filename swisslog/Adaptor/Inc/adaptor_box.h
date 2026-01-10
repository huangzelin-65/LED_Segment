#ifndef ADAPTOR_INC_ADAPTOR_BOX_H_
#define ADAPTOR_INC_ADAPTOR_BOX_H_

/*************************** NumDisp *****************************/
void vSendToNumDisp(uint8_t * CmdDataArr,uint8_t len);



/*************************** Elock *****************************/
//锁状态GPIO读取：0：锁上状态，1：解锁状态
//锁状态判定：0：没上锁，1：上锁
#define ELOCK1_LEVEL !(GPIO_READ(ELOCK1_STATUS))
#define ELOCK2_LEVEL !(GPIO_READ(ELOCK2_STATUS))

uint8_t ELock_unLock(void);
void vBoxELockStatusCheck(void);


/*************************** 紫外线灯 *****************************/
void v_UVClean_Enable(void);
void v_UVClean_Disable(void);


/*************************** RGB LED *****************************/
void vRGB_LED(uint8_t ucColor); //RED,GREEN,BLUE

#endif /* ADAPTOR_INC_ADAPTOR_BOX_H_ */
