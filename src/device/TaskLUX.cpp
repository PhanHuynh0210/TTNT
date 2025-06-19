#include "TaskLUX.h"

#define PinLux 2

void TaskLUX(void *pvParameters)
{

    while (1)
    {
        getValueLux();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

int getValueLux() {
    int luxValue = analogRead(PinLux); 
    publishData("LUX", String(luxValue)); 
    return luxValue;
}

void initLUX()
{
    xTaskCreate(TaskLUX,"TaskLUX", 4096 ,NULL , 1, NULL);
}