#ifndef INC_TASKLED_H_
#define INC_TASKLED_H_

#include "globals.h"

// Assign output variables to GPIO pins
extern int outputA;
extern int outputB;

extern String outputAState;
extern String outputBState;

extern void handleGPIOAOn();
extern void handleGPIOAOff();
extern void handleGPIOBOn();
extern void handleGPIOBOff();

extern void initLED();

#endif /* INC_TASKLED_H_ */