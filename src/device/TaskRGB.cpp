#include "TaskRGB.h"

 int PIN_NEO_PIXEL = 6;   
#define NUM_PIXELS     4  

String LedRGB = "off";
String rgbMode = "auto";
int customRed = 255;
int customGreen = 0;
int customBlue = 0;

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

int autoIndex = 0;
int rainbowIndex = 0;
unsigned long lastUpdate = 0;
const int autoDelay = 2000;
const int rainbowDelay = 50;

void TaskRGB(void *pvParameters)
{
    NeoPixel.begin();
    NeoPixel.clear();
    NeoPixel.show();

    while (1)
    {
        if (LedRGB == "on") {
            unsigned long now = millis();

            if (rgbMode == "auto" && now - lastUpdate > autoDelay) {
                handleAutoMode();
                lastUpdate = now;
            }
            else if (rgbMode == "rainbow" && now - lastUpdate > rainbowDelay) {
                handleRainbowMode();
                lastUpdate = now;
            }
            else if (rgbMode == "static") {
                handleStaticMode();
            }
        } else {
            NeoPixel.clear();
            NeoPixel.show();
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

void handleAutoMode()
{
    uint32_t colors[] = {
        NeoPixel.Color(255, 0, 0),     // Đỏ
        NeoPixel.Color(255, 69, 0),    // Cam
        NeoPixel.Color(255, 255, 0),   // Vàng
        NeoPixel.Color(0, 255, 0),     // Lục
        NeoPixel.Color(0, 0, 255),     // Lam
        NeoPixel.Color(75, 0, 130),    // Chàm
        NeoPixel.Color(148, 0, 211),   // Tím
        NeoPixel.Color(255, 255, 255), // Trắng
        NeoPixel.Color(0, 0, 0)        // Đen
    };

    const int numColors = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < NUM_PIXELS; i++) {
        NeoPixel.setPixelColor(i, colors[autoIndex]);
    }
    NeoPixel.show();

    autoIndex = (autoIndex + 1) % numColors;
}

void handleRainbowMode()
{
uint32_t colors[] = {
    NeoPixel.Color(255, 0, 0),     // Đỏ
    NeoPixel.Color(255, 165, 0),   // Cam  
    NeoPixel.Color(255, 255, 0),   // Vàng
    NeoPixel.Color(0, 255, 0),     // Lục
    NeoPixel.Color(0, 0, 255),     // Lam
    NeoPixel.Color(75, 0, 130),    // Chàm
    NeoPixel.Color(148, 0, 211)    // Tím 
    };


    const int numColors = sizeof(colors) / sizeof(colors[0]);

    for (int i = 0; i < NUM_PIXELS; i++) {
        NeoPixel.setPixelColor(i, colors[(rainbowIndex + i) % numColors]);
    }
    NeoPixel.show();

    rainbowIndex = (rainbowIndex + 1) % numColors;
}

void handleStaticMode()
{
    static int lastR = -1, lastG = -1, lastB = -1;

    if (customRed != lastR || customGreen != lastG || customBlue != lastB) {
        uint32_t color = NeoPixel.Color(customRed, customGreen, customBlue);
        for (int i = 0; i < NUM_PIXELS; i++) {
            NeoPixel.setPixelColor(i, color);
        }
        NeoPixel.show();

        lastR = customRed;
        lastG = customGreen;
        lastB = customBlue;
    }
}

void setRGBMode(String mode) {
  if (mode == "auto" || mode == "rainbow" || mode == "static") {
    rgbMode = mode;
    autoIndex = 0;
    rainbowIndex = 0;
    lastUpdate = 0;
  }
}

void setCustomColor(int r, int g, int b) {
  customRed = r;
  customGreen = g;
  customBlue = b;
}

void initRGB()
{
    xTaskCreate(TaskRGB, "TaskRGB", 4096, NULL, 1, NULL);
}
