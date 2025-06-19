#include "TaskHumid.h"

#define PinHumid 1

int humidValue = 0;  // Lưu giá trị độ ẩm đất để dùng nếu cần

int getHumid() {
    int raw = analogRead(PinHumid);
    return map(raw, 0, 4095, 0, 100);  
}

void TaskHumid(void *pvParameters){
    lcd.clear();
    humidValue = getHumid();  

    lcd.setCursor(0, 0);
    lcd.print("Soil Humid:");

    lcd.setCursor(13, 0);
    lcd.print(humidValue);
    lcd.print("%");

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    xTaskCreate(Task_LCDDHT, "Task_LCDDHT", 2048, NULL, 1, NULL);
    vTaskDelete(NULL);
}

void initHumid(){
    xTaskCreate(TaskHumid, "TaskHumid", 4096, NULL, 1, NULL);
}
