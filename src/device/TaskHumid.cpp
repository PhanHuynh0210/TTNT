#include "TaskHumid.h"

#define PinHumid 1

int humidValue = 0;  // Lưu giá trị độ ẩm đất để dùng nếu cần

int getHumid() {
    int raw = analogRead(PinHumid);
    return map(raw, 0, 4095, 0, 100);  
}

void TaskHumid(void *pvParameters){
    while(1){
        getHumid();
    }
}

void initHumid(){
    xTaskCreate(TaskHumid, "TaskHumid", 4096, NULL, 1, NULL);
}
