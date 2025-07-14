
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  InitWiFi();      
    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed, starting Access Point mode...");
    initAP();
  } else {
    Serial.println("WiFi connected successfully, starting services...");
    initMQTT();    
    initWebServer();
  }
  
  initDevice(); 
  Serial.println("=== Setup Complete ===");
}

void loop()
{
  if (!Wifi_reconnect())
  {
    if (WiFi.getMode() != WIFI_AP) {
      Serial.println("WiFi connection lost, starting Access Point...");
      initAP();
    }
  } else {
    reconnectMQTT();   
    loopWebServer();   
  }
  accpoint();
  delay(100);
}