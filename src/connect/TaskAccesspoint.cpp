#include "TaskAccesspoint.h"

WiFiServer apServer(90);
String header;

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

// Biến để theo dõi trạng thái AP
bool apMode = false;

// Function kiểm tra nút boot
bool checkBootButton() {
  const int BOOT_PIN = 0;  // Pin 0 là nút boot trên ESP32
  const unsigned long HOLD_TIME = 3000;  // 3 giây
  
  pinMode(BOOT_PIN, INPUT);
  
  Serial.println("Kiểm tra nút boot...");
  Serial.println("Nhấn giữ nút BOOT trong 3 giây để vào chế độ Access Point");
  
  unsigned long startTime = millis();
  bool buttonPressed = false;
  
  while (millis() - startTime < HOLD_TIME) {
    if (digitalRead(BOOT_PIN) == LOW) {  // Nút được nhấn (LOW vì có pull-up)
      buttonPressed = true;
      Serial.print(".");
      delay(100);
    } else {
      if (buttonPressed) {
        Serial.println("\nNút boot được thả ra sớm!");
        return false;
      }
    }
  }
  
  if (buttonPressed) {
    Serial.println("\nNút boot được nhấn giữ đủ 3 giây!");
    Serial.println("Vào chế độ Access Point...");
    return true;
  }
  
  Serial.println("\nNút boot không được nhấn.");
  return false;
}

// Function khởi tạo AP có điều kiện
void initAPConditional() {
  if (checkBootButton()) {
    apMode = true;
    initAP();
  } else {
    apMode = false;
    Serial.println("Không vào chế độ Access Point.");
  }
}

// Function kiểm tra có đang ở chế độ AP không
bool isAPMode() {
  return apMode;
}

// Function tắt Access Point
void stopAP() {
  if (apMode) {
    Serial.println("Tắt Access Point...");
    WiFi.softAPdisconnect(true);  // Tắt AP và xóa cấu hình
    apServer.stop();  // Dừng server
    apMode = false;
    Serial.println("Access Point đã được tắt.");
  }
}

void initAP() {
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point started successfully!");
  Serial.print("AP SSID: ");
  Serial.println(ap_ssid);
  Serial.print("AP Password: ");
  Serial.println(ap_password);
  Serial.println("http://192.168.4.1:90");
  apServer.begin();
}

void accpoint() {
  // Chỉ xử lý khi ở chế độ AP
  if (!apMode) {
    return;
  }
  
  WiFiClient client = apServer.available();

  if (client) {
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

              Serial.println("SSID: " + ssid);
              Serial.println("PASS: " + pass);

              Serial.println("WiFi save.");
              delay(1000);
              
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
                Serial.println("\nSave, connected");

                saveWiFi(ssid, pass);
                String staIP = WiFi.localIP().toString();
                Serial.println("IP Address: " + staIP);

                // Gửi phản hồi HTML thông báo kết nối thành công
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println();
                client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
                client.println("<title>WiFi Connected</title>");
                client.println("<style>");
                client.println("body{font-family:sans-serif;text-align:center;background:#f0f0f0;padding:50px;}");
                client.println(".card{display:inline-block;background:white;padding:30px;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1);}");
                client.println("h2{color:green;}");
                client.println("</style></head><body>");
                client.println("<div class='card'>");
                client.println("<h2> Kết nối WiFi thành công!</h2>");
                client.println("<p>Địa chỉ IP của ESP32:</p>");
                client.println("<p><strong>" + staIP + "</strong></p>");
                client.println("<p>Access Point sẽ tự động tắt sau 3 giây.</p>");
                client.println("</div>");
                client.println("</body></html>");
                client.println();

                client.stop();
                delay(3000);
                
                // Tắt Access Point thay vì restart
                stopAP();
                
                // Khởi tạo lại các task cần thiết sau khi có WiFi
                Serial.println("Khởi tạo lại các task sau khi có WiFi...");
                initConnect();
                Serial.println("Các task đã được khởi tạo lại thành công!");
                
                return;
              } else {
                Serial.println("\n Failed to connect to new WiFi!");
                Serial.println("Final WiFi Status: " + String(WiFi.status()));
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
              Serial.println(String("Key: ") + key);

              saveMQTTSettings(server, username, key);
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/plain");
              client.println();
              client.println("SAVED");

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
            

            client.println("<form id='mqttForm' onsubmit='submitMQTT(event)'>");
            client.println("<input id='mqttServer' placeholder='MQTT Server (vd: io.adafruit.com)' value='io.adafruit.com' required>");
            client.println("<div class='note'>Mặc định: io.adafruit.com (port 1883)</div>");
            client.println("<input id='mqttUsername' placeholder='MQTT Username' required>");
            client.println("<input id='mqttKey' type='password' placeholder='MQTT Key/Password' required>");
            client.println("<button type='submit'>Save MQTT Settings</button>");
            client.println("</form>");
            client.println("<div id='mqttMessage' class='success' style='display:none;'>Đã lưu cấu hình MQTT thành công!</div>");
            client.println("</div>");

            client.println("<script>");
            client.println("function submitMQTT(event) {");
            client.println("  event.preventDefault();");
            client.println("  const server = document.getElementById('mqttServer').value;");
            client.println("  const username = document.getElementById('mqttUsername').value;");
            client.println("  const key = document.getElementById('mqttKey').value;");
            client.println("  const url = `/mqtt?server=${encodeURIComponent(server)}&username=${encodeURIComponent(username)}&key=${encodeURIComponent(key)}`;");
            client.println("  fetch(url)");
            client.println("    .then(res => res.text())");
            client.println("    .then(text => {");
            client.println("      if (text.trim() === 'SAVED') {");
            client.println("        const msg = document.getElementById('mqttMessage');");
            client.println("        msg.style.display = 'block';");
            client.println("        setTimeout(() => msg.style.display = 'none', 3000);");
            client.println("      }");
            client.println("    });");
            client.println("}");
            client.println("</script>");

            
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
  }
}



void clearAllSettings() {
  Serial.println("Clearing all settings...");
  clearWiFiSettings();
  clearMQTTSettings();
  Serial.println("All settings cleared successfully!");
}
