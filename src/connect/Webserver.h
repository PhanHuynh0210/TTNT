#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_

#include "globals.h" 

extern WebServer server;
extern const int output18;
extern const int output21;
extern String output18State;
extern String output21State;

void initWebServer();
void handleRoot();
void handleGPIO18On();
void handleGPIO18Off();
void handleGPIO21On();
void handleGPIO21Off();
void web();

#endif
