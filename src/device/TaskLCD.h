#ifndef TASKLCD_H
#define TASKLCD_H

#include "globals.h"

extern int singleLed;
extern LiquidCrystal_I2C lcd;
extern void initLCD();

extern void Task_LCDADC(void *pvParameters);
extern void Task_LCDDHT(void *pvParameters);
extern void TaskDisLCD(void *pvParameters);

#endif // TASKLCD_H