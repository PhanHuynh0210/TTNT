#include "Device.h"

void initDevice(){
    // initDHT20();
    // initkc();
    // initLCD();
     initRGB();
     initLED();
    // initLUX();
    pinMode(17, OUTPUT);
    pinMode(18, OUTPUT);

    digitalWrite(17, HIGH); 
    digitalWrite(18, HIGH);

}