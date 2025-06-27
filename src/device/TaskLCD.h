#ifndef TASKLCD_H
#define TASKLCD_H

#include "globals.h"
extern void Task_LCDADC(void *pvParameters);
extern void Task_LCDDHT(void *pvParameters);
extern void TaskDisLCD(void *pvParameters);
extern void initLCD();
extern int outputA;
extern LiquidCrystal_I2C lcd;


#endif // TASKLCD_H