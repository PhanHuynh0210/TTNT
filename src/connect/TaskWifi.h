#ifndef INC_TASKWIFI_H_
#define INC_TASKWIFI_H_

#include "globals.h"

extern bool Wifi_reconnect();
extern void InitWiFi();
extern bool attemptConnect(String ssid, String pass, bool forceDisconnect);

extern void clearWiFiSettings();
extern void saveWiFi(String ssid, String pass);
extern String getSSID();
extern String getPassword();

extern bool isWiFiConnected();
#endif /* INC_TASKWIFI_H_ */