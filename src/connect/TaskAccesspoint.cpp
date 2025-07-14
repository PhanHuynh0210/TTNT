#include "TaskAccesspoint.h"

Preferences preferences;

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

WiFiServer apServer(90);
String header;
bool apMode = false;

void initAP() {
  Serial.println("Initializing Access Point...");
  
  preferences.begin("wifi-config", false);

  String savedSSID = preferences.getString("ssid", "");
  String savedPass = preferences.getString("pass", "");
  
  if (savedSSID.length() > 0) {
    Serial.println("Found saved WiFi credentials, trying to connect...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID.c_str(), savedPass.c_str());
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(100);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to saved WiFi!");
      Serial.println("IP: " + WiFi.localIP().toString());
      apMode = false;
      return;
    } else {
      Serial.println("\nFailed to connect to saved WiFi, starting AP mode...");
    }
  }
  
  // Nếu không kết nối được, chuyển sang AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  apServer.begin();
  apMode = true;
  Serial.println("Access Point started. Connect to ESP32_Config and go to 192.168.4.1");
}

void accpoint() {
  if (!apMode) return; // Chỉ chạy khi đang ở AP mode
  
  WiFiClient client = apServer.available();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    header = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Parse nếu có tham số GET ?ssid=...&pass=...
            if (header.indexOf("GET /?ssid=") >= 0) {
              int ssidIndex = header.indexOf("ssid=") + 5;
              int passIndex = header.indexOf("&pass=");
              String ssid = header.substring(ssidIndex, passIndex);
              String pass = header.substring(passIndex + 6, header.indexOf("HTTP") - 1);

              ssid.replace("%20", " "); // Nếu tên Wi-Fi có dấu cách

              Serial.println("SSID: " + ssid);
              Serial.println("PASS: " + pass);

              // Lưu vào Preferences
              preferences.putString("ssid", ssid);
              preferences.putString("pass", pass);

              // Thử kết nối
              WiFi.mode(WIFI_STA);
              WiFi.begin(ssid.c_str(), pass.c_str());
              Serial.println("Connecting...");

              unsigned long startAttemptTime = millis();
              while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
                delay(100);
                Serial.print(".");
              }

              if (WiFi.status() == WL_CONNECTED) {
                Serial.println("Connected! IP: " + WiFi.localIP().toString());
                apMode = false; // Tắt AP mode khi đã kết nối WiFi
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();
                client.println("<!DOCTYPE html><html><body>");
                client.println("<h1>WiFi Connected Successfully!</h1>");
                client.println("<p>IP Address: " + WiFi.localIP().toString() + "</p>");
                client.println("<p>Device will restart in AP+STA mode.</p>");
                client.println("</body></html>");
                client.println();
                break;
              } else {
                Serial.println("Failed to connect.");
                // Quay lại AP mode
                WiFi.mode(WIFI_AP);
                WiFi.softAP(ap_ssid, ap_password);
              }
            }

            // Gửi trang web form cấu hình
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='UTF-8'>");
            client.println("<style>body{font-family:Arial;margin:40px;} input{padding:10px;margin:5px;width:200px;} button{padding:10px 20px;background:#007cba;color:white;border:none;cursor:pointer;}</style>");
            client.println("</head><body>");
            client.println("<h1>ESP32 Wi-Fi Setup</h1>");
            client.println("<form action=\"/\">");
            client.println("SSID:<br><input type=\"text\" name=\"ssid\" placeholder=\"Enter WiFi Name\"><br>");
            client.println("Password:<br><input type=\"password\" name=\"pass\" placeholder=\"Enter WiFi Password\"><br><br>");
            client.println("<button type=\"submit\">Connect to WiFi</button>");
            client.println("</form>");
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
    Serial.println("Client disconnected.");
  }
}
