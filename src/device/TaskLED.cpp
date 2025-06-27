#include "TaskLED.h"

int outputA = 48;

String outputAState = "off";

void handleGPIOAOn() {
  outputAState = "on";
  digitalWrite(outputA, HIGH);
  handleRoot();
}

void handleGPIOAOff() {
  outputAState = "off";
  digitalWrite(outputA, LOW);
  handleRoot();
}

void initLED() {
  pinMode(outputA, OUTPUT);
  digitalWrite(outputA, LOW);
}