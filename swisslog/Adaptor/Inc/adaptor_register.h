#ifndef ADAPTOR_INC_ADAPTOR_REGISTER_H_
#define ADAPTOR_INC_ADAPTOR_REGISTER_H_

#include "stdbool.h"
#include "stdint.h"
#include "stm32h5xx_hal.h"
#include "Register.h"

typedef enum {
    REGISTER_NOTIFY_GET_INFO = 0x01,
}RegisterNotify_t;

typedef struct {
	uint8_t to_register;
	uint8_t register_cnt;
}RegisterState_t;

extern RegisterState_t g_register_state;
extern Stru_Field_Register_Typedef g_register_information; 

void Register_Notify(uint32_t value);

#endif