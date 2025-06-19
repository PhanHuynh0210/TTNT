#include "TaskWebServer.h"

// Create a web server object
WebServer server(80);


void handleRoot() {
  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Failed to open HTML file");
    return;
  }

  String html = file.readString();
  file.close();
  server.send(200, "text/html", html);
}

void initWebServer() {

  // Set up the web server to handle different routes
  server.on("/", handleRoot);
  server.on("/A/on", handleGPIOAOn);
  server.on("/A/off", handleGPIOAOff);
  // server.on("/B/on", handleGPIOBOn);
  // server.on("/B/off", handleGPIOBOff);
  server.on("/C/on", handleGPIOCOn);
  server.on("/C/off", handleGPIOCOff);

  server.on("/api/leds", []() {
  String json = "{";
  json += "\"A\":\"" + outputAState + "\",";
  // json += "\"B\":\"" + outputBState + "\",";
  json += "\"C\":\"" + outputRGBState + "\"";
  json += "}";
  server.send(200, "application/json", json);
});


  server.on("/api/sensors", []() {
    String json = "{";
    json += "\"Temp\":" + String(dht20.getTemperature()) + ",";
    json += "\"Humid\":" + String(dht20.getHumidity()) + ",";
    json += "\"Soli\":" + String(getHumid()) + ",";
    json += "\"Distance\":" + String(test()) + ",";
    json += "\"Lux\":" + String(getValueLux()) ;
    json += "}";
    server.send(200, "application/json", json);
  });

//   server.on("/fan", []() {
//   if (server.hasArg("speed")) {
//     int speed = server.arg("speed").toInt();
//     setFanSpeed(speed);
//   } 
// });

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loopWebServer() {
  server.handleClient();
}