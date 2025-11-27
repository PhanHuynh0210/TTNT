#include "TaskMQTT.h"
#include "TaskAccesspoint.h"

Preferences mqttPrefs;
Preferences otaPrefs;
Preferences SensorPrefs;

String email = "";
String emailKey = "";
float thresholdTemp = 30.0;
float thresholdHumid = 80.0;
float thresholdLux = 400.0;
float thresholdSoli = 300.0;
float thresholdDistance = 50.0;

#define TOPIC_LED_CONTROL 1
#define TOPIC_RGB_CONTROL 2
#define TOPIC_THRESHOLD_CONFIG 3
#define TOPIC_AUTH_REQUEST 4
#define TOPIC_OTA_UPDATE 5
#define TOPIC_VERSION_REQUEST 6
#define TOPIC_MQTT_CONFIG_TEST 7
#define TOPIC_UNKNOWN 0

String opTemp, opHumid, opSoli, opLux, opDistance;

unsigned long lastMailTime = 0;
const unsigned long MAIL_INTERVAL = 60000;

// WiFi Status publishing
unsigned long lastWiFiStatusTime = 0;
const unsigned long WIFI_STATUS_INTERVAL = 30000;  // 30 seconds

// Forward declarations
void InitMQTT(); 

String MQTT_SERVER = "m811669b.ala.asia-southeast1.emqxsl.com";
int MQTT_PORT = 8883;
String IO_USERNAME = "huynh0210";
String IO_KEY = "Huynh@0210";
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

// decoder
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


// MQTT Default Configuration
#define DEFAULT_MQTT_SERVER "m811669b.ala.asia-southeast1.emqxsl.com"
#define DEFAULT_MQTT_USERNAME "huynh0210"
#define DEFAULT_MQTT_PASSWORD "Huynh@0210"

void loadMQTTSettings() {
    mqttPrefs.begin("mqtt-config", true);
    
    MQTT_SERVER = mqttPrefs.getString("server", DEFAULT_MQTT_SERVER);
    IO_USERNAME = mqttPrefs.getString("username", DEFAULT_MQTT_USERNAME);
    IO_KEY = mqttPrefs.getString("key", DEFAULT_MQTT_PASSWORD);
    MQTT_PORT = mqttPrefs.getInt("port", 8883);  // Default port for MQTT over TLS
    
    mqttPrefs.end();

    Serial.println("MQTT Settings loaded:");
    Serial.println("Server: " + MQTT_SERVER);
    Serial.println("Port: " + String(MQTT_PORT));
    Serial.println("Username: " + IO_USERNAME);
    Serial.println(String("Key: ") + (IO_KEY.length() > 0 ? "****" : "empty"));
}

void saveMQTTSettings(String server, String username, String key, int port) {
    key = urlDecode(key);

    mqttPrefs.begin("mqtt-config", false);
    
    mqttPrefs.putString("server", server);
    mqttPrefs.putString("username", username);
    mqttPrefs.putString("key", key);
    mqttPrefs.putInt("port", port);
    
    mqttPrefs.end();
    
    Serial.println("MQTT settings saved to flash memory.");
    Serial.printf("Server: %s, Port: %d, Username: %s\n", server.c_str(), port, username.c_str());
    
    MQTT_SERVER = server;
    IO_USERNAME = username;
    IO_KEY = key;
    MQTT_PORT = port;
    
    // Cập nhật lại server cho client
    client.setServer(MQTT_SERVER.c_str(), MQTT_PORT);
}

// Overload for backward compatibility
void saveMQTTSettings(String server, String username, String key) {
    saveMQTTSettings(server, username, key, 8883);
}

float parseValue(const JsonVariant &v) {
  if (v.is<float>()) return v.as<float>();
  if (v.is<const char*>()) return String(v.as<const char*>()).toFloat();
  return 0.0;
}


