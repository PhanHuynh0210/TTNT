#include "TaskAccesspoint.h"

Preferences preferences;

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

WiFiServer apServer(90);
String header;
bool apMode = false;

void initAP() {
  if (apMode && WiFi.getMode() == WIFI_AP) {
    return;
  }
  preferences.begin("wifi-config", false);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(ap_ssid, ap_password);
  
  if (apStarted) {
    Serial.println("Access Point started successfully!");
    Serial.print("AP SSID: ");
    Serial.println(ap_ssid);
    Serial.print("AP Password: ");
    Serial.println(ap_password);
    Serial.println("http://192.168.4.1:90");
    apServer.begin();
    apMode = true;
  } else {
    Serial.println(" Failed to start Access Point!");
    apMode = false;
  }
}

void accpoint() {
  if (!apMode) return; 
 
  WiFiClient client = apServer.available();

  if (client) {
    Serial.println("New Client connected to AP.");
    String currentLine = "";
    header = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (header.indexOf("GET /?ssid=") >= 0) {
              int ssidIndex = header.indexOf("ssid=") + 5;
              int passIndex = header.indexOf("&pass=");
              String ssid = header.substring(ssidIndex, passIndex);
              String pass = header.substring(passIndex + 6, header.indexOf("HTTP") - 1);

              ssid.replace("%20", " ");

              Serial.println("Received WiFi credentials:");
              Serial.println("SSID: " + ssid);
              Serial.println("PASS: " + pass);

              preferences.putString("ssid", ssid);
              preferences.putString("pass", pass);
              Serial.println("WiFi credentials saved to flash memory.");

              WiFi.disconnect(true); 
              delay(1000);
              
              WiFi.mode(WIFI_STA);
              WiFi.begin(ssid.c_str(), pass.c_str());
              Serial.println("Attempting to connect to new WiFi...");
              Serial.println("SSID: " + ssid);

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
                  Serial.println("\n Successfully connected to new WiFi!");
                  String staIP = WiFi.localIP().toString();
                  Serial.println("IP Address: " + staIP);

                  client.println("HTTP/1.1 302 Found");
                  client.print("Location: http://");
                  client.print(staIP);
                  client.println("/"); 
                  client.println("Connection: close");
                  client.println();
                  client.println("<!DOCTYPE html><html><body>");
                  client.println("<h1>Redirecting...</h1>");
                  client.println("</body></html>");
                  client.flush();
                  delay(1000);

                  apMode = false;
                  break;
              } else {
                Serial.println("\n Failed to connect to new WiFi!");
                Serial.println("Final WiFi Status: " + String(WiFi.status()));
                WiFi.disconnect(true);
                delay(1000);
                WiFi.mode(WIFI_AP);
                WiFi.softAP(ap_ssid, ap_password);
                Serial.println("Switched back to AP mode");
                break;
              }
            }
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
            client.println("<title>ESP32 WiFi Setup</title>");
            client.println("<style>");
            client.println("body{font-family:Arial,sans-serif;background:#f5f5f5;padding:20px;}");
            client.println(".box{background:#fff;padding:20px;border-radius:8px;max-width:400px;margin:auto;box-shadow:0 0 10px rgba(0,0,0,0.1);}");
            client.println("h2{text-align:center;color:#007cba;}");
            client.println("input,button{width:100%;padding:12px;margin:8px 0;box-sizing:border-box;border-radius:5px;border:1px solid #ccc;}");
            client.println("button{background:#007cba;color:white;border:none;cursor:pointer;}");
            client.println("button:hover{background:#005a87;}");
            client.println("</style></head><body>");
            client.println("<div class='box'>");
            client.println("<h2>ESP32 WiFi Setup</h2>");
            client.println("<form action='/'>");
            client.println("<input name='ssid' placeholder='WiFi SSID' required>");
            client.println("<input name='pass' type='password' placeholder='WiFi Password'>");
            client.println("<button type='submit'>Connect</button>");
            client.println("</form></div>");
            client.println("</body></html>");
            client.println();

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected from AP.");
  }
}

void forceAPMode() {
  Serial.println("Force switching to Access Point mode...");
  clearWiFiSettings();
  WiFi.disconnect(true);
  delay(1000);
  apMode = false;
  initAP();
  Serial.println("Forced switch to AP mode completed");
}
