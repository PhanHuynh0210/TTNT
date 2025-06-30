#include "TaskFan.h"

#define PinFan 10

void setFanSpeed(int speedPercent) {
    analogWrite(PinFan,speedPercent);
    handleRoot();
} 

void TaskFan(void *papvParameters){
    analogWrite(PinFan,100);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    analogWrite(PinFan, 0);
    xTaskCreate(TaskRelay, "TaskRelay", 4096, NULL, 1, NULL);
    vTaskDelete(NULL);

}
void initFan(){
    xTaskCreate(TaskFan, "TaskFan", 4096, NULL, 1, NULL);
}