// Hash function để chuyển topic thành số
int hashTopic(String topic) {
    if (topic == "esp32/control/relay1") return TOPIC_LED_CONTROL;
    if (topic == "esp32/control/relay2") return TOPIC_RGB_CONTROL;
    if (topic == "esp32/config/threshold") return TOPIC_THRESHOLD_CONFIG;
    if (topic == "esp32/auth/request") return TOPIC_AUTH_REQUEST;
    if (topic == "esp32/ota") return TOPIC_OTA_UPDATE;
    if (topic == "esp32/ota/version") return TOPIC_VERSION_REQUEST;
    if (topic == "esp32/config/mqtt/test") return TOPIC_MQTT_CONFIG_TEST;
    return TOPIC_UNKNOWN;
}



void handleThresholdConfig(String message) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);
    if (!err) {
        if (doc.containsKey("temp")) {
            thresholdTemp = parseValue(doc["temp"]["value"]);
            opTemp = doc["temp"]["op"].as<String>();
        }
        if (doc.containsKey("humid")) {
            thresholdHumid = parseValue(doc["humid"]["value"]);
            opHumid = doc["humid"]["op"].as<String>();
        }
        if (doc.containsKey("soli")) {
            thresholdSoli = parseValue(doc["soli"]["value"]);
            opSoli = doc["soli"]["op"].as<String>();
        }
        if (doc.containsKey("lux")) {
            thresholdLux = parseValue(doc["lux"]["value"]);
            opLux = doc["lux"]["op"].as<String>();
        }
        if (doc.containsKey("distance")) {
            thresholdDistance = parseValue(doc["distance"]["value"]);
            opDistance = doc["distance"]["op"].as<String>();
        }
        if (doc.containsKey("email")) {
            email = doc["email"].as<String>();
        }
        if (doc.containsKey("key")) {            // 🔑 đọc key email
            emailKey = doc["key"].as<String>();
        }

        // Lưu vào Preferences
        SensorPrefs.begin("sensor-config", false);
        SensorPrefs.putFloat("temp", thresholdTemp);
        SensorPrefs.putString("tempOp", opTemp);
        SensorPrefs.putFloat("humid", thresholdHumid);
        SensorPrefs.putString("humidOp", opHumid);
        SensorPrefs.putFloat("lux", thresholdLux);
        SensorPrefs.putString("luxOp", opLux);
        SensorPrefs.putFloat("soli", thresholdSoli);
        SensorPrefs.putString("soliOp", opSoli);
        SensorPrefs.putFloat("distance", thresholdDistance);
        SensorPrefs.putString("distanceOp", opDistance);
        SensorPrefs.putString("email", email);
        SensorPrefs.putString("emailKey", emailKey);  // 🔑 lưu key vào flash
        SensorPrefs.end();

        Serial.println("✔ Cập nhật ngưỡng và toán tử:");
        Serial.printf("  Temp: %s %.2f\n", opTemp.c_str(), thresholdTemp);
        Serial.printf("  Humid: %s %.2f\n", opHumid.c_str(), thresholdHumid);
        Serial.printf("  Soli: %s %.2f\n", opSoli.c_str(), thresholdSoli);
        Serial.printf("  Lux: %s %.2f\n", opLux.c_str(), thresholdLux);
        Serial.printf("  Distance: %s %.2f\n", opDistance.c_str(), thresholdDistance);
        Serial.println("  Email: " + email);
        Serial.println("  Email Key: " + emailKey);
    }
}


void handleOTAUpdate(String message) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);
    if (!err) {
        String firmwareUrl = doc["url"];
        String version = doc["version"] | "unknown";

        WiFiClientSecure otaClient;
        otaClient.setInsecure(); 

        otaPrefs.begin("ota-info", false);
        otaPrefs.putString("version", version);
        otaPrefs.end();

        Serial.println("=== OTA yêu cầu nhận được ===");
        Serial.println("URL OTA: " + firmwareUrl);
        Serial.println("=============================");

        // Chuyển sang trạng thái OTA update - đèn cam nhấp nháy
        setStatus(STATUS_OTA_UPDATE);
        
        // Gửi thông báo bắt đầu OTA qua topic wifi status
        client.publish("esp32/status/wifi", "ota_update");
        delay(100);  // Đảm bảo message được gửi
        
        t_httpUpdate_return ret = httpUpdate.update(otaClient, firmwareUrl);
        switch (ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("OTA thất bại: %s\n", httpUpdate.getLastErrorString().c_str());
                setStatus(STATUS_ERROR);
                break;
            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("Không có phiên bản mới.");
                setStatus(STATUS_NORMAL);
                break;
            case HTTP_UPDATE_OK:
                Serial.println("OTA thành công. ESP sẽ khởi động lại.");
                break;
        }
    }
}

