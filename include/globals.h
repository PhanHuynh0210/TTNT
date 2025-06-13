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

///
#include "../src/connect/TaskWifi.h"
#include "../src/connect/TaskMQTT.h"
#include "../src/device/taskkc.h"
#include "../src/device/TaskDHT20.h"
#include "../src/connect/Webserver.h"
#include "../src/device/TaskFan.h"

#endif