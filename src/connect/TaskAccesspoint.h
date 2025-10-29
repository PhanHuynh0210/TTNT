#ifndef TASKACCESSPOINT_H
#define TASKACCESSPOINT_H

#include "globals.h"
#include "TaskMQTT.h"

extern void initAP();
extern void accpoint();
extern void clearAllSettings();

extern bool checkBootButtonNonBlocking();
extern bool isAPMode();
extern void stopAP(); 
extern unsigned long getAPRemainingTime(); 

extern void saveAccountSettings(String username, String password);
extern String getCurrentAccountUsername();
extern String getCurrentAccountPassword();
extern void clearAccountSettings();

// Function để vào AP mode từ web interface
extern void enterAPMode();

// Email settings functions
extern void saveEmailSettings(String email);
extern String getCurrentEmail();
extern void clearEmailSettings();

#endif // TASKACCESSPOINT_H