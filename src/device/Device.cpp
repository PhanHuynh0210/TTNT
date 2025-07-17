#include "Device.h"

void initDevice(){
    initWebServer();
    initDHT20();
    initGGsheet();
    // initES35();  
    // initJXBS();
    // initkc();
    // initLCD();
    // initRGB();
     initLED();
    // initLUX();
    // initFan();
}