
#include "TaskMQTT.h"

Preferences mqttPrefs;

String MQTT_SERVER = "io.adafruit.com";
int MQTT_PORT = 1883;
String IO_USERNAME = "";
String IO_KEY = "";

WiFiClient espClient;
PubSubClient client(espClient);

void loadMQTTSettings() {
    mqttPrefs.begin("mqtt-config", true);
    
    MQTT_SERVER = mqttPrefs.getString("server", "io.adafruit.com");
    IO_USERNAME = mqttPrefs.getString("username", "");
    IO_KEY = mqttPrefs.getString("key", "");
    
    mqttPrefs.end();

    Serial.println("MQTT Settings loaded:");
    Serial.println("Server: " + MQTT_SERVER);
    Serial.println("Username: " + IO_USERNAME);
    Serial.println(String("Key: ") + (IO_KEY.length() > 0 ? "****" : "empty"));
}

void saveMQTTSettings(String server, String username, String key) {
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
    
    MQTT_SERVER = "io.adafruit.com";
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
    if (strcmp(topic, (String(IO_USERNAME) + "/feeds/feed_2").c_str()) == 0)
    {
        Serial.println(message);
    }
    else if (strcmp(topic, (String(IO_USERNAME) + "/feeds/feed_3").c_str()) == 0)
    {
        Serial.println(message);
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
    // Kiểm tra WiFi connection trước khi kết nối MQTT
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Cannot connect to MQTT.");
        return;
    }
    
    // Kiểm tra có đầy đủ thông tin MQTT không
    if (IO_USERNAME.length() == 0 || IO_KEY.length() == 0) {
        Serial.println("MQTT credentials not set. Cannot connect to MQTT.");
        return;
    }
    
    Serial.println("Connecting to MQTT...");
    String clientId = "ESP32Client" + String(random(0, 1000));
    if (client.connect(clientId.c_str(), IO_USERNAME.c_str(), IO_KEY.c_str()))
    {
        Serial.println("MQTT Connected");
        client.subscribe((String(IO_USERNAME) + "/feeds/feed_2").c_str());
        client.subscribe((String(IO_USERNAME) + "/feeds/feed_3").c_str());
        publishData("wifi", WiFi.localIP().toString() );

    }
    else
    {
        Serial.print("MQTT connection failed, rc=");
        Serial.println(client.state());
        delay(1000);
    }
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