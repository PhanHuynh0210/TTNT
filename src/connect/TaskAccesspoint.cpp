#include "TaskAccesspoint.h"

WiFiServer apServer(90);
String header;

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

bool apMode = false;
unsigned long apStartTime = 0;  // Thời điểm bắt đầu AP
const unsigned long AP_TIMEOUT = 15 * 60 * 1000;  // 15 phút = 15 * 60 * 1000 ms

bool checkBootButton() {
  const int BOOT_PIN = 0;  
  const unsigned long HOLD_TIME = 3000;  
  
  pinMode(BOOT_PIN, INPUT_PULLUP);
  
  Serial.println("Nhấn giữ nút BOOT trong 3 giây để vào chế độ Access Point");
  
  unsigned long startTime = millis();
  bool buttonPressed = false;
  
  while (millis() - startTime < HOLD_TIME) {
    if (digitalRead(BOOT_PIN) == LOW) { 
      buttonPressed = true;
      Serial.print(".");
      // Hiển thị đèn vàng nhấp nháy nhanh khi đang nhấn nút
      setStatus(STATUS_BOOTING);
      delay(100);
    } else {
      if (buttonPressed) {
        Serial.println("\nNút boot được thả ra sớm!");
        setStatus(STATUS_NORMAL);
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
  }
}

bool isAPMode() {
  return apMode;
}

void stopAP() {
  if (apMode) {
    WiFi.softAPdisconnect(true);
    apServer.stop(); 
    apMode = false;
    
    // Tắt đèn tín hiệu AP và chuyển về trạng thái bình thường
    setStatus(STATUS_NORMAL);
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
  
  // Ghi lại thời điểm bắt đầu AP
  apStartTime = millis();
  
  // Bật đèn tín hiệu xanh dương nhấp nháy cho chế độ AP
  setStatus(STATUS_AP_MODE);
}

void accpoint() {
  if (!apMode) {
    return;
  }
  
  // Kiểm tra timeout 15 phút
  if (millis() - apStartTime > AP_TIMEOUT) {
    Serial.println("Access Point timeout after 15 minutes. Stopping AP...");
    stopAP();
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
                
                // Không tự động chuyển sang NORMAL, để updateSystemStatus() xử lý
                // khi tất cả kết nối hoàn tất

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
                client.println(".button{background:#dc3545;color:white;padding:10px 20px;border:none;border-radius:5px;text-decoration:none;display:inline-block;margin:10px;}");
                client.println(".button:hover{background:#c82333;}");
                client.println("</style></head><body>");
                client.println("<div class='card'>");
                client.println("<h2>Kết nối WiFi thành công!</h2>");
                client.println("<p>Địa chỉ IP của ESP32:</p>");
                client.println("<p><strong>" + staIP + "</strong></p>");
                client.println("<p>Access Point vẫn hoạt động để cấu hình MQTT.</p>");
                client.println("<p>Bạn có thể:</p>");
                client.println("<a href='/stop' class='button'>Tắt Access Point</a>");
                client.println("<p><small>Hoặc AP sẽ tự động tắt sau 15 phút</small></p>");
                client.println("</div>");
                client.println("</body></html>");
                client.println();

                client.stop();
                return;
              } else {
                Serial.println("\n Failed to connect to new WiFi!");
                Serial.println("Final WiFi Status: " + String(WiFi.status()));
                
                // Hiển thị đèn đỏ khi kết nối WiFi thất bại
                setStatus(STATUS_ERROR);
                delay(2000); // Hiển thị lỗi trong 2 giây
                setStatus(STATUS_AP_MODE); // Quay lại trạng thái AP
                break;
              }
            }
            else if (header.indexOf("GET /stop") >= 0) {
              // Tắt Access Point thủ công
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
              client.println("<title>AP Stopped</title>");
              client.println("<style>");
              client.println("body{font-family:sans-serif;text-align:center;background:#f0f0f0;padding:50px;}");
              client.println(".card{display:inline-block;background:white;padding:30px;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1);}");
              client.println("h2{color:orange;}");
              client.println("</style></head><body>");
              client.println("<div class='card'>");
              client.println("<h2>Access Point đã được tắt!</h2>");
              client.println("<p>ESP32 sẽ khởi động lại để áp dụng cấu hình.</p>");
              client.println("</div>");
              client.println("</body></html>");
              client.println();

              client.stop();
              delay(2000);
              stopAP();
              ESP.restart();
              return;
            }
            else if (header.indexOf("GET /account?username=") >= 0) {
              int usernameIndex = header.indexOf("username=") + 9;
              int passwordIndex = header.indexOf("&password=") + 10;
              int httpIndex = header.indexOf("HTTP");
              
              String username = header.substring(usernameIndex, passwordIndex - 10);
              String password = header.substring(passwordIndex, httpIndex - 1);
              
              username.replace("%20", " ");
              password.replace("%20", " ");

              Serial.println("Received Account settings:");
              Serial.println("Username: " + username);
              Serial.println(String("Password: ") + (password.length() > 0 ? "****" : "empty"));

              saveAccountSettings(username, password);
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/plain");
              client.println();
              client.println("SAVED");

              break;
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
            
            // Hiển thị thông tin thời gian còn lại
            unsigned long remainingTime = (AP_TIMEOUT - (millis() - apStartTime)) / 1000; // Chuyển về giây
            unsigned long minutes = remainingTime / 60;
            unsigned long seconds = remainingTime % 60;
            
            client.println("<div class='highlight'>");
            client.println("<strong>Thời gian còn lại: " + String(minutes) + ":" + String(seconds < 10 ? "0" : "") + String(seconds) + "</strong>");
            client.println("<br><small>Access Point sẽ tự động tắt sau 15 phút</small>");
            client.println("</div>");
            
            
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
            client.println("<input id='mqttServer' placeholder='MQTT Server (vd: m811669b.ala.asia-southeast1.emqxsl.com' value='io.adafruit.com' required>");
            client.println("<div class='note'>Mặc định: m811669b.ala.asia-southeast1.emqxsl.com (port 8883)</div>");
            client.println("<input id='mqttUsername' placeholder='MQTT Username' required>");
            client.println("<input id='mqttKey' type='password' placeholder='MQTT Key/Password' required>");
            client.println("<button type='submit'>Save MQTT Settings</button>");
            client.println("</form>");
            client.println("<div id='mqttMessage' class='success' style='display:none;'>Đã lưu cấu hình MQTT thành công!</div>");
            client.println("</div>");
            
            // Thêm form tài khoản
            String currentAccountUsername = getCurrentAccountUsername();
            client.println("<div class='card'>");
            client.println("<h2>Account Settings</h2>");
            client.println("<div class='account-info'>");
            client.println("<strong>Tài khoản hiện tại:</strong><br>");
            client.println("Username: " + (currentAccountUsername.length() > 0 ? currentAccountUsername : "Chưa cấu hình"));
            client.println("</div>");
            
            client.println("<form id='accountForm' onsubmit='submitAccount(event)'>");
            client.println("<input id='accountUsername' placeholder='Tài khoản' required>");
            client.println("<input id='accountPassword' type='password' placeholder='Mật khẩu' required>");
            client.println("<button type='submit'>Lưu Tài Khoản</button>");
            client.println("</form>");
            client.println("<div id='accountMessage' class='success' style='display:none;'>Đã lưu tài khoản thành công!</div>");
            client.println("</div>");
            
            // Thêm nút tắt Access Point
            client.println("<div class='card'>");
            client.println("<h2>Quản lý Access Point</h2>");
            client.println("<p>Khi đã cấu hình xong, bạn có thể tắt Access Point:</p>");
            client.println("<a href='/stop' class='button' style='background:#dc3545;color:white;padding:12px;text-decoration:none;border-radius:5px;display:inline-block;'>🛑 Tắt Access Point</a>");
            client.println("<p><small>Hoặc chờ 15 phút để tự động tắt</small></p>");
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
            client.println("");
            client.println("function submitAccount(event) {");
            client.println("  event.preventDefault();");
            client.println("  const username = document.getElementById('accountUsername').value;");
            client.println("  const password = document.getElementById('accountPassword').value;");
            client.println("  const url = `/account?username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}`;");
            client.println("  fetch(url)");
            client.println("    .then(res => res.text())");
            client.println("    .then(text => {");
            client.println("      if (text.trim() === 'SAVED') {");
            client.println("        const msg = document.getElementById('accountMessage');");
            client.println("        msg.style.display = 'block';");
            client.println("        setTimeout(() => msg.style.display = 'none', 3000);");
            client.println("        // Cập nhật hiển thị username hiện tại");
            client.println("        const currentUser = document.querySelector('.account-info strong').nextSibling.nextSibling;");
            client.println("        if (currentUser) {");
            client.println("          currentUser.textContent = 'Username: ' + username;");
            client.println("        }");
            client.println("      }");
            client.println("    });");
            client.println("}");
            client.println("");
            client.println("// Cập nhật thời gian còn lại mỗi giây");
            client.println("let remainingSeconds = " + String(remainingTime) + ";");
            client.println("setInterval(() => {");
            client.println("  remainingSeconds--;");
            client.println("  if (remainingSeconds <= 0) {");
            client.println("    document.body.innerHTML = '<div style=\"text-align:center;padding:50px;\"><h2>Access Point đã tự động tắt</h2><p>ESP32 sẽ khởi động lại...</p></div>';");
            client.println("  } else {");
            client.println("    const minutes = Math.floor(remainingSeconds / 60);");
            client.println("    const seconds = remainingSeconds % 60;");
            client.println("    const timeDisplay = document.querySelector('.highlight strong');");
            client.println("    if (timeDisplay) {");
            client.println("      timeDisplay.textContent = '⏰ Thời gian còn lại: ' + minutes + ':' + (seconds < 10 ? '0' : '') + seconds;");
            client.println("    }");
            client.println("  }");
            client.println("}, 1000);");
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



unsigned long getAPRemainingTime() {
  if (!apMode) {
    return 0;
  }
  unsigned long elapsed = millis() - apStartTime;
  if (elapsed >= AP_TIMEOUT) {
    return 0;
  }
  return AP_TIMEOUT - elapsed;
}

// Function để lưu thông tin tài khoản vào Preferences
void saveAccountSettings(String username, String password) {
  Preferences accountPrefs;
  accountPrefs.begin("account-config", false);
  accountPrefs.putString("username", username);
  accountPrefs.putString("password", password);
  accountPrefs.end();
  Serial.println("Account settings saved successfully!");
}

String getCurrentAccountUsername() {
  Preferences accountPrefs;
  accountPrefs.begin("account-config", true);
  String username = accountPrefs.getString("username", "");
  accountPrefs.end();
  return username;
}

String getCurrentAccountPassword() {
  Preferences accountPrefs;
  accountPrefs.begin("account-config", true);
  String password = accountPrefs.getString("password", "");
  accountPrefs.end();
  return password;
}

// Function để xóa thông tin tài khoản
void clearAccountSettings() {
  Preferences accountPrefs;
  accountPrefs.begin("account-config", false);
  accountPrefs.clear();
  accountPrefs.end();
  Serial.println("Account settings cleared successfully!");
}

void clearAllSettings() {
  Serial.println("Clearing all settings...");
  clearWiFiSettings();
  clearMQTTSettings();
  clearAccountSettings();
  Serial.println("All settings cleared successfully!");
}
