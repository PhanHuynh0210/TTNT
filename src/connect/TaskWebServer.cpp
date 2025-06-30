#include "TaskWebServer.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String createStatusJSON() {
  String json = "{";
  json += "\"type\":\"sensor\",";
  json += "\"Temp\":" + String(dht20.getTemperature()) + ",";
  json += "\"Humid\":" + String(dht20.getHumidity()) + ",";
  json += "\"Soli\":" + String(getHumid()) + ",";
  json += "\"Distance\":" + String(test()) + ",";
  json += "\"Lux\":" + String(getValueLux()) + ",";
  json += "\"A\":\"" + singleLedState + "\",";
  json += "\"C\":\"" + LedRGB + "\",";
  json += "\"rgbMode\":\"" + rgbMode + "\",";
  json += "\"customR\":" + String(customRed) + ",";
  json += "\"customG\":" + String(customGreen) + ",";
  json += "\"customB\":" + String(customBlue);
  json += "}";
  return json;
}

void notifyAllClients() {
  ws.textAll(createStatusJSON());
}

void handleLEDAControl(String msg) {
  if (msg.startsWith("toggleLED:")) {
    char led = msg.charAt(10);
    String state = msg.substring(12);
    if (led == 'A') {
      setSingleLedState(state);
      notifyAllClients();
    }
  }
}

void handleLEDCControl(String msg) {
  if (msg.startsWith("toggleLED:")) {
    char led = msg.charAt(10);
    String state = msg.substring(12);
    if (led == 'C') {
      digitalWrite(PIN_NEO_PIXEL, state == "on" ? LOW : HIGH);
      LedRGB = state;
      notifyAllClients();
    }
  } else if (msg.startsWith("rgbMode:")) {
    String mode = msg.substring(8);
    setRGBMode(mode);
    notifyAllClients();
  } else if (msg.startsWith("setColor:")) {
    int r, g, b;
    sscanf(msg.c_str(), "setColor:%d,%d,%d", &r, &g, &b);
    setCustomColor(r, g, b);
    notifyAllClients();
  }
}

void handleWebSocketMessage(AsyncWebSocketClient *client, String msg) {
  if (msg.startsWith("toggleLED:")) {
    char led = msg.charAt(10);
    if (led == 'A') {
      handleLEDAControl(msg);
    } else if (led == 'C') {
      handleLEDCControl(msg);
    }
  } else if (msg.startsWith("rgbMode:") || msg.startsWith("setColor:")) {
    handleLEDCControl(msg);
  }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Client connected");
    client->text(createStatusJSON());
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
  
  ElegantOTA.loop();
}