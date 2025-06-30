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

#include "../src/connect/TaskWifi.h"
#include "../src/connect/TaskMQTT.h"
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
#endif