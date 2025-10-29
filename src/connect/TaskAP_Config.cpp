#include "TaskAccesspointUI.h"

void handleAPMqttSave(WiFiClient &client, const String &server, const String &username, const String &key) {
  String serverClean = server;
  String usernameClean = username;
  String keyClean = key;

  Serial.println("Received MQTT settings:");
  Serial.println("Server: " + serverClean);
  Serial.println("Username: " + usernameClean);
  Serial.println(String("Key: ") + keyClean);

  saveMQTTSettings(serverClean, usernameClean, keyClean);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("SAVED");
}

void handleAPAccountSave(WiFiClient &client, const String &username, const String &password) {
  String usernameClean = username;
  String passwordClean = password;

  Serial.println("Received Account settings:");
  Serial.println("Username: " + usernameClean);
  Serial.println(String("Password: ") + (passwordClean.length() > 0 ? "****" : "empty"));

  saveAccountSettings(usernameClean, passwordClean);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("SAVED");
}

