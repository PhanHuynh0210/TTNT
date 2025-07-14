#include "TaskJXBS.h"

void getValueJXBS()
{
  float temperature = -1, humidity = -1;
  float soli = -1;
    byte request[] = {0x02, 0x03, 0x00, 0x12, 0x00, 0x02, 0x00, 0xCB}; // Thay 0x02
    byte soliRequest[] = {0x02, 0x03, 0x00, 0x12, 0x00, 0x01, 0x00, 0x00};
    byte response[7];
  

  memset(response, 0, sizeof(response));
  sendRS485Command(request, sizeof(request), response, sizeof(response));
    if (response[0] == 0x01 && response[1] == 0x03 && response[2] == 0x04)
    {
        uint16_t rawHumidity = (response[3] << 8) | response[4];
        int16_t rawTemperature = (int16_t)((response[5] << 8) | response[6]);

        humidity = rawHumidity / 10.0;
        temperature = rawTemperature / 10.0;
    }

  memset(response, 0, sizeof(response));
  sendRS485Command(soliRequest, sizeof(soliRequest), response, sizeof(response));
  if (response[1] == 0x03)
  {
    soli = (response[3] << 8) | response[4];
    soli /= 10.0;
  }

  Serial.println("Temperature: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");
  Serial.println("Soli: " + String(soli, 1) +  "%");
  publishData("Temperature", String(temperature));
  publishData("Humidity", String(humidity));
}

void TaskJXBSSensor(void *pvParameters)
{
    while (true)
    {
        getValueJXBS();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void initJXBS()
{
    RS485Serial.begin(BAUD_RATE_2, SERIAL_8N1, RXD_RS485, TXD_RS485);
    xTaskCreate(TaskJXBSSensor, "TaskJXBSSensor", 4096, NULL, 1, NULL);
    Serial.println("JXBS RS485 Sensor initialized");
} 