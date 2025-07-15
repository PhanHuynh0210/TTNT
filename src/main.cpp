
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
    if (!Wifi_reconnect()) {
      if (WiFi.getMode() != WIFI_AP) {
        initAP();
      }
    } else {
      if (WiFi.getMode() != WIFI_STA) {
        initMQTT();
        initWebServer();
      }
    }
  }
  accpoint();
}