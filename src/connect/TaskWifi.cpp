#include "TaskWifi.h"
#include <Preferences.h>

Preferences wifiPrefs;

void InitWiFi()
{
    Serial.println("Initializing WiFi...");
    
    wifiPrefs.begin("wifi-config", true); 
    String savedSSID = wifiPrefs.getString("ssid", "");
    String savedPass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();

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
            return;
        } else {
            Serial.println("\nSaved WiFi failed.");
        }
    } else {
        Serial.println("No saved WiFi found in Preferences.");
    }
    
    Serial.println("WiFi initialization failed. Will start Access Point mode.");
}

bool Wifi_reconnect()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    InitWiFi();
    return true;
}
