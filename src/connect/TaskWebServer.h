#ifndef INC_TASKWEBSERVER_H_
#define INC_TASKWEBSERVER_H_

#include "globals.h"

extern void initWebServer();
extern void loopWebServer();
extern String email;
extern float thresholdTemp;
extern float thresholdHumid;
extern float thresholdLux;
extern float thresholdSoli;
extern float thresholdDistance;

extern String opTemp;
extern String opHumid;
extern String opSoli;
extern String opLux;
extern String opDistance;

#endif /* INC_TASKWEBSERVER_H_ */