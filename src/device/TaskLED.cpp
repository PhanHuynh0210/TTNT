#include "TaskLED.h"

int singleLed = 48;

String singleLedState = "off";

void initLED() {
  pinMode(singleLed, OUTPUT);
  digitalWrite(singleLed, LOW);
}

void setSingleLedState(String state) {
  if (state == "on") {
    digitalWrite(singleLed, HIGH);
    singleLedState = "on";
  } else if (state == "off") {
    digitalWrite(singleLed, LOW);
    singleLedState = "off";
  }
  
}