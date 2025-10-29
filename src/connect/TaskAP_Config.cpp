#include "TaskAccesspointUI.h"

void handleAPMqttSave(WiFiClient &client, const String &server, const String &username, const String &key) {
  String serverClean = server;
  String usernameClean = username;
  String keyClean = key;

  saveMQTTSettings(serverClean, usernameClean, keyClean);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("SAVED");
}

void handleAPAccountSave(WiFiClient &client, const String &username, const String &password) {
  String usernameClean = username;
  String passwordClean = password;

  saveAccountSettings(usernameClean, passwordClean);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("SAVED");
}

