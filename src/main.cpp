
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  InitWiFi();
  initMQTT();
  initDevice();
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