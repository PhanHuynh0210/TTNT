#ifndef TASKHUMID_H
#define TASKHUMID_H

#include "globals.h"

extern int getHumid();
extern void initHumid();
extern void TaskHumid(void *pvParameters);

#endif // TASKHUMID_H