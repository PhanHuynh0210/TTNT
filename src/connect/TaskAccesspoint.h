#ifndef TASKACCESSPOINT_H
#define TASKACCESSPOINT_H

#include "globals.h"
#include "TaskMQTT.h"

extern void initAP();
extern void accpoint();
extern void clearAllSettings();

// Thêm các function mới
extern bool checkBootButton();
extern void initAPConditional();
extern bool isAPMode();
extern void stopAP();  // Thêm function tắt AP
extern unsigned long getAPRemainingTime(); // Thêm function lấy thời gian còn lại

// Functions cho Account Settings
extern void saveAccountSettings(String username, String password);
extern String getCurrentAccountUsername();
extern String getCurrentAccountPassword();
extern void clearAccountSettings();

#endif // TASKACCESSPOINT_H