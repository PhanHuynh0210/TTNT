#ifndef TASKACCESSPOINT_H
#define TASKACCESSPOINT_H

#include "globals.h"
#include "TaskMQTT.h"

extern void initAP();
extern void accpoint();

extern bool checkBootButtonNonBlocking();
extern bool isAPMode();
extern void stopAP(); 
extern unsigned long getAPRemainingTime(); 

extern void saveAccountSettings(String username, String password);
extern String getCurrentAccountUsername();
extern String getCurrentAccountPassword();

#endif // TASKACCESSPOINT_H