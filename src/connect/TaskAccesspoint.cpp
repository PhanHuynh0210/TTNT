#include "TaskAccesspoint.h"

WiFiServer apServer(80);
String header;

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

bool apMode = false;
unsigned long apStartTime = 0;  
const unsigned long AP_TIMEOUT = 15 * 60 * 1000;  

bool checkBootButtonNonBlocking() {
  const int BOOT_PIN = 0;  
  const unsigned long HOLD_TIME = 3000;  

  static bool initialized = false;
  static unsigned long pressStartTime = 0;
  static bool wasPressed = false;

  if (!initialized) {
    pinMode(BOOT_PIN, INPUT_PULLUP);
    initialized = true;
  }

  int level = digitalRead(BOOT_PIN);
  unsigned long now = millis();

  if (level == LOW) {
    if (!wasPressed) {
      wasPressed = true;
      pressStartTime = now;
      Serial.println("Giữ BOOT để vào Access Point...");
      setStatus(STATUS_BOOTING);
    }
    if (now - pressStartTime >= HOLD_TIME) {
      return true;
    }
  } else {
    if (wasPressed && (now - pressStartTime < HOLD_TIME)) {
      setStatus(STATUS_NORMAL);
    }
    wasPressed = false;
  }
  return false;
}

bool isAPMode() {
  return apMode;
}

void stopAP() {
  if (apMode) {
    WiFi.softAPdisconnect(true);
    delay(200);
    WiFi.mode(WIFI_STA);   
    apServer.stop();
    apMode = false;
    setStatus(STATUS_CONNECTING);
    InitWiFi();
    initMQTT();
  }
}
void initAP() {
  WiFi.softAP(ap_ssid, ap_password);
  apServer.begin();
  apStartTime = millis();
  apMode = true;
  setStatus(STATUS_AP_MODE);
}

