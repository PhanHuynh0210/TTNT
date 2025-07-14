#include "TaskWifi.h"

String WIFI_SSID = "ACLAB";
String WIFI_PASSWORD = "ACLAB2023";

Preferences wifiPrefs;

void InitWiFi()
{
    Serial.println("Initializing WiFi...");
    wifiPrefs.begin("wifi-config", true); 
    
    String savedSSID = wifiPrefs.getString("ssid", "");
    String savedPass = wifiPrefs.getString("pass", "");
    
    if (savedSSID.length() > 0) {
        Serial.println("Trying saved WiFi: " + savedSSID);
        WiFi.mode(WIFI_STA);
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
        
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
            delay(500);
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected to saved WiFi!");
            Serial.println("Station IP: " + WiFi.localIP().toString());
            wifiPrefs.end();
            return;
        } else {
            Serial.println("\nSaved WiFi failed, trying default...");
        }
    }
    
    Serial.println("Connecting to default WiFi: " + WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 15000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to default WiFi!");
        Serial.println("Station IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nDefault WiFi failed. Access Point will be started.");
    }
    
    wifiPrefs.end();
}

bool Wifi_reconnect()
{
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
    {
        return true;
    }
    
    Serial.println("WiFi disconnected, attempting reconnect...");
    
    wifiPrefs.begin("wifi-config", true);
    String savedSSID = wifiPrefs.getString("ssid", "");
    String savedPass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();
    
    if (savedSSID.length() > 0) {
        WiFi.begin(savedSSID.c_str(), savedPass.c_str());
        unsigned long startTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) {
            delay(500);
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nReconnected to saved WiFi!");
            return true;
        }
    }
    
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 5000) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nReconnected to default WiFi!");
        return true;
    }
    
    Serial.println("\nFailed to reconnect WiFi");
    return false;
}


