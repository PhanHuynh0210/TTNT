#include "TaskWebServer.h"

AsyncWebServer server(80);



// void handleRoot() {
//   File file = LittleFS.open("/index.html", "r");
//   if (!file) {
//     server.send(500, "text/plain", "Failed to open HTML file");
//     return;
//   }

//   String html = file.readString();
//   file.close();
//   server.send(200, "text/html", html);
// }

// void initWebServer() {

//   // Set up the web server to handle different routes
//   server.on("/", handleRoot);
//   server.on("/A/on", handleGPIOAOn);
//   server.on("/A/off", handleGPIOAOff);
//   server.on("/C/on", handleGPIOCOn);
//   server.on("/C/off", handleGPIOCOff);

//   server.on("/api/leds", []() {
//   String json = "{";
//   json += "\"A\":\"" + outputAState + "\",";
//   json += "\"C\":\"" + outputRGBState + "\"";
//   json += "}";
//   server.send(200, "application/json", json);
// });


//   server.on("/api/sensors", []() {
//     String json = "{";
//     json += "\"Temp\":" + String(dht20.getTemperature()) + ",";
//     json += "\"Humid\":" + String(dht20.getHumidity()) + ",";
//     json += "\"Soli\":" + String(getHumid()) + ",";
//     json += "\"Distance\":" + String(test()) + ",";
//     json += "\"Lux\":" + String(getValueLux()) ;
//     json += "}";
//     server.send(200, "application/json", json);
//   });

// server.on("/api/rgbMode", []() {
//   if (server.hasArg("mode")) {
//     String mode = server.arg("mode");
//     if (mode == "auto" || mode == "rainbow" || mode == "static") {
//       rgbMode = mode;
//       server.send(200, "application/json", "{\"status\":\"ok\",\"mode\":\"" + mode + "\"}");
//     } else {
//       server.send(400, "application/json", "{\"error\":\"Invalid mode\"}");
//     }
//   } else {
//     server.send(400, "application/json", "{\"error\":\"Missing 'mode' parameter\"}");
//   }
// });

// server.on("/api/setColor", []() {
//   if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
//     customRed = server.arg("r").toInt();
//     customGreen = server.arg("g").toInt();
//     customBlue = server.arg("b").toInt();
//     server.send(200, "application/json", 
//       "{\"status\":\"ok\",\"r\":" + String(customRed) +
//       ",\"g\":" + String(customGreen) +
//       ",\"b\":" + String(customBlue) + "}");
//   } else {
//     server.send(400, "application/json", "{\"error\":\"Missing r, g, or b parameter\"}");
//   }
// });


//   // Start the web server
//   server.begin();
//   Serial.println("HTTP server started");
// }

// void loopWebServer() {
//   server.handleClient();
// }


AsyncWebSocket ws("/ws");

void notifyAllClients() {
  String json = "{";
  json += "\"type\":\"sensor\",";
  json += "\"Temp\":" + String(dht20.getTemperature()) + ",";
  json += "\"Humid\":" + String(dht20.getHumidity()) + ",";
  json += "\"Soli\":" + String(getHumid()) + ",";
  json += "\"Distance\":" + String(test()) + ",";
  json += "\"Lux\":" + String(getValueLux()) + ",";
  json += "\"A\":\"" + outputAState + "\",";
  json += "\"C\":\"" + outputRGBState + "\"";
  json += "}";
  ws.textAll(json);
}

void handleWebSocketMessage(AsyncWebSocketClient *client, String msg) {
  if (msg.startsWith("toggleLED:")) {
    char led = msg.charAt(10);
    String state = msg.substring(12);
    if (led == 'A') {
      digitalWrite(outputA, state == "on" ? HIGH : LOW);
      outputAState = state;
    } else if (led == 'C') {
      digitalWrite(PIN_NEO_PIXEL, state == "on" ? LOW : HIGH);
      outputRGBState = state;
    }
  } else if (msg.startsWith("rgbMode:")) {
    rgbMode = msg.substring(8);
  } else if (msg.startsWith("setColor:")) {
    int r, g, b;
    sscanf(msg.c_str(), "setColor:%d,%d,%d", &r, &g, &b);
    customRed = r;
    customGreen = g;
    customBlue = b;
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Client connected");
    client->text("connected");
  } else if (type == WS_EVT_DATA) {
    String msg = "";
    for (size_t i = 0; i < len; i++) msg += (char)data[i];
    handleWebSocketMessage(client, msg);
  }
}

void initWebServer() {
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  
  ElegantOTA.begin(&server);    
  
  server.begin();
  Serial.println("HTTP + WS server started");
  Serial.println("OTA ready! Open http://<IP>/update to access OTA update page");
}

void loopWebServer() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 2000) {
    notifyAllClients(); 
    lastTime = millis();
  }
  
  // Handle ElegantOTA
  ElegantOTA.loop();
}