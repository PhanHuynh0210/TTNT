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
}

void loop(){
  TaskGGsheet();
  reconnectMQTT();   
  loopWebServer();  
  accpoint(); 
  updateSystemStatus();
}