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
        updateSystemStatus();
        return true;
    } else {
        return false;
    }
}

void saveWiFi(String ssid, String pass) {
    wifiPrefs.begin("wifi-config", false);
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("pass", pass);
    wifiPrefs.end();
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
        bool success = attemptConnect(savedSSID, savedPass, false);
        if (success) {
            return;
        } 
    } 
    
}


