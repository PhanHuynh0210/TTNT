
#include "TaskMQTT.h"

Preferences mqttPrefs;

String MQTT_SERVER = "io.adafruit.com";
int MQTT_PORT = 1883;
String IO_USERNAME = "huynh0210";
String IO_KEY = "";

WiFiClient espClient;
PubSubClient client(espClient);

void loadMQTTSettings() {
    mqttPrefs.begin("mqtt-config", true);
    
    IO_USERNAME = mqttPrefs.getString("username", "");
    IO_KEY = mqttPrefs.getString("key", "");
    
    mqttPrefs.end();

    Serial.println("MQTT Settings loaded:");
    Serial.println("Username: " + IO_USERNAME);
    Serial.println(String("Key: ") + (IO_KEY.length() > 0 ? "****" : "empty"));
}

void saveMQTTSettings(String username, String key) {
    mqttPrefs.begin("mqtt-config", false);
    
    mqttPrefs.putString("username", username);
    mqttPrefs.putString("key", key);
    
    mqttPrefs.end();
    
    Serial.println("MQTT settings saved to flash memory.");
    
    IO_USERNAME = username;
    IO_KEY = key;
}

void clearMQTTSettings() {
    Serial.println("Clearing saved MQTT settings...");
    
    mqttPrefs.begin("mqtt-config", false);
    mqttPrefs.clear();
    mqttPrefs.end();
    
    Serial.println("MQTT settings cleared successfully!");
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
        InitMQTT();
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

bool isMQTTConnected() {
    return client.connected();
}