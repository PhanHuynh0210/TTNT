
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  InitWiFi();
  initMQTT();
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }
  initWebServer();
  // initDHT20();
  initES35();  
  // initkc();
  // initLCD();
  // initRGB();
  // initLED();
  // initLUX();
  // initFan();
}

void loop()
{
  if (!Wifi_reconnect())
  {
    return;
  }
  reconnectMQTT();
  loopWebServer();
}