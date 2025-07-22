#include "globals.h"


void setup(){
  Serial.begin(115200);
  // clearAllSettings();
  InitWiFi();  
  initAP();    
  initDevice();
  initConnect();
}

void loop(){
  TaskGGsheet();
  reconnectMQTT();   
  loopWebServer();  
  accpoint(); 
}