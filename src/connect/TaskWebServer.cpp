#include "TaskWebServer.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Preferences SensorPrefs;


float lastTemp = -999, lastHumid = -999, lastSoli = -999, lastDistance = -999, lastLux = -999;
String lastLedA = "";
String lastLedC = "";
String lastRGBMode = "";
int lastCustomR = -1, lastCustomG = -1, lastCustomB = -1;

String email = "";
float thresholdTemp = 30.0;
float thresholdHumid = 80.0;
float thresholdLux = 400.0;
float thresholdSoli = 300.0;
float thresholdDistance = 50.0;

unsigned long lastMailTime = 0;
const unsigned long MAIL_INTERVAL = 60000; 

String createStatusJSON() {
  JsonDocument doc;
  

  float temp = round(dht20.getTemperature() * 10) / 10.0;
  float humid = round(dht20.getHumidity() * 10) / 10.0;
  float soli = getHumid();
  float distance = test();
  float lux = getValueLux();

  doc["type"] = "sensor";
  doc["Temp"] = temp;
  doc["Humid"] = humid;
  doc["Soli"] = soli;
  doc["Distance"] = distance;
  doc["Lux"] = lux;

  doc["A"] = singleLedState;
  doc["C"] = LedRGB;
  doc["rgbMode"] = rgbMode;
  doc["customR"] = customRed;
  doc["customG"] = customGreen;
  doc["customB"] = customBlue;

  String json;
  serializeJson(doc, json);
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
      ws.textAll("{\"A\":\"" + singleLedState + "\"}");
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
      ws.textAll("{\"C\":\"" + LedRGB + "\"}");
    }
  } else if (msg.startsWith("rgbMode:")) {
    String mode = msg.substring(8);
    setRGBMode(mode);
    ws.textAll("{\"rgbMode\":\"" + mode + "\"}");
  } else if (msg.startsWith("setColor:")) {
    int r, g, b;
    sscanf(msg.c_str(), "setColor:%d,%d,%d", &r, &g, &b);
    setCustomColor(r, g, b);
    JsonDocument doc;
    doc["customR"] = r;
    doc["customG"] = g; 
    doc["customB"] = b;
    String json;
    serializeJson(doc, json);
    ws.textAll(json);
  } else if (msg.startsWith("setThreshold:")) {
    String jsonData = msg.substring(13);
    JsonDocument doc;
    Serial.println(jsonData);
    DeserializationError error = deserializeJson(doc, jsonData);
    if (!error) {
    email = doc["email"].as<String>();
    thresholdTemp = doc["thresholdTemp"].as<float>();
    thresholdHumid = doc["thresholdHumid"].as<float>();
    thresholdLux = doc["thresholdLux"].as<float>();
    thresholdSoli = doc["thresholdSoli"].as<float>();
    thresholdDistance = doc["thresholdDistance"].as<float>();

    // Lưu vào Preferences
    SensorPrefs.begin("sensor-config", false);
    SensorPrefs.putString("email", email);
    SensorPrefs.putFloat("temp", thresholdTemp);
    SensorPrefs.putFloat("humid", thresholdHumid);
    SensorPrefs.putFloat("lux", thresholdLux);
    SensorPrefs.putFloat("soli", thresholdSoli);
    SensorPrefs.putFloat("distance", thresholdDistance);
    SensorPrefs.end();

      // Phản hồi client
      JsonDocument res;
      res["action"] = "configSaved";
      String response;
      serializeJson(res, response);
      ws.textAll(response);
    }
  }
}
void handleAPModeSwitch(AsyncWebSocketClient *client) {
  Serial.println("Received request to switch to AP mode");
  JsonDocument doc;
  doc["action"] = "switchToAP";
  doc["apUrl"] = "http://192.168.4.1:90";
  doc["message"] = "Switching to Access Point mode...";
  
  String response;
  serializeJson(doc, response);
  client->text(response);
  delay(500);
  
}

