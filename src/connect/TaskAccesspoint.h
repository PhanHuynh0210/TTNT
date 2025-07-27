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

#endif // TASKACCESSPOINT_H