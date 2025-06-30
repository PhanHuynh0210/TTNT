#include "TaskLCD.h"

#define MY_SCL 11
#define MY_SDA 12

LiquidCrystal_I2C lcd(33, 16, 2);
int adcPins[] = {1, 2, 3, 4};

// Task 1: Hiển thị Nhiệt độ & Độ ẩm
void Task_LCDDHT(void *pvParameters) {
    unsigned long startTime = millis();
    char buffer[16];
    lcd.clear();

    while (millis() - startTime < 5000) {
        dht20.read();
        lcd.clear();
        dht20.read();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(String(dht20.getTemperature()));
        lcd.print(" C");

        lcd.setCursor(0, 1);
        lcd.print("Humi: ");
        lcd.print(dht20.getHumidity());
        lcd.print(" %");

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    xTaskCreate(Task_LCDADC, "Task_LCDADC", 4096, NULL, 1, NULL);
    vTaskDelete(NULL);
}

// Task 2: Hiển thị ADC
void Task_LCDADC(void *pvParameters) {
    unsigned long startTime = millis();
    char temp[16];
    lcd.clear();

    while (millis() - startTime < 5000) {
        for(int i = 0; i <= 3; i++){
            int adc = analogRead(adcPins[i]);
            lcd.setCursor(0, i % 2);
            snprintf(temp, sizeof(temp), "ADC%d: %4d", i, adc);
            lcd.print(temp);

            if (i == 1) {
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            lcd.clear();
            }
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);
        xTaskCreate(TaskDisLCD, "TaskDisLCD", 4096, NULL, 1, NULL);
        vTaskDelete(NULL);
        
    }

   
}

// Task 3: Hiển thị Khoảng cách
void TaskDisLCD(void *pvParameters) {
    unsigned long startTime = millis();
    lcd.clear();

    while (millis() - startTime < 5000) {
        int discante = test();
        char buffer[16];

        lcd.setCursor(0, 0);
        lcd.print("Distance:");

        lcd.setCursor(0, 1);
        snprintf(buffer, sizeof(buffer), "%d cm         ", discante);
        lcd.print(buffer);

        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // Quay lại Task đầu tiên
    xTaskCreate(Task_LCDDHT, "Task_LCDDHT", 4096, NULL, 1, NULL);
    vTaskDelete(NULL);
}

// Hàm khởi tạo LCD và task đầu tiên
void initLCD() {
    Wire.begin(MY_SCL, MY_SDA);
    lcd.init();
    lcd.backlight();
    xTaskCreate(Task_LCDDHT, "Task_LCDDHT", 4096, NULL, 1, NULL);
}
