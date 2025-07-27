#ifndef GLOBALS_H
#define GLOBALS_H

#define delay_time 10000

// include libraries
#include <Wire.h>
#include <WiFi.h>
#include <DHT20.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal_I2C.h>
#include <FS.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <Preferences.h> 
#include <ArduinoJson.h> 
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <ESP_Mail_Client.h>
#include <HTTPUpdate.h>  




#include "../src/connect/TaskWifi.h"
#include "../src/connect/TaskMQTT.h"
#include "../src/connect/Task485.h"
#include "../src/device/taskkc.h"
#include "../src/device/TaskDHT20.h"
#include "../src/connect/TaskWebServer.h"
#include "../src/device/TaskFan.h"
#include "../src/device/TaskLED.h"
#include "../src/device/TaskRGB.h"
#include "../src/device/TaskLUX.h"
#include "../src/device/TaskLCD.h"
#include "../src/device/TaskRelay.h"
#include "../src/device/TaskHumid.h"
#include "../src/device/TaskES35.h"
#include "../src/device/Device.h"
#include "../src/device/TaskJXBS.h"
#include "../src/connect/TaskAccesspoint.h"
#include "../src/connect/TaskGGsheet.h"
#include "../src/task/TaskCnWifi.h"
#include "../src/device/TasksendMail.h"
#include "../src/device/TaskStatusRGB.h"
#include "../src/device/TaskSystemMonitor.h"
#include "../secrest.h"

#endif