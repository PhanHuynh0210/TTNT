#include "TaskWifi.h"

Preferences wifiPrefs;

bool attemptConnect(String ssid, String pass, bool forceDisconnect)
{
    WiFi.begin(ssid.c_str(), pass.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Serial.print("."); 
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n Connected to WiFi!");
        Serial.println("IP Address: " + WiFi.localIP().toString());
        return true;
    } else {
        Serial.println("\n Connection failed.");
        return false;
    }
}

void saveWiFi(String ssid, String pass) {
    wifiPrefs.begin("wifi-config", false);
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("pass", pass);
    wifiPrefs.end();
    Serial.println("WiFi settings saved successfully!");
}

String getSSID() {
    wifiPrefs.begin("wifi-config", true);
    String ssid = wifiPrefs.getString("ssid", "");
    wifiPrefs.end();
    return ssid;
}

String getPassword() {
    wifiPrefs.begin("wifi-config", true);
    String pass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();
    return pass;
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void InitWiFi() {
    WiFi.mode(WIFI_AP_STA);
    String savedSSID = getSSID();
    String savedPass = getPassword();
    if (savedSSID.length() > 0) {
        Serial.println("Trying saved WiFi: " + savedSSID);
        bool success = attemptConnect(savedSSID, savedPass, false);
        if (success) {
            return;
        } else {
            Serial.println("\n Saved WiFi failed.");
        }
    } else {
        Serial.println("No saved WiFi found in Preferences.");
    }
    
    Serial.println("WiFi initialization failed. Access Point mode available.");
}

bool Wifi_reconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    InitWiFi();

    return false;
}

void clearWiFiSettings() {    
    wifiPrefs.begin("wifi-config", false);
    wifiPrefs.clear(); 
    wifiPrefs.end();
    WiFi.disconnect(true);
    delay(1000);
    ESP.restart();
}
