
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  // initDHT20();
  InitWiFi();
  initMQTT();
  // initkc();
  initWebServer();
}

void loop()
{
  if (!Wifi_reconnect())
  {
    return;
  }
  reconnectMQTT();
  web();
}