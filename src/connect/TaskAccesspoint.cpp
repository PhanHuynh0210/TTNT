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
            if (header.indexOf("GET /wifi?ssid=") >= 0) {
              int ssidIndex = header.indexOf("ssid=") + 5;
              int passIndex = header.indexOf("&pass=");
              int httpIndex = header.indexOf("HTTP");
              String ssid = header.substring(ssidIndex, passIndex);
              String pass = header.substring(passIndex + 6, httpIndex - 1);

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
                  client.println("<h1>WiFi Connected! Redirecting...</h1>");
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
                
                // Send error response
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();
                client.println("<!DOCTYPE html><html><body>");
                client.println("<h1>WiFi Connection Failed!</h1>");
                client.println("<p>Please check your credentials and try again.</p>");
                client.println("<a href='/'>Go Back</a>");
                client.println("</body></html>");
                client.println();
                break;
              }
            }
            else if (header.indexOf("GET /mqtt?server=") >= 0) {
              int serverIndex = header.indexOf("server=") + 7;
              int usernameIndex = header.indexOf("&username=") + 10;
              int keyIndex = header.indexOf("&key=");
              int httpIndex = header.indexOf("HTTP");
              
              String server = header.substring(serverIndex, usernameIndex - 10);
              String username = header.substring(usernameIndex, keyIndex);
              String key = header.substring(keyIndex + 5, httpIndex - 1);
              
              server.replace("%20", " ");
              server.replace("%3A", ":");
              username.replace("%20", " ");
              key.replace("%20", " ");

              Serial.println("Received MQTT settings:");
              Serial.println("Server: " + server);
              Serial.println("Username: " + username);
              Serial.println(String("Key: ") + (key.length() > 0 ? "****" : "empty"));

              saveMQTTSettings(server, username, key);
              
              // Send success response
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
              client.println("<!DOCTYPE html><html><body>");
              client.println("<h1>MQTT Settings Saved!</h1>");
              client.println("<p>MQTT configuration has been updated successfully.</p>");
              client.println("<a href='/'>Go Back</a>");
              client.println("</body></html>");
              client.println();
              break;
            }
            
            // Default page - show both forms
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
            client.println("<title>ESP32 Configuration</title>");
            client.println("<style>");
            client.println("body{font-family:Arial,sans-serif;background:#f5f5f5;padding:20px;}");
            client.println(".container{max-width:600px;margin:auto;}");
            client.println(".card{background:#fff;padding:20px;border-radius:8px;margin-bottom:20px;box-shadow:0 0 10px rgba(0,0,0,0.1);}");
            client.println("h1{text-align:center;color:#007cba;margin-bottom:30px;}");
            client.println("h2{color:#007cba;margin-bottom:15px;border-bottom:2px solid #007cba;padding-bottom:5px;}");
            client.println("input,button{width:100%;padding:12px;margin:8px 0;box-sizing:border-box;border-radius:5px;border:1px solid #ccc;}");
            client.println("button{background:#007cba;color:white;border:none;cursor:pointer;}");
            client.println("button:hover{background:#005a87;}");
            client.println(".note{font-size:12px;color:#666;margin-top:5px;}");
            client.println(".success{background:#d4edda;color:#155724;padding:10px;border-radius:5px;margin-bottom:15px;}");
            client.println(".mqtt-info{background:#e7f3ff;padding:15px;border-radius:5px;margin-bottom:15px;border-left:4px solid #007cba;}");
            client.println(".highlight{background:#fff3cd;border:1px solid #ffeaa7;padding:10px;border-radius:5px;margin-bottom:15px;}");
            client.println("</style></head><body>");
            client.println("<div class='container'>");
            client.println("<h1>ESP32 Configuration</h1>");
            
            
            client.println("<div class='card'>");
            client.println("<h2>WiFi Settings</h2>");
            client.println("<form action='/wifi'>");
            client.println("<input name='ssid' placeholder='WiFi SSID' required>");
            client.println("<input name='pass' type='password' placeholder='WiFi Password'>");
            client.println("<button type='submit'>Connect WiFi</button>");
            client.println("</form>");
            client.println("</div>");
            
            String currentServer = getCurrentMQTTServer();
            String currentUsername = getCurrentMQTTUsername();
            client.println("<div class='card'>");
            client.println("<h2>MQTT Settings</h2>");
            client.println("<div class='mqtt-info'>");
            client.println("<strong>Cấu hình hiện tại:</strong><br>");
            client.println("Server: " + currentServer + "<br>");
            client.println("Username: " + (currentUsername.length() > 0 ? currentUsername : "Chưa cấu hình"));
            client.println("</div>");
            

            client.println("<form action='/mqtt'>");
            client.println("<input name='server' placeholder='MQTT Server (vd: io.adafruit.com)' value='io.adafruit.com' required>");
            client.println("<div class='note'>Mặc định: io.adafruit.com (port 1883)</div>");
            client.println("<input name='username' placeholder='MQTT Username' required>");
            client.println("<input name='key' type='password' placeholder='MQTT Key/Password' required>");
            client.println("<button type='submit'>Save MQTT Settings</button>");
            client.println("</form>");
            client.println("</div>");
            
            client.println("</div></body></html>");
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

void clearAllSettings() {
  Serial.println("Clearing all settings (WiFi & MQTT)...");
  clearWiFiSettings();
  clearMQTTSettings();
  Serial.println("All settings cleared successfully!");
}