void handleVersionRequest() {
    otaPrefs.begin("ota-info", true);
    String currentVersion = otaPrefs.getString("version", "unknown");
    otaPrefs.end();
    client.publish("esp32/ota/version/response", currentVersion.c_str());
}

// ===== MQTT CONFIG TEST - Test new MQTT configuration via topic =====
void handleMQTTConfigTest(String message) {
    Serial.println("=== MQTT Config Test Received ===");
    Serial.println("Payload: " + message);
    
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);
    
    if (err) {
        Serial.println("JSON parse error: " + String(err.c_str()));
        // Gửi response lỗi
        JsonDocument errorDoc;
        errorDoc["status"] = "error";
        errorDoc["reason"] = "Invalid JSON format: " + String(err.c_str());
        String errorJson;
        serializeJson(errorDoc, errorJson);
        client.publish("esp32/config/mqtt/mqtt_result", errorJson.c_str());
        return;
    }
    
    // Lấy thông tin từ JSON
    String newHost = doc["host"].as<String>();
    String newUsername = doc["username"].as<String>();
    String newPassword = doc["password"].as<String>();
    
    Serial.println("New Host: " + newHost);
    Serial.println("New Username: " + newUsername);
    Serial.println("New Password: " + String(newPassword.length() > 0 ? "****" : "empty"));
    
    // Validate input
    if (newHost.length() == 0 || newUsername.length() == 0 || newPassword.length() == 0) {
        JsonDocument errorDoc;
        errorDoc["status"] = "error";
        errorDoc["reason"] = "Missing required fields (host, username, password)";
        String errorJson;
        serializeJson(errorDoc, errorJson);
        client.publish("esp32/config/mqtt/mqtt_result", errorJson.c_str());
        Serial.println("Error: Missing required fields");
        return;
    }
    
    // Parse host URL để lấy server
    // Format hỗ trợ: wss://server:port/mqtt, mqtts://server:port, server:port, server
    String testServer = newHost;
    int testPort = 8883; // Default MQTT over TLS port (ESP32 dùng MQTTS, không phải WSS)
    
    // Xử lý URL format wss://server:port/mqtt hoặc mqtts://server:port
    if (newHost.startsWith("wss://")) {
        testServer = newHost.substring(6); // Bỏ "wss://"
        testPort = 8883; // ESP32 dùng MQTT over TLS, không phải WebSocket
    } else if (newHost.startsWith("ws://")) {
        testServer = newHost.substring(5); // Bỏ "ws://"
        testPort = 8883;
    } else if (newHost.startsWith("mqtts://")) {
        testServer = newHost.substring(8); // Bỏ "mqtts://"
    } else if (newHost.startsWith("mqtt://")) {
        testServer = newHost.substring(7); // Bỏ "mqtt://"
        testPort = 1883; // Non-TLS port
    }
    
    // Tách port nếu có trong URL (nhưng ưu tiên dùng 8883 cho TLS)
    int colonIndex = testServer.indexOf(':');
    int slashIndex = testServer.indexOf('/');
    
    if (colonIndex > 0) {
        // Chỉ lấy server name, bỏ port trong URL vì ESP32 dùng port 8883
        testServer = testServer.substring(0, colonIndex);
    }
    
    // Bỏ phần path (/mqtt)
    slashIndex = testServer.indexOf('/');
    if (slashIndex > 0) {
        testServer = testServer.substring(0, slashIndex);
    }
    
    Serial.println("Parsed Server: " + testServer);
    Serial.println("Using Port: " + String(testPort) + " (MQTT over TLS)");
    
    // Lưu cấu hình hiện tại để có thể khôi phục
    String backupServer = MQTT_SERVER;
    int backupPort = MQTT_PORT;
    String backupUsername = IO_USERNAME;
    String backupKey = IO_KEY;
    
    // Tạo client test mới
    WiFiClientSecure testSecure;
    testSecure.setCACert(emqxCACert);
    PubSubClient testClient(testSecure);
    testClient.setServer(testServer.c_str(), testPort);
    
    Serial.println("Testing connection to new MQTT server...");
    
    // Thử kết nối với thông tin mới
    String testClientId = "ESP32Test" + String(random(0, 10000));
    bool connectSuccess = testClient.connect(testClientId.c_str(), newUsername.c_str(), newPassword.c_str());
    
    JsonDocument resultDoc;
    
    if (connectSuccess) {
        Serial.println("✅ Test connection successful!");
        
        // Ngắt kết nối test
        testClient.disconnect();
        
        // Lưu cấu hình mới vào flash (bao gồm port)
        saveMQTTSettings(testServer, newUsername, newPassword, testPort);
        
        resultDoc["status"] = "ok";
        
        String resultJson;
        serializeJson(resultDoc, resultJson);
        
        // Gửi kết quả qua kết nối hiện tại trước khi chuyển đổi
        client.publish("esp32/config/mqtt/mqtt_result", resultJson.c_str());
        Serial.println("Result sent: " + resultJson);
        
        // Cho thời gian gửi message
        delay(500);
        
        // Ngắt kết nối cũ và kết nối lại với server mới
        Serial.println("Switching to new MQTT server...");
        client.disconnect();
        
        // Cập nhật client chính với server mới
        espSecure.setCACert(emqxCACert);
        client.setServer(MQTT_SERVER.c_str(), MQTT_PORT);
        
        // Kết nối lại
        delay(1000);
        InitMQTT();
        
    } else {
        int state = testClient.state();
        String errorReason = "Connection failed. ";
        
        switch (state) {
            case -4: errorReason += "MQTT_CONNECTION_TIMEOUT"; break;
            case -3: errorReason += "MQTT_CONNECTION_LOST"; break;
            case -2: errorReason += "MQTT_CONNECT_FAILED"; break;
            case -1: errorReason += "MQTT_DISCONNECTED"; break;
            case 1: errorReason += "MQTT_CONNECT_BAD_PROTOCOL"; break;
            case 2: errorReason += "MQTT_CONNECT_BAD_CLIENT_ID"; break;
            case 3: errorReason += "MQTT_CONNECT_UNAVAILABLE"; break;
            case 4: errorReason += "MQTT_CONNECT_BAD_CREDENTIALS"; break;
            case 5: errorReason += "MQTT_CONNECT_UNAUTHORIZED"; break;
            default: errorReason += "Unknown error (code: " + String(state) + ")"; break;
        }
        
        Serial.println("❌ Test connection failed: " + errorReason);
        
        resultDoc["status"] = "error";
        resultDoc["reason"] = errorReason;
        
        String resultJson;
        serializeJson(resultDoc, resultJson);
        client.publish("esp32/config/mqtt/mqtt_result", resultJson.c_str());
        Serial.println("Result sent: " + resultJson);
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    
    String topicStr = String(topic);
    
    // Sử dụng switch với hash của topic
    switch (hashTopic(topicStr)) {
        case TOPIC_LED_CONTROL:
            digitalWrite(17, message == "1" ? HIGH : LOW);
            break;
            
        case TOPIC_RGB_CONTROL:
            digitalWrite(18, message == "1" ? HIGH : LOW);
            break;
            
        case TOPIC_THRESHOLD_CONFIG:
            handleThresholdConfig(message);
            break;
            
        case TOPIC_AUTH_REQUEST:
            handleAuthRequest(message);
            break;
            
        case TOPIC_OTA_UPDATE:
            handleOTAUpdate(message);
            break;
            
        case TOPIC_VERSION_REQUEST:
            handleVersionRequest();
            break;
            
        case TOPIC_MQTT_CONFIG_TEST:
            handleMQTTConfigTest(message);
            break;
            
        default:
            Serial.println("Unknown topic: " + topicStr);
            break;
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
        // Cập nhật trạng thái hệ thống khi MQTT kết nối thành công
        updateSystemStatus();
    }
    else
    {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(client.state());
        delay(1000);
    }
        client.subscribe("esp32/control/relay1");
        client.subscribe("esp32/control/relay2");
        client.subscribe("esp32/config/threshold");
        client.subscribe("esp32/ota");
        client.subscribe("esp32/auth/request"); 
        client.subscribe("esp32/ota/version");
        client.subscribe("esp32/config/mqtt/test");  // Topic nhận cấu hình MQTT mới
}

