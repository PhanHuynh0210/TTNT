#include "TaskMQTT.h"

Preferences mqttPrefs;

String MQTT_SERVER = server_mqtt;
int MQTT_PORT = mqtt_port;
String IO_USERNAME = "";
String IO_KEY = "";
const char* emqxCACert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";


WiFiClientSecure espSecure; 
PubSubClient client(espSecure);
String urlDecode(String input) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = input.length();
  unsigned int i = 0;

  while (i < len) {
    char c = input.charAt(i);
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < len) {
      temp[2] = input.charAt(i + 1);
      temp[3] = input.charAt(i + 2);
      decoded += (char)strtol(temp, NULL, 16);
      i += 2;
    } else {
      decoded += c;
    }
    i++;
  }
  return decoded;
}


void loadMQTTSettings() {
    mqttPrefs.begin("mqtt-config", true);
    
    MQTT_SERVER = mqttPrefs.getString("server", "m811669b.ala.asia-southeast1.emqxsl.com");
    IO_USERNAME = mqttPrefs.getString("username", "");
    IO_KEY = mqttPrefs.getString("key", "");
    
    mqttPrefs.end();

    Serial.println("MQTT Settings loaded:");
    Serial.println("Server: " + MQTT_SERVER);
    Serial.println("Username: " + IO_USERNAME);
    Serial.println(String("Key: ") + (IO_KEY.length() > 0 ? "****" : "empty"));
}

void saveMQTTSettings(String server, String username, String key) {
    key = urlDecode(key);

    mqttPrefs.begin("mqtt-config", false);
    
    mqttPrefs.putString("server", server);
    mqttPrefs.putString("username", username);
    mqttPrefs.putString("key", key);
    
    mqttPrefs.end();
    
    Serial.println("MQTT settings saved to flash memory.");
    
    MQTT_SERVER = server;
    IO_USERNAME = username;
    IO_KEY = key;
    
    // Cập nhật lại server cho client
    client.setServer(MQTT_SERVER.c_str(), MQTT_PORT);
}

void clearMQTTSettings() {
    Serial.println("Clearing saved MQTT settings...");
    
    mqttPrefs.begin("mqtt-config", false);
    mqttPrefs.clear();
    mqttPrefs.end();
    
    Serial.println("MQTT settings cleared successfully!");
    
    MQTT_SERVER = "";
    IO_USERNAME = "";
    IO_KEY = "";
}

void callback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    if (String(topic) == "esp32/control/led") {
    digitalWrite(48, message == "1" ? HIGH : LOW);
    }
    // else if (String(topic) == "esp32/control/rgb") {
    //     digitalWrite(RGB_POWER_PIN, msg == "1" ? HIGH : LOW);
    // }
    // else if (String(topic) == "esp32/control/mode") {
    //     rgbMode = msg; // "auto" hoặc "static"
    // }
    // else if (String(topic) == "esp32/control/color") {
    //     rgbColorHex = msg;
    //     // Chuyển hex sang RGB nếu cần
    // }
    else if (String(topic) == "esp32/config/threshold") {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, message);
        if (!err) {
        thresholdTemp = doc["temp"];
        thresholdHumid = doc["humid"];
        thresholdSoli = doc["soli"];
        thresholdLux = doc["lux"];
        thresholdDistance = doc["distance"];
        email = doc["email"].as<String>();

        Serial.print("Temp: "); Serial.println(thresholdTemp);
        Serial.print("Humid: "); Serial.println(thresholdHumid);
        Serial.print("Soli: "); Serial.println(thresholdSoli);
        Serial.print("Lux: "); Serial.println(thresholdLux);
        Serial.print("Distance: "); Serial.println(thresholdDistance);
        Serial.print("Email: "); Serial.println(email);
        }
    }
    else if (String(topic) == "esp32/ota") {
        // Parse URL và bắt đầu OTA
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, message);
        if (!err) {
        String firmwareUrl = doc["url"];
        // Gọi hàm OTA cập nhật từ URL
        Serial.println("=== OTA yêu cầu nhận được ===");
        Serial.println("URL OTA: " + firmwareUrl);
        Serial.println("=============================");
        t_httpUpdate_return ret = httpUpdate.update(espSecure, firmwareUrl);
        switch (ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("OTA thất bại: %s\n", httpUpdate.getLastErrorString().c_str());
                break;
            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("Không có phiên bản mới.");
                break;
            case HTTP_UPDATE_OK:
                Serial.println("OTA thành công. ESP sẽ khởi động lại.");
                break;

        }
    }
    }
}

void publishData(String feed, String data)
{
    String topic = String(IO_USERNAME) + "/feeds/" + feed;
    if (client.connected())
    {
        client.publish(topic.c_str(), data.c_str());
    }
}

void InitMQTT()
{
    espSecure.setCACert(emqxCACert);  
    client.setServer(MQTT_SERVER.c_str(), MQTT_PORT);
    client.setCallback(callback);
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Cannot connect to MQTT.");
        return;
    }
    
    if (IO_USERNAME.length() == 0 || IO_KEY.length() == 0) {
        Serial.println("MQTT credentials not set. Cannot connect to MQTT.");
        return;
    }
    Serial.println("MQTT_USERNAME: " + IO_USERNAME);
    Serial.println("MQTT_KEY: " + IO_KEY);

    
    Serial.println("Connecting to MQTT...");
    String clientId = "ESP32Client" + String(random(0, 1000));
    if (client.connect(clientId.c_str(), IO_USERNAME.c_str(), IO_KEY.c_str()))
    {
        Serial.println("MQTT Connected");

    }
    else
    {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(client.state());
        delay(1000);
    }
        client.subscribe("esp32/control/led");
        client.subscribe("esp32/control/rgb");
        client.subscribe("esp32/control/mode");
        client.subscribe("esp32/control/color");
        client.subscribe("esp32/config/threshold");
        client.subscribe("esp32/ota");
}

void reconnectMQTT()
{
    if (client.connected())
    {
        client.loop();
    }
    else
    {
        if (WiFi.status() == WL_CONNECTED) {
            InitMQTT();
        }
    }
}

void initMQTT()
{
    // Load settings from preferences first
    loadMQTTSettings();
    
    client.setServer(MQTT_SERVER.c_str(), MQTT_PORT);
    client.setCallback(callback);
}

String getCurrentMQTTUsername() {
    return IO_USERNAME;
}

String getCurrentMQTTServer() {
    return MQTT_SERVER;
}

bool isMQTTConnected() {
    return client.connected();
}