#ifndef ADAPTOR_INC_ADAPTOR_HMI_H_
#define ADAPTOR_INC_ADAPTOR_HMI_H_

#define HMI_RX_BUF_SIZE  100

void vSendToHMI(uint8_t * CmdDataArr,uint8_t len); //发送数据到HMI
void vHMI_Start_DMA_Receive(uint8_t* ucHMI_Rx_Buffer); //开始接收HMI数据

uint32_t ulHMI_Get_DMA_Receive_Len(void);

#endif /* ADAPTOR_INC_ADAPTOR_HMI_H_ */