void handleWebSocketMessage(AsyncWebSocketClient *client, String msg) {
  Serial.println("[DEBUG] handleWebSocketMessage called with msg: " + msg);
  if (msg.startsWith("toggleLED:")) {
    char led = msg.charAt(10);
    if (led == 'A') {
      handleLEDAControl(msg);
    } else if (led == 'C') {
      handleLEDCControl(msg);
    }
  } else if (msg.startsWith("rgbMode:") || msg.startsWith("setColor:") || msg.startsWith("setThreshold:")) {
    handleLEDCControl(msg);
  } else if (msg == "switchToAP") {
    handleAPModeSwitch(client);
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
if (!LittleFS.begin(true)) { 
  Serial.println("LittleFS Mount Failed, even after formatting!");
  return;
}

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  ElegantOTA.begin(&server);
  server.begin();

  SensorPrefs.begin("sensor-config", true);
  email = SensorPrefs.getString("email", "");
  thresholdTemp = SensorPrefs.getFloat("temp", 30.0);
  thresholdHumid = SensorPrefs.getFloat("humid", 80.0);
  thresholdLux = SensorPrefs.getFloat("lux", 400.0);
  thresholdSoli = SensorPrefs.getFloat("soli", 300.0);
  thresholdDistance = SensorPrefs.getFloat("distance", 50.0);
  SensorPrefs.end();

  Serial.println("== Giá trị đã lưu trong NVS ==");
  Serial.println("Email: " + email);
  Serial.printf("Ngưỡng nhiệt độ: %.2f°C\n", thresholdTemp);
  Serial.printf("Ngưỡng độ ẩm: %.2f%%\n", thresholdHumid);
  Serial.printf("Ngưỡng ánh sáng: %.2f lux\n", thresholdLux);
  Serial.printf("Ngưỡng độ ẩm đất: %.2f\n", thresholdSoli);
  Serial.printf("Ngưỡng khoảng cách: %.2f cm\n", thresholdDistance);

  String staIP = WiFi.localIP().toString();
  Serial.println("IP Address: " + staIP);
  Serial.println("HTTP + WS server started");
}

void loopWebServer() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 2000) {
    float temp = round(dht20.getTemperature() * 10) / 10.0;
    float humid = round(dht20.getHumidity() * 10) / 10.0;
    float soli = getHumid();
    float distance = test();
    float lux = getValueLux();

    bool changed = false;

    if (abs(temp - lastTemp) > 0.2) { lastTemp = temp; changed = true; }
    if (abs(humid - lastHumid) > 0.2) { lastHumid = humid; changed = true; }
    if (abs(soli - lastSoli) > 0.2) { lastSoli = soli; changed = true; }
    if (abs(distance - lastDistance) > 1) { lastDistance = distance; changed = true; }
    if (abs(lux - lastLux) > 1) { lastLux = lux; changed = true; }

    if (singleLedState != lastLedA) { lastLedA = singleLedState; changed = true; }
    if (LedRGB != lastLedC) { lastLedC = LedRGB; changed = true; }
    if (rgbMode != lastRGBMode) { lastRGBMode = rgbMode; changed = true; }
    if (customRed != lastCustomR || customGreen != lastCustomG || customBlue != lastCustomB) {
      lastCustomR = customRed; lastCustomG = customGreen; lastCustomB = customBlue;
      changed = true;
    }

    if (changed) {
      notifyAllClients();
      bool shouldAlert = false;
      String reason = "";

      if (temp > thresholdTemp) {
        shouldAlert = true;
        reason += "Nhiệt độ vượt ngưỡng (" + String(temp) + "°C > " + String(thresholdTemp) + "°C)<br>";
      }
      if (humid > thresholdHumid) {
        shouldAlert = true;
        reason += "Độ ẩm vượt ngưỡng (" + String(humid) + "% > " + String(thresholdHumid) + "%)<br>";
      }
      if (lux > thresholdLux) {
        shouldAlert = true;
        reason += "Ánh sáng vượt ngưỡng (" + String(lux) + " lux > " + String(thresholdLux) + " lux)<br>";
      }
      if (soli > thresholdSoli) {
        shouldAlert = true;
        reason += "Độ ẩm đất vượt ngưỡng (" + String(soli) + " > " + String(thresholdSoli) + ")<br>";
      }
      if (distance < thresholdDistance) {
        shouldAlert = true;
        reason += "Có vật cản gần (" + String(distance) + " cm < " + String(thresholdDistance) + " cm)<br>";
      }

      if (shouldAlert && email.length() > 5) {
        unsigned long now = millis();
        if (now - lastMailTime > MAIL_INTERVAL || lastMailTime == 0) {
          StaticJsonDocument<1024> doc;
          doc["device"] = NAME_DEVICE;
          doc["temp"] = temp;
          doc["humid"] = humid;
          doc["lux"] = lux;
          doc["soli"] = soli;
          doc["distance"] = distance;
          doc["message"] = reason;
          String alertContent;
          serializeJson(doc, alertContent);
          sendMail(alertContent);
          lastMailTime = now;
        }
      }

    }

    lastTime = millis();
  }

  ws.cleanupClients();
  ElegantOTA.loop();
}
