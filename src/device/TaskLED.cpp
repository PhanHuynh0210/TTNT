#include "TaskLED.h"

int singleLed = 48;

String singleLedState = "off";

void initLED() {
  pinMode(singleLed, OUTPUT);
  digitalWrite(singleLed, LOW);
}