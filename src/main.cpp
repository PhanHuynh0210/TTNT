
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  initDHT20();
  initMQTT();
  initkc();
  
}

void loop()
{
  if (!Wifi_reconnect())
  {
    return;
  }
  reconnectMQTT();
}