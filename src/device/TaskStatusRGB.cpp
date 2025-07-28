#include "TaskStatusRGB.h"

#define STATUS_RGB_PIN 45
#define STATUS_NUM_PIXELS 1

Adafruit_NeoPixel statusNeoPixel(STATUS_NUM_PIXELS, STATUS_RGB_PIN, NEO_GRB + NEO_KHZ800);

SystemStatus currentStatus = STATUS_BOOTING;
static bool ledState = false;
static unsigned long lastBlinkTime = 0;

const uint32_t statusColors[] = {
    0xFFFF00,  // STATUS_BOOTING - Vàng
    0x00FF00,  // STATUS_NORMAL - Xanh lá
    0xFF0000,  // STATUS_ERROR - Đỏ
    0x0000FF   // STATUS_AP_MODE - Xanh dương
};

const uint16_t blinkIntervals[] = {500, 0, 200, 1000};

void TaskStatusRGB(void *pvParameters)
{
    // Khởi tạo NeoPixel
    statusNeoPixel.begin();
    statusNeoPixel.clear();
    statusNeoPixel.show();
    
    
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
    }
}

void updateSystemStatus() {
    bool wifiConnected = isWiFiConnected();
    bool mqttConnected = isMQTTConnected();
    
    if (currentStatus == STATUS_AP_MODE) {
        return;
    }
    
    // Nếu cả WiFi và MQTT đều kết nối thành công
    if (wifiConnected && mqttConnected) {
        setStatus(STATUS_NORMAL);
    }
    // Nếu WiFi kết nối nhưng MQTT chưa kết nối
    else if (wifiConnected && !mqttConnected) {
        // Giữ nguyên trạng thái hiện tại (có thể là BOOTING hoặc ERROR)
        // Không chuyển sang NORMAL cho đến khi MQTT cũng kết nối
    }
    // Nếu WiFi không kết nối
    else if (!wifiConnected) {
        setStatus(STATUS_ERROR);
    }
}

void initStatusRGB()
{
    xTaskCreate(TaskStatusRGB, "TaskStatusRGB", 4096, NULL, 2, NULL);
} 