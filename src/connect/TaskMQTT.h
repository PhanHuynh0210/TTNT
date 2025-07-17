#ifndef INC_TASKMQTT_H_
#define INC_TASKMQTT_H_

#include "globals.h"

void initMQTT();
void reconnectMQTT();
void publishData(String feed, String data);
void loadMQTTSettings();
void saveMQTTSettings(String username, String key);
void clearMQTTSettings();
String getCurrentMQTTUsername();
bool isMQTTConnected();

#endif /* INC_TASKMQTT_H_ */