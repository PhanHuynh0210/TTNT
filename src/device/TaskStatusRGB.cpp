#include "TaskStatusRGB.h"

#define STATUS_RGB_PIN 45
#define STATUS_NUM_PIXELS 1

Adafruit_NeoPixel statusNeoPixel(STATUS_NUM_PIXELS, STATUS_RGB_PIN, NEO_GRB + NEO_KHZ800);

SystemStatus currentStatus = STATUS_BOOTING;
static bool ledState = false;
static unsigned long lastBlinkTime = 0;
static unsigned long bootStartTime = 0;
static unsigned long connectStartTime = 0;

const uint32_t statusColors[] = {
    0xFFFF00,  // STATUS_BOOTING - Vàng
    0xFF00FF,  // STATUS_CONNECTING - Tím
    0x00FF00,  // STATUS_NORMAL - Xanh lá
    0xF00000,  // STATUS_ERROR - Đỏ
    0x0000FF,  // STATUS_AP_MODE - Xanh dương
    0xffffff   // STATUS_OTA_UPDATE - Trắng
};

const uint16_t blinkIntervals[] = {500, 300, 1000, 200, 1000, 300};

void TaskStatusRGB(void *pvParameters)
{
    // Khởi tạo NeoPixel
    statusNeoPixel.begin();
    statusNeoPixel.clear();
    statusNeoPixel.show();
    
    bootStartTime = millis();
    
    while (1)
    {
        unsigned long currentTime = millis();
        uint32_t color = statusColors[currentStatus];
        uint16_t interval = blinkIntervals[currentStatus];
        
        bool shouldBeOn = true;
        if (interval > 0) {
            shouldBeOn = ((currentTime - lastBlinkTime) % interval) < (interval / 2);
        }
        
        if (shouldBeOn != ledState) {
            statusNeoPixel.setPixelColor(0, shouldBeOn ? color : 0);
            statusNeoPixel.show();
            ledState = shouldBeOn;
        }
        
        if (interval > 0 && currentTime - lastBlinkTime >= interval) {
            lastBlinkTime = currentTime;
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}


void setStatus(SystemStatus status)
{
    if (currentStatus != status) {
        currentStatus = status;
        lastBlinkTime = millis();
        ledState = false;
        if (status == STATUS_BOOTING) {
            bootStartTime = millis();
        }
        if (status == STATUS_CONNECTING) {
            connectStartTime = millis();
        }
    }
}

void updateSystemStatus() {
    bool wifiConnected = isWiFiConnected();
    bool mqttConnected = isMQTTConnected();
    
    if (currentStatus == STATUS_AP_MODE) {
        return;
    }
    
    // BOOTING: chỉ kiểm tra nút BOOT trong 10s, sau đó chuyển sang CONNECTING
    if (currentStatus == STATUS_BOOTING) {
        if (checkBootButtonNonBlocking()) {
            if (!isAPMode()) {
                initAP();
            }
            setStatus(STATUS_AP_MODE);
            return;
        }
        if (millis() - bootStartTime >= 10000UL) {
            // Kết thúc giai đoạn boot -> bắt đầu kết nối WiFi/MQTT
            setStatus(STATUS_CONNECTING);
            InitWiFi();
            initMQTT();
            return;
        }
        return; // chỉ ở trong boot logic trong 10s
    }

    // CONNECTING: hiển thị đèn tím, chờ WiFi + MQTT. Thành công -> NORMAL, timeout -> ERROR
    if (currentStatus == STATUS_CONNECTING) {
        if (wifiConnected && mqttConnected) {
            setStatus(STATUS_NORMAL);
            initGGsheet();
            return;
        }
        // Cho thời gian kết nối tổng 20s
        if (millis() - connectStartTime >= 20000UL) {
            if (!(wifiConnected && mqttConnected)) {
                setStatus(STATUS_ERROR);
            }
            return;
        }
        return;
    }

    if (wifiConnected && mqttConnected) {
        setStatus(STATUS_NORMAL);
        return;
    }
    if (!wifiConnected || !mqttConnected) {
        setStatus(STATUS_ERROR);
        return;
    }
}

void initStatusRGB()
{
    xTaskCreate(TaskStatusRGB, "TaskStatusRGB", 4096, NULL, 2, NULL);
} 