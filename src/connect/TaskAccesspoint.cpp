#include "TaskAccesspoint.h"
#include "TaskAccesspointUI.h"

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
              handleAPScan(client);
              break;
            }
            else if (header.indexOf("GET /wifi?ssid=") >= 0) {
              int ssidIndex = header.indexOf("ssid=") + 5;
              int passIndex = header.indexOf("&pass=");
              int httpIndex = header.indexOf("HTTP");
              String ssid = header.substring(ssidIndex, passIndex);
              String pass = header.substring(passIndex + 6, httpIndex - 1);

              ssid.replace("%20", " ");
              handleAPWifiConnect(client, ssid, pass);
              return;
            }
            else if (header.indexOf("GET /stop") >= 0) {
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

              handleAPAccountSave(client, username, password);
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

              handleAPMqttSave(client, server, username, key);
              break;
            }
            
            // Default page - show both forms
            renderAPIndexPage(client);

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

