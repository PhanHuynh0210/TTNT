
#include "Webserver.h"

const int output18 = 18;
const int output21 = 21;
String output18State = "off";
String output21State = "off";

// Create a web server object
WebServer server(80);

// Function to handle turning GPIO 18 on
void handleGPIO18On() {
  output18State = "on";
  digitalWrite(output18, HIGH);
  handleRoot();
  String data = "on_pin_18";
  publishData("pin", data);
}

// Function to handle turning GPIO 18 off
void handleGPIO18Off() {
  output18State = "off";
  digitalWrite(output18, LOW);
  handleRoot();
  String data = "off_pin_18";
  publishData("pin", data);
}

// Function to handle turning GPIO 21 on
void handleGPIO21On() {
  output21State = "on";
  digitalWrite(output21, HIGH);
  handleRoot();
  String data = "on_pin_21";
  publishData("pin", data);
}
void setFanSpeed(int speedPercent) {
  // Giới hạn giá trị từ 0 - 100
    analogWrite(1,speedPercent);

}




// Function to handle turning GPIO 21 off
void handleGPIO21Off() {
  output21State = "off_pin_21";
  digitalWrite(output21, LOW);
  handleRoot();
  String data = "off";
  publishData("pin", data);
}

// Function to handle the root URL and show the current states
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<link rel=\"icon\" href=\"data:,\">";
  html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  html += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
  html += ".button2 { background-color: #555555; }</style></head>";
  html += "<body><h1>ESP32 Web Server</h1>";

  html += "<p>Set Fan Speed (0-100):</p>";
  html += "<form action=\"/fan\"><input type=\"number\" name=\"speed\" min=\"0\" max=\"100\">";
  html += "<input type=\"submit\" value=\"Set\"></form>";


  // Display GPIO 18 controls
  html += "<p>GPIO 18 - State " + output18State + "</p>";
  if (output18State == "off") {
    html += "<p><a href=\"/18/on\"><button class=\"button\">ON</button></a></p>";
  } else {
    html += "<p><a href=\"/18/off\"><button class=\"button button2\">OFF</button></a></p>";
  }

  // Display GPIO 21 controls
  html += "<p>GPIO 21 - State " + output21State + "</p>";
  if (output21State == "off") {
    html += "<p><a href=\"/21/on\"><button class=\"button\">ON</button></a></p>";
  } else {
    html += "<p><a href=\"/21/off\"><button class=\"button button2\">OFF</button></a></p>";
  }

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void initWebServer() {

  // Initialize the output variables as outputs
  pinMode(output18, OUTPUT);
  pinMode(output21, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output18, LOW);
  digitalWrite(output21, LOW);

  // Connect to Wi-Fi network
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/18/on", handleGPIO18On);
  server.on("/18/off", handleGPIO18Off);
  server.on("/21/on", handleGPIO21On);
  server.on("/21/off", handleGPIO21Off);

  server.begin();
  Serial.println("HTTP server started");
  server.on("/fan", []() {
  if (server.hasArg("speed")) {
    int speed = server.arg("speed").toInt();
    setFanSpeed(speed);
    server.sendHeader("Location", "/http://172.28.182.191/");
  } 
});
}

void web() {
  server.handleClient();
}