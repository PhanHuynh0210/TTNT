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
  Serial.println("=== Preferences Loaded ===");
Serial.println("Email: " + email);
Serial.print("Temp threshold: "); Serial.println(thresholdTemp);
Serial.print("Humid threshold: "); Serial.println(thresholdHumid);
Serial.print("Lux threshold: "); Serial.println(thresholdLux);
Serial.print("Soil threshold: "); Serial.println(thresholdSoli);
Serial.println("==========================");
}