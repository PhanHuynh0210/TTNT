
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  InitWiFi();      
  if (WiFi.status() != WL_CONNECTED) {
    initAP();
  } else {
    initMQTT();    
    initWebServer();
  }
    initDevice(); 
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) {
    reconnectMQTT();   
    loopWebServer();   
  } else {
    if (WiFi.getMode() != WIFI_AP && WiFi.getMode() != WIFI_AP_STA) {
      initAP();
    }
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 30000) { 
      lastReconnectAttempt = millis();
      if (Wifi_reconnect() && WiFi.status() == WL_CONNECTED) {
        initMQTT();
        initWebServer();
      }
    }
  }
  accpoint();
}