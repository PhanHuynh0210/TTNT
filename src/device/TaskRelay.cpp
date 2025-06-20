#include "TaskRelay.h"

#define PinRelay 8

void TaskRelay(void *papvParameters){
    pinMode(PinRelay, OUTPUT);
    digitalWrite(PinRelay, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(PinRelay, LOW);
    xTaskCreate(TaskFan, "TaskFan", 2048, NULL, 1, NULL);
    vTaskDelete(NULL);
}
void initRelay(){
    xTaskCreate(TaskRelay, "TaskRelay", 4096, NULL, 1, NULL);
}


