#ifndef TASKSENDMAI_H
#define TASKSENDMAI_H

#include "globals.h"

extern void sendMail(String content, String recipientEmail, String emailKey);
extern bool isMailConnected();
#define NAME_DEVICE "ESP32-Yolo"


#endif // TASKSENDMAI_H