void accpoint() {
  if (!apMode) {
    return;
  }
  
  if (millis() - apStartTime > AP_TIMEOUT) {
    Serial.println("Access Point timeout after 15 minutes. Stopping AP...");
    WiFiClient timeoutClient = apServer.available();
    if (timeoutClient) {
      timeoutClient.println("HTTP/1.1 200 OK");
      timeoutClient.println("Content-type:text/html");
      timeoutClient.println("Connection: close");
      timeoutClient.println();
      timeoutClient.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
      timeoutClient.println("<title>AP Timeout</title>");
      timeoutClient.println("<style>");
      timeoutClient.println("body{font-family:sans-serif;text-align:center;background:#f0f0f0;padding:50px;}");
      timeoutClient.println(".card{display:inline-block;background:white;padding:30px;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,0.1);}");
      timeoutClient.println("h2{color:orange;}");
      timeoutClient.println(".redirect{color:#007cba;margin-top:20px;}");
      timeoutClient.println("</style>");
      timeoutClient.println("<script>");
      timeoutClient.println("setTimeout(function() {");
      timeoutClient.println("  window.location.href = 'https://esp-web-80490.web.app/';");
      timeoutClient.println("}, 3000);");
      timeoutClient.println("</script>");
      timeoutClient.println("</head><body>");
      timeoutClient.println("<div class='card'>");
      timeoutClient.println("<h2>Access Point đã tự động tắt!</h2>");
      timeoutClient.println("<p>Hết thời gian cấu hình (15 phút).</p>");
      timeoutClient.println("<p class='redirect'>Tự động chuyển đến trang web điều khiển trong 3 giây...</p>");
      timeoutClient.println("<p><a href='https://esp-web-80490.web.app/' style='color:#007cba;text-decoration:none;'>Nhấn vào đây nếu không tự động chuyển</a></p>");
      timeoutClient.println("</div>");
      timeoutClient.println("</body></html>");
      timeoutClient.println();
      timeoutClient.stop();
      delay(1000);
    }
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
            if (header.indexOf("GET /scan") >= 0) {
              // Trả về danh sách WiFi dưới dạng JSON
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:application/json");
              client.println("Access-Control-Allow-Origin: *");
              client.println();
              
              int n = WiFi.scanNetworks();
              client.print("[");
              for (int i = 0; i < n; ++i) {
                if (i > 0) client.print(",");
                client.print("{\"ssid\":\"");
                client.print(WiFi.SSID(i));
                client.print("\",\"rssi\":");
                client.print(WiFi.RSSI(i));
                client.print(",\"encryption\":");
                client.print(WiFi.encryptionType(i));
                client.print("}");
              }
              client.print("]");
              break;
            }
            else if (header.indexOf("GET /wifi?ssid=") >= 0) {
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
                

                // Phản hồi ngắn gọn giống các API khác
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/plain");
                client.println("Connection: close");
                client.println();
                client.println("SAVED");
                client.stop();
                return;
              } else {
                Serial.println("\n Failed to connect to new WiFi!");
                Serial.println("Final WiFi Status: " + String(WiFi.status()));

                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/plain");
                client.println("Connection: close");
                client.println();
                client.println("FAILED");
                client.stop();

                setStatus(STATUS_ERROR);
                delay(2000);
                setStatus(STATUS_AP_MODE);
                return;
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
              client.println("h2{color:green;}");
              client.println(".redirect{color:#007cba;margin-top:20px;}");
              client.println("</style>");
              client.println("<script>");
              client.println("setTimeout(function() {");
              client.println("  window.location.href = 'https://esp-web-80490.web.app/';");
              client.println("}, 3000);");
              client.println("</script>");
              client.println("</head><body>");
              client.println("<div class='card'>");
              client.println("<h2> Access Point tắt!</h2>");
              client.println("<p>Cấu hình đã được lưu thành công.</p>");
              client.println("<p class='redirect'>Tự động chuyển đến trang web điều khiển trong 3 giây...</p>");
              client.println("<p><a href='https://esp-web-80490.web.app/' style='color:#007cba;text-decoration:none;'>Nhấn vào đây nếu không tự động chuyển</a></p>");
              client.println("</div>");
              client.println("</body></html>");
              client.println();

              client.stop();
              delay(300);
              stopAP();
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
            client.println("input,select,button{width:100%;padding:12px;margin:8px 0;box-sizing:border-box;border-radius:5px;border:1px solid #ccc;}");
            client.println("button{background:#007cba;color:white;border:none;cursor:pointer;}");
            client.println("button:hover{background:#005a87;}");
            client.println(".note{font-size:12px;color:#666;margin-top:5px;}");
            client.println(".success{background:#d4edda;color:#155724;padding:10px;border-radius:5px;margin-bottom:15px;}");
            client.println(".mqtt-info{background:#e7f3ff;padding:15px;border-radius:5px;margin-bottom:15px;border-left:4px solid #007cba;}");
            client.println(".highlight{background:#fff3cd;border:1px solid #ffeaa7;padding:10px;border-radius:5px;margin-bottom:15px;}");
            client.println(".wifi-item{display:flex;justify-content:space-between;align-items:center;padding:8px 0;border-bottom:1px solid #eee;}");
            client.println(".wifi-name{font-weight:bold;}");
            client.println(".wifi-strength{color:#666;font-size:12px;}");
            client.println(".refresh-btn{background:#28a745;margin-bottom:15px;}");
            client.println(".refresh-btn:hover{background:#218838;}");
            client.println(".loading{text-align:center;color:#666;font-style:italic;}");
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
            client.println("<button onclick='scanWiFi()' class='refresh-btn'>🔄 Quét WiFi</button>");
            client.println("<div id='wifiList'>");
            client.println("<div class='loading'>Nhấn nút 'Quét WiFi' để tìm mạng WiFi</div>");
            client.println("</div>");
            client.println("<form id='wifiForm' style='display:none;'>");
            client.println("<select id='wifiSelect' required>");
            client.println("<option value=''>Chọn mạng WiFi...</option>");
            client.println("</select>");
            client.println("<input id='wifiPassword' type='password' placeholder='Mật khẩu WiFi'>");
            client.println("<button type='submit'>Kết nối WiFi</button>");
            client.println("</form>");
            client.println("<div id='wifiMessage' class='success' style='display:none;'></div>");
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
            client.println("<input id='mqttServer' placeholder='MQTT Server (vd: m811669b.ala.asia-southeast1.emqxsl.com' required>");
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
            client.println("function scanWiFi() {");
            client.println("  const wifiList = document.getElementById('wifiList');");
            client.println("  wifiList.innerHTML = '<div class=\"loading\">Đang quét WiFi...</div>';");
            client.println("  ");
            client.println("  fetch('/scan')");
            client.println("    .then(res => res.json())");
            client.println("    .then(networks => {");
            client.println("      if (networks.length === 0) {");
            client.println("        wifiList.innerHTML = '<div class=\"loading\">Không tìm thấy mạng WiFi nào</div>';");
            client.println("        return;");
            client.println("      }");
            client.println("      ");
            client.println("      // Hiển thị danh sách WiFi");
            client.println("      let html = '<h3>Mạng WiFi tìm thấy:</h3>';");
            client.println("      networks.forEach(network => {");
            client.println("        const strength = network.rssi > -50 ? 'Mạnh' : network.rssi > -70 ? 'Trung bình' : 'Yếu';");
            client.println("        const strengthColor = network.rssi > -50 ? '#28a745' : network.rssi > -70 ? '#ffc107' : '#dc3545';");
            client.println("        html += `<div class=\"wifi-item\">`;");
            client.println("        html += `<span class=\"wifi-name\">${network.ssid}</span>`;");
            client.println("        html += `<span class=\"wifi-strength\" style=\"color:${strengthColor}\">${strength} (${network.rssi}dBm)</span>`;");
            client.println("        html += `</div>`;");
            client.println("      });");
            client.println("      wifiList.innerHTML = html;");
            client.println("      ");
            client.println("      // Cập nhật dropdown");
            client.println("      const select = document.getElementById('wifiSelect');");
            client.println("      select.innerHTML = '<option value=\"\">Chọn mạng WiFi...</option>';");
            client.println("      networks.forEach(network => {");
            client.println("        const option = document.createElement('option');");
            client.println("        option.value = network.ssid;");
            client.println("        option.textContent = `${network.ssid} (${network.rssi}dBm)`;");
            client.println("        select.appendChild(option);");
            client.println("      });");
            client.println("      ");
            client.println("      // Hiển thị form");
            client.println("      document.getElementById('wifiForm').style.display = 'block';");
            client.println("    })");
            client.println("    .catch(err => {");
            client.println("      wifiList.innerHTML = '<div class=\"loading\" style=\"color:#dc3545;\">Lỗi khi quét WiFi</div>';");
            client.println("    });");
            client.println("}");
            client.println("");
            client.println("// Xử lý form WiFi");
            client.println("document.getElementById('wifiForm').addEventListener('submit', function(e) {");
            client.println("  e.preventDefault();");
            client.println("  const ssid = document.getElementById('wifiSelect').value;");
            client.println("  const password = document.getElementById('wifiPassword').value;");
            client.println("  ");
            client.println("  if (!ssid) {");
            client.println("    alert('Vui lòng chọn mạng WiFi');");
            client.println("    return;");
            client.println("  }");
            client.println("  ");
            client.println("  const url = `/wifi?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(password)}`;");
            client.println("  window.location.href = url;");
            client.println("});");
            client.println("");
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
            client.println("      timeDisplay.textContent = 'Thời gian còn lại: ' + minutes + ':' + (seconds < 10 ? '0' : '') + seconds;");
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
