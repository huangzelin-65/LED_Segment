#ifndef INC_STATE_H_
#define INC_STATE_H_
#include <stdbool.h>
#include <stdio.h>

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
} Error_t;
typedef struct {
    char headerId[64];
    char timestamp[32];  
    char version[16];   
    const char *manufacturer;
    const char *serialNumber;
    const char *operatingMode;
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


#endif
