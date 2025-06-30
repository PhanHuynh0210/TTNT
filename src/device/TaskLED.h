#ifndef INC_TASKLED_H_
#define INC_TASKLED_H_

#include "globals.h"

extern int singleLed;

extern String singleLedState;

extern void initLED();
extern void setSingleLedState(String state);

#endif /* INC_TASKLED_H_ */