void reconnectMQTT()
{
    if (isAPMode() || currentStatus == STATUS_BOOTING) {
        // Đang trong AP mode hoặc boot nên bỏ qua reconnect.
        return;
    }

    bool wifiConnected = (WiFi.status() == WL_CONNECTED);
    bool mqttConnected = client.connected();

    if (wifiConnected && mqttConnected) {
        client.loop();
        
        // Gửi trạng thái WiFi mỗi 30 giây
        publishWiFiStatus();
        return;
    }

    if (currentStatus != STATUS_CONNECTING) {
        setStatus(STATUS_CONNECTING);
    }

    if (!wifiConnected) {
        InitWiFi();
        // Chờ WiFi lên rồi mới thử MQTT ở vòng lặp sau.
        return;
    }

    if (!mqttConnected) {
        InitMQTT();
    }
}

void initMQTT()
{
    loadMQTTSettings();
    initAlertSystem();
    
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

// ===== WIFI STATUS PUBLISHER =====
void publishWiFiStatus() {
    unsigned long now = millis();
    
    // Chỉ gửi nếu đã qua 30 giây và MQTT đang kết nối
    if (now - lastWiFiStatusTime < WIFI_STATUS_INTERVAL) {
        return;
    }
    
    if (!client.connected()) {
        return;
    }
    
    lastWiFiStatusTime = now;
    
    // Kiểm tra WiFi còn kết nối không
    bool wifiConnected = (WiFi.status() == WL_CONNECTED);
    
    String status = wifiConnected ? "connected" : "disconnected";
    client.publish("esp32/status/wifi", status.c_str());
    
    Serial.println("📡 WiFi Status: " + status);
}

void handleAuthRequest(String message) {
    Serial.println("=== Auth Request Received ===");
    Serial.println("Message: " + message);
    
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, message);
    if (!err) {
        String requestUsername = doc["username"].as<String>();
        String requestPassword = doc["password"].as<String>();
        
        Serial.println("Request Username: " + requestUsername);
        Serial.println("Request Password: " + String(requestPassword.length() > 0 ? "****" : "empty"));
        
        // Lấy thông tin tài khoản đã lưu
        String savedUsername = getCurrentAccountUsername();
        String savedPassword = getCurrentAccountPassword();
        
        Serial.println("Saved Username: " + savedUsername);
        Serial.println("Saved Password: " + String(savedPassword.length() > 0 ? "****" : "empty"));
        
        // So sánh thông tin
        bool isAuthenticated = false;
        if (requestUsername == savedUsername && requestPassword == savedPassword) {
            isAuthenticated = true;
            Serial.println("Authentication: ACCEPTED");
        } else {
            Serial.println("Authentication: REJECTED");
        }
        
        // Tạo response JSON
        JsonDocument responseDoc;
        responseDoc["status"] = isAuthenticated ? "accept" : "no";
        responseDoc["message"] = isAuthenticated ? "Authentication successful" : "Invalid credentials";
        
        String responseJson;
        serializeJson(responseDoc, responseJson);
        
        // Gửi response
        client.publish("esp32/auth/response", responseJson.c_str());
        Serial.println("Response sent: " + responseJson);
    } else {
        Serial.println("Failed to parse auth request JSON");
        // Gửi response lỗi
        JsonDocument errorDoc;
        errorDoc["status"] = "no";
        errorDoc["message"] = "Invalid JSON format";
        
        String errorJson;
        serializeJson(errorDoc, errorJson);
        client.publish("esp32/auth/response", errorJson.c_str());
    }
}

