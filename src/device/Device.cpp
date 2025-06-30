#include "Device.h"

void initDevice(){
    if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
    }
    initWebServer();
    initDHT20();
    initES35();  
    initkc();
    initLCD();
    initRGB();
    initLED();
    initLUX();
    initFan();
}