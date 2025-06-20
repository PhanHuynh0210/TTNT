#ifndef INC_TASKRGB_H_
#define INC_TASKRGB_H_

#include "globals.h"

extern String outputRGBState;
extern String rgbMode;
extern int customRed, customGreen, customBlue;

extern void TaskRGB(void *pvParameters);
extern void setRGBMode(String mode);
extern void initRGB();
extern void setCustomColor(int r, int g, int b);
extern void setRGBMode(String mode);
extern void handleGPIOCOff();
extern void handleGPIOCOn();
extern void handleRainbowMode();
extern void handleAutoMode();
extern void handleStaticMode();

#endif /* INC_TASKRGB_H_ */