// ===== ALERT SYSTEM FUNCTIONS (moved from WebServer) =====

void initAlertSystem() {
    SensorPrefs.begin("sensor-config", true);
    email = SensorPrefs.getString("email", "");
    emailKey = SensorPrefs.getString("emailKey", "");
    thresholdTemp = SensorPrefs.getFloat("temp", 30.0);
    thresholdHumid = SensorPrefs.getFloat("humid", 80.0);
    thresholdLux = SensorPrefs.getFloat("lux", 400.0);
    thresholdSoli = SensorPrefs.getFloat("soli", 300.0);
    thresholdDistance = SensorPrefs.getFloat("distance", 50.0);
    opTemp = SensorPrefs.getString("tempOp", ">");
    opHumid = SensorPrefs.getString("humidOp", ">");
    opLux = SensorPrefs.getString("luxOp", ">");
    opSoli = SensorPrefs.getString("soliOp", ">");
    opDistance = SensorPrefs.getString("distanceOp", "<");
    SensorPrefs.end();

    Serial.println("== Alert System Initialized ==");
    Serial.println("Email: " + email);
    Serial.printf("Ngưỡng nhiệt độ: %s %.2f°C\n", opTemp.c_str(), thresholdTemp);
    Serial.printf("Ngưỡng độ ẩm: %s %.2f%%\n", opHumid.c_str(), thresholdHumid);
    Serial.printf("Ngưỡng ánh sáng: %s %.2f lux\n", opLux.c_str(), thresholdLux);
    Serial.printf("Ngưỡng độ ẩm đất: %s %.2f\n", opSoli.c_str(), thresholdSoli);
    Serial.printf("Ngưỡng khoảng cách: %s %.2f cm\n", opDistance.c_str(), thresholdDistance);
}

