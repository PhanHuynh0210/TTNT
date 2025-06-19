#ifndef INC_TASKRGB_H_
#define INC_TASKRGB_H_

#include "globals.h"

extern String outputRGBState;
extern void TaskRGB(void *pvParameters);
extern void setRGB();
extern void initRGB();
extern void handleGPIOCOff();
extern void handleGPIOCOn();


#endif /* INC_TASKRGB_H_ */