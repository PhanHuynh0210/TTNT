#include "TaskLCD.h"

#define MY_SCL 11
#define MY_SDA 12

LiquidCrystal_I2C lcd(33,16,2);
int adcPins[] = {1, 2, 3, 4};

void Task_LCDDHT(void *pvParameters){
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
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        xTaskCreate(Task_LCDADC, "Task_ShowADC", 2048, NULL, 1, NULL);
        vTaskDelete(NULL);

}

void Task_LCDADC(void *pvParameters){
    char temp[16];
     lcd.clear();
        for(int i = 0; i <= 3; i++){
            int adc = analogRead(adcPins[i]);
            lcd.setCursor(0, i % 2);
            snprintf(temp, sizeof(temp), "ADC%d: %4d", i, adc);
            lcd.print(temp);

            if (i == 1) {
            vTaskDelay(2500 / portTICK_PERIOD_MS);
            lcd.clear();
            }
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);
        xTaskCreate(TaskHumid, "TaskHumid", 4096, NULL, 1, NULL);
        vTaskDelete(NULL);


}

void initLCD(){
    Wire.begin(MY_SCL,MY_SDA);
    lcd.init();
    lcd.backlight();
    xTaskCreate(Task_LCDDHT, "Task_LCDDHT", 2048, NULL, 1, NULL);
}