bool compareThreshold(float value, String op, float threshold) {
    if (op == ">")  return value > threshold;
    if (op == ">=") return value >= threshold;
    if (op == "<")  return value < threshold;
    if (op == "<=") return value <= threshold;
    if (op == "==") return value == threshold;
    if (op == "!=") return value != threshold;
    return false; // Không khớp toán tử nào
}

void sendAlertEmail(float temp, float humid, float soli, float distance, float lux, String reason) {
    if (email.length() > 5) {
        unsigned long now = millis();
        if (now - lastMailTime > MAIL_INTERVAL || lastMailTime == 0) {
            StaticJsonDocument<1024> doc;
            doc["device"] = NAME_DEVICE;
            doc["temp"] = temp;
            doc["humid"] = humid;
            doc["lux"] = lux;
            doc["soli"] = soli;
            doc["distance"] = distance;
            doc["message"] = reason;
            String alertContent;
            serializeJson(doc, alertContent);
            sendMail(alertContent, email, emailKey);
            lastMailTime = now;
            
            Serial.println("=== Alert Email Sent ===");
            Serial.println("Reason: " + reason);
            Serial.println("Email: " + email);
        }
    }
}

void checkAndSendAlerts(float temp, float humid, float soli, float distance, float lux) {
    bool shouldAlert = false;
    String reason = "";

    if (compareThreshold(temp, opTemp, thresholdTemp)) {
        shouldAlert = true;
        reason += "Nhiệt độ vi phạm ngưỡng (" + String(temp) + "°C " + opTemp + " " + String(thresholdTemp) + "°C)<br>";
    }

    if (compareThreshold(humid, opHumid, thresholdHumid)) {
        shouldAlert = true;
        reason += "Độ ẩm vi phạm ngưỡng (" + String(humid) + "% " + opHumid + " " + String(thresholdHumid) + "%)<br>";
    }

    if (compareThreshold(lux, opLux, thresholdLux)) {
        shouldAlert = true;
        reason += "Ánh sáng vi phạm ngưỡng (" + String(lux) + " lux " + opLux + " " + String(thresholdLux) + " lux)<br>";
    }

    if (compareThreshold(soli, opSoli, thresholdSoli)) {
        shouldAlert = true;
        reason += "Độ ẩm đất vi phạm ngưỡng (" + String(soli) + " " + opSoli + " " + String(thresholdSoli) + ")<br>";
    }

    if (compareThreshold(distance, opDistance, thresholdDistance)) {
        shouldAlert = true;
        reason += "Có vật cản gần (" + String(distance) + " cm " + opDistance + " " + String(thresholdDistance) + " cm)<br>";
    }

    if (shouldAlert) {
        sendAlertEmail(temp, humid, soli, distance, lux, reason);
    }
}