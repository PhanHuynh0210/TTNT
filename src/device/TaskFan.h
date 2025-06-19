#ifndef INC_TASKFAN_H_
#define INC_TASKFAN_H_

#include "globals.h"

extern void TaskFan(void *papvParameters);
extern void setFanSpeed(int speedPercent);
extern void initFan();
#endif /* INC_TASKFAN_H_ */