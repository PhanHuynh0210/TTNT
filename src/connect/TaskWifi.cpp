#include "TaskWifi.h"
#include <Preferences.h>

Preferences wifiPrefs;

unsigned long lastWifiAttempt = 0;
const unsigned long WIFI_RETRY_INTERVAL = 30000; 
bool wifiConnectionFailed = false; 

bool errorMessageShown = false;
bool initialAttemptDone = false;

bool attemptConnect(String ssid, String pass, bool forceDisconnect)
{
    if (forceDisconnect) {
        WiFi.disconnect(true);
        delay(1000);
    }

    WiFi.mode(WIFI_STA);
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

void InitWiFi() {
    if (!initialAttemptDone) {
        Serial.println("Initializing WiFi...");
    }

    wifiPrefs.begin("wifi-config", true);
    String savedSSID = wifiPrefs.getString("ssid", "");
    String savedPass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();

    if (savedSSID.length() > 0) {
        if (!initialAttemptDone) {
            Serial.println("Trying saved WiFi: " + savedSSID);
        }

        bool success = attemptConnect(savedSSID, savedPass, false);

        if (success) {
            errorMessageShown = false;
            wifiConnectionFailed = false;
            initialAttemptDone = true;
            return;
        } else {
            wifiConnectionFailed = true;

            if (!errorMessageShown) {
                Serial.println("\n Saved WiFi failed.");
                errorMessageShown = true;
            }
        }
    } else {
        wifiConnectionFailed = true;

        if (!errorMessageShown) {
            Serial.println("No saved WiFi found in Preferences.");
            errorMessageShown = true;
        }
    }

    if (!errorMessageShown || !initialAttemptDone) {
        Serial.println("WiFi initialization failed. Will start Access Point mode.");
        initialAttemptDone = true;
    }
}

bool Wifi_reconnect() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    Serial.println("\n Attempting WiFi reconnection...");

    wifiPrefs.begin("wifi-config", true);
    String savedSSID = wifiPrefs.getString("ssid", "");
    String savedPass = wifiPrefs.getString("pass", "");
    wifiPrefs.end();

    if (savedSSID.length() > 0) {
        Serial.println("Trying to reconnect to: " + savedSSID);

        bool success = attemptConnect(savedSSID, savedPass, true);

        if (success) {
            return true;
        } else {
            Serial.println("Reconnection failed.");
        }
    } else {
        Serial.println("No saved WiFi credentials found.");
    }

    return false;
}

void clearWiFiSettings() {
    Serial.println("Clearing saved WiFi settings...");
    
    wifiPrefs.begin("wifi-config", false);
    wifiPrefs.clear(); 
    wifiPrefs.end();
    
    Serial.println("WiFi settings cleared successfully!");
    Serial.println("Device will restart in AP mode for new configuration.");
    
    errorMessageShown = false;
    initialAttemptDone = false;
    wifiConnectionFailed = false;
    
    WiFi.disconnect(true);
    delay(1000);
    ESP.restart();
}
