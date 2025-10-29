#include "globals.h"


void setup(){
  Serial.begin(115200);
  initStatusRGB();  
  initDevice();
  initSystemMonitor();
}

void loop(){
  TaskGGsheet();
  reconnectMQTT();   
  accpoint(); 
  updateSystemStatus();
}