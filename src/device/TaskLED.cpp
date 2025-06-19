#include "TaskLED.h"

int outputA = 48;
int outputB = 3;

String outputAState = "off";
String outputBState = "off";

// Function to handle turning GPIO 26 on
void handleGPIOAOn() {
  outputAState = "on";
  digitalWrite(outputA, HIGH);
  // publishData("LED", "PIN_ON_1");
  handleRoot();
}

// Function to handle turning GPIO 26 off
void handleGPIOAOff() {
  outputAState = "off";
  digitalWrite(outputA, LOW);
  // publishData("LED", "PIN_OFF_1");
  handleRoot();
}

// Function to handle turning GPIO 27 on
void handleGPIOBOn() {
  outputBState = "on";
  digitalWrite(outputB, HIGH);
  // publishData("LED", "PIN_ON_2");
  handleRoot();
}

// Function to handle turning GPIO 27 off
void handleGPIOBOff() {
  outputBState = "off";
  digitalWrite(outputB, LOW);
  // publishData("LED", "PIN_OFF_2");
  handleRoot();
}

void initLED() {

  // Initialize GPIO pins
  pinMode(outputA, OUTPUT);
  pinMode(outputB, OUTPUT);
  
  // Set initial states
  digitalWrite(outputA, LOW);
  digitalWrite(outputB, LOW);
  
}