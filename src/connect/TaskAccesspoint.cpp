#include "TaskAccesspoint.h"

Preferences preferences;

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

WiFiServer apServer(90);
String header;
bool apMode = false;

void initAP() {
  
  preferences.begin("wifi-config", false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  apServer.begin();
  apMode = true;
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
            // Parse nếu có tham số GET ?ssid=...&pass=...
            if (header.indexOf("GET /?ssid=") >= 0) {
              int ssidIndex = header.indexOf("ssid=") + 5;
              int passIndex = header.indexOf("&pass=");
              String ssid = header.substring(ssidIndex, passIndex);
              String pass = header.substring(passIndex + 6, header.indexOf("HTTP") - 1);

              ssid.replace("%20", " "); // Nếu tên Wi-Fi có dấu cách

              Serial.println("Received WiFi credentials:");
              Serial.println("SSID: " + ssid);
              Serial.println("PASS: " + pass);

              // Lưu vào Preferences
              preferences.putString("ssid", ssid);
              preferences.putString("pass", pass);
              Serial.println("WiFi credentials saved to flash memory.");

              // Thử kết nối với WiFi mới
              WiFi.mode(WIFI_STA);
              WiFi.begin(ssid.c_str(), pass.c_str());
              Serial.println("Attempting to connect to new WiFi...");

              unsigned long startAttemptTime = millis();
              while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
                delay(100);
                Serial.print(".");
              }

              if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\nSuccessfully connected to new WiFi!");
                Serial.println("IP Address: " + WiFi.localIP().toString());
                apMode = false; // Tắt AP mode khi đã kết nối WiFi
                
                // Gửi response thành công
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();
                client.println("<!DOCTYPE html><html>");
                client.println("<head><meta charset='UTF-8'>");
                client.println("<style>body{font-family:Arial;margin:40px;color:green;} h1{color:#007cba;}</style>");
                client.println("</head><body>");
                client.println("<h1>✅ WiFi Connected Successfully!</h1>");
                client.println("<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>");
                client.println("<p>Device is now connected to your WiFi network.</p>");
                client.println("<p>You can disconnect from the ESP32_Config hotspot.</p>");
                client.println("</body></html>");
                client.println();
                break;
              } else {
                Serial.println("\nFailed to connect to new WiFi!");
                // Quay lại AP mode nếu kết nối thất bại
                WiFi.mode(WIFI_AP);
                WiFi.softAP(ap_ssid, ap_password);
                
                // Gửi response lỗi
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();
                client.println("<!DOCTYPE html><html>");
                client.println("<head><meta charset='UTF-8'>");
                client.println("<style>body{font-family:Arial;margin:40px;color:red;} h1{color:#d32f2f;}</style>");
                client.println("</head><body>");
                client.println("<h1>❌ WiFi Connection Failed!</h1>");
                client.println("<p>Could not connect to the WiFi network.</p>");
                client.println("<p>Please check your credentials and try again.</p>");
                client.println("<a href='/'>Go Back</a>");
                client.println("</body></html>");
                client.println();
                break;
              }
            }

            // Gửi trang web form cấu hình
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='UTF-8'>");
            client.println("<title>ESP32 WiFi Setup</title>");
            client.println("<style>");
            client.println("body{font-family:Arial,sans-serif;margin:40px;background:#f5f5f5;}");
            client.println(".container{background:white;padding:30px;border-radius:10px;box-shadow:0 2px 10px rgba(0,0,0,0.1);max-width:400px;margin:0 auto;}");
            client.println("h1{color:#007cba;text-align:center;margin-bottom:30px;}");
            client.println("input{padding:12px;margin:8px 0;width:100%;border:2px solid #ddd;border-radius:5px;box-sizing:border-box;}");
            client.println("button{padding:15px;width:100%;background:#007cba;color:white;border:none;border-radius:5px;cursor:pointer;font-size:16px;margin-top:10px;}");
            client.println("button:hover{background:#005a87;}");
            client.println(".info{background:#e3f2fd;padding:15px;border-radius:5px;margin-bottom:20px;border-left:4px solid #007cba;}");
            client.println("</style>");
            client.println("</head><body>");
            client.println("<div class='container'>");
            client.println("<h1>🛜 ESP32 WiFi Setup</h1>");
            client.println("<div class='info'>");
            client.println("<strong>Instructions:</strong><br>");
            client.println("1. Enter your WiFi network name (SSID)<br>");
            client.println("2. Enter your WiFi password<br>");
            client.println("3. Click Connect to save and connect");
            client.println("</div>");
            client.println("<form action='/'>");
            client.println("<label>WiFi Network Name (SSID):</label>");
            client.println("<input type='text' name='ssid' placeholder='Enter WiFi Name' required>");
            client.println("<label>WiFi Password:</label>");
            client.println("<input type='password' name='pass' placeholder='Enter WiFi Password'>");
            client.println("<button type='submit'>🔗 Connect to WiFi</button>");
            client.println("</form>");
            client.println("</div>");
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
