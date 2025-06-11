#ifndef INC_TASKDHT20_H_
#define INC_TASKDHT20_H_

#include "globals.h"

extern void initDHT20();
extern void getValueDHT20(); 
extern void TaskTemperatureHumidity(void *pvParameters);
#endif /* INC_TASKDHT20_H_ */