#include "TaskAccesspointUI.h"

void handleAPScan(WiFiClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println();

  int n = WiFi.scanNetworks();
  client.print("[");
  for (int i = 0; i < n; ++i) {
    if (i > 0) client.print(",");
    client.print("{\"ssid\":\"");
    client.print(WiFi.SSID(i));
    client.print("\",\"rssi\":");
    client.print(WiFi.RSSI(i));
    client.print(",\"encryption\":");
    client.print(WiFi.encryptionType(i));
    client.print("}");
  }
  client.print("]");
}

void handleAPWifiConnect(WiFiClient &client, const String &ssid, const String &pass) {
  String ssidClean = ssid; // already decoded by caller
  String passClean = pass;

  delay(1000);

  WiFi.begin(ssidClean.c_str(), passClean.c_str());

  unsigned long startAttemptTime = millis();
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
    dotCount++;
    if (dotCount % 20 == 0) {
      Serial.println();
      Serial.println("WiFi Status: " + String(WiFi.status()));
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nSave, connected");
    saveWiFi(ssidClean, passClean);
    String staIP = WiFi.localIP().toString();
    Serial.println("IP Address: " + staIP);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/plain");
    client.println("Connection: close");
    client.println();
    client.println("SAVED");
    client.stop();
    return;
  }

  Serial.println("\n Failed to connect to new WiFi!");
  Serial.println("Final WiFi Status: " + String(WiFi.status()));
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println("Connection: close");
  client.println();
  client.println("FAILED");
  client.stop();

  setStatus(STATUS_ERROR);
  delay(2000);
  setStatus(STATUS_AP_MODE);
}

