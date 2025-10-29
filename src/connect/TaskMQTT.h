#ifndef INC_TASKMQTT_H_
#define INC_TASKMQTT_H_

#include "globals.h"

extern PubSubClient client;
void initMQTT();
void reconnectMQTT();
void publishData(String feed, String data);
void loadMQTTSettings();
void saveMQTTSettings(String server, String username, String key);
void clearMQTTSettings();
String getCurrentMQTTUsername();
String getCurrentMQTTServer();
bool isMQTTConnected();

// Function xử lý xác thực
void handleAuthRequest(String message);

// Alert and Email functionality (moved from WebServer)
void initAlertSystem();
void checkAndSendAlerts(float temp, float humid, float soli, float distance, float lux);
bool compareThreshold(float value, String op, float threshold);
void sendAlertEmail(float temp, float humid, float soli, float distance, float lux, String reason);

#endif /* INC_TASKMQTT_H_ */