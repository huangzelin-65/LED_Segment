#ifndef INC_STATE_H_
#define INC_STATE_H_
#include <stdbool.h>
#include <stdio.h>
#include "include_defs.h"

#define ERROR_LEVEL_LOW_  "LOW"
#define ERROR_LEVEL_HIGH_  "HIGH"

#define ERROR_TYPE_NONE_ "NONE"
#define ERROR_TYPE_MOTOR_ "MOTOR"


typedef struct {
    bool front;
    bool back;
} SensorPos
;
typedef enum {
    MOTORDIR_STOP = 0,
    MOTORDIR_FORWARD,
    MOTORDIR_BACKWARD
} MotorDir;

typedef struct {
    int speedLevel;
    MotorDir direction;
} MotorState;

typedef struct {
    bool runState;
    int runTime;
    int startTime;
    int setTime;
}UDisinfection;

typedef struct {
    const char * Type;
    const char * Level;
    int Code;
} Error_t;
typedef struct {
    char headerId[64];
    char timestamp[32];  
    char version[16];   
    const char *manufacturer;
    const char *serialNumber;
    const char *operatingMode;
    int id;//流水号
    int runtime;
    int car_running;
    uint32_t curPos;
    bool lockState1;
    bool lockState2;
    bool lockState;
    SensorPos bumperState;
    SensorPos hallState;
    MotorState motorState;
    UDisinfection disinfect;
    Error_t errors;
    uint32_t uid[3];         
    uint16_t encode_number;
} State_t;


void State_ListInit(void);
void State_Init(void);
void State_Event(int id);
void State_DeleteId(int id);

#endif
