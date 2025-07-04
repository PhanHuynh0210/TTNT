#ifndef TASK485_H
#define TASK485_H

#include "globals.h"

#define BAUD_RATE_2 9600
#define RXD_RS485 17  
#define TXD_RS485 10 
extern HardwareSerial RS485Serial;


extern void sendRS485Command(byte *command, int commandSize, byte *response, int responseSize);

#endif // TASK485_H