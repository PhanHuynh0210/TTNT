#ifndef TASK_ACCESSPOINT_UI_H
#define TASK_ACCESSPOINT_UI_H

#include "globals.h"
#include "TaskAccesspoint.h"

// Rendering
void renderAPIndexPage(WiFiClient &client);

// WiFi endpoints
void handleAPScan(WiFiClient &client);
void handleAPWifiConnect(WiFiClient &client, const String &ssid, const String &pass);

// Account endpoints
void handleAPAccountSave(WiFiClient &client, const String &username, const String &password);

#endif // TASK_ACCESSPOINT_UI_H

