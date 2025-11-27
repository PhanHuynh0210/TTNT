#include "TaskAccesspointUI.h"

void handleAPAccountSave(WiFiClient &client, const String &username, const String &password) {
  String usernameClean = username;
  String passwordClean = password;

  saveAccountSettings(usernameClean, passwordClean);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("SAVED");
}

