#include "globals.h"


void setup(){
  Serial.begin(115200);
  // clearAllSettings();
  
  initStatusRGB();
  setStatus(STATUS_BOOTING);
  InitWiFi();  
  initAPConditional(); 
  initDevice();
  initConnect();
  initSystemMonitor();
  delay(1000);
  setStatus(STATUS_NORMAL);
}

void loop(){
  TaskGGsheet();
  reconnectMQTT();   
  loopWebServer();  
  accpoint(); 
}