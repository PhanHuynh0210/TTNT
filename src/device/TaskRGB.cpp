#include "TaskRGB.h"

#define PIN_NEO_PIXEL  6   
#define NUM_PIXELS     4  

String outputRGBState = "off";

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

void TaskRGB(void *pvParameters)
{
    NeoPixel.begin();
    NeoPixel.clear();
    NeoPixel.show();
    while (1)
    {
        setRGB();
    }
}

void setRGB()
{
    if (outputRGBState == "on")
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

        for (int i = 0; i < numColors; i++) {
            for (int j = 0; j < NUM_PIXELS; j++) {
                NeoPixel.setPixelColor(j, colors[i]);
            }
            NeoPixel.show(); 
            for (int d = 0; d < 2000; d += 100) {  
                if (outputRGBState != "on") break;
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            NeoPixel.clear();
            NeoPixel.show();
}

    }
    else
    {
        NeoPixel.clear();
        NeoPixel.show();
    }
}


void handleGPIOCOn() {
  outputRGBState = "on";
  handleRoot();
}
void handleGPIOCOff() {
  outputRGBState = "off";
  handleRoot();
}

void initRGB()
{
    xTaskCreate(TaskRGB, "TaskRGB", 4096, NULL, 1, NULL);
}