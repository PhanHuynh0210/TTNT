#include "TaskDHT20.h"

#define MY_SCL 11
#define MY_SDA 12

DHT20 dht20;

void getValueDHT20()
{
    if (dht20.read() == DHT20_OK)
    {
        Serial.println(String(dht20.getTemperature()) + "-" + String(dht20.getHumidity()));
        String tempStr = String(dht20.getTemperature());
        String humidStr = String(dht20.getHumidity());
        client.publish("esp32/sensor/temp", tempStr.c_str());
        client.publish("esp32/sensor/humid", humidStr.c_str());

    }
    else
    {
        Serial.println("Failed to read DHT20 sensor.");
    }
}

 
void TaskTemperatureHumidity(void *pvParameters)
{
    while (true)
    {
        getValueDHT20();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}
void initDHT20()
{
    Wire.begin(MY_SCL, MY_SDA);
    dht20.begin();
    xTaskCreate(TaskTemperatureHumidity, "TaskTemperatureHumidity", 4096, NULL, 1, NULL);
}

