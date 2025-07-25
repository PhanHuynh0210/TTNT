#include "TaskES35.h"


void getValueES35()
{
  float temperature, humidity;
  float sound = 0, pressure = 0, pm2p5 = 0, light = 0, pm10 = 0;
    byte soundRequest[] = {0x15, 0x03, 0x01, 0xF6, 0x00, 0x01, 0x66, 0xD0};
    byte PressureRequest[] = {0x15, 0x03, 0x01, 0xF9, 0x00, 0x01, 0x56, 0xD3};
    byte pm2p5Request[] = {0x15, 0x03, 0x01, 0xF7, 0x00, 0x01, 0x37, 0x10};
    byte PM10Request[] = {0x15, 0x03, 0x01, 0xF8, 0x00, 0x01, 0x07, 0x13};
    byte AmbientLightRequest[] = {0x15, 0x03, 0x01, 0xFB, 0x00, 0x01, 0xF7, 0x13};
    byte temperatureRequest[] = {0x15, 0x03, 0x01, 0xF5, 0x00, 0x01, 0x96, 0xD0};
    byte humidityRequest[] = {0x15, 0x03, 0x01, 0xF4, 0x00, 0x01, 0xC7, 0x10};
    byte response[7];

// Đọc áp suất
    memset(response, 0, sizeof(response));
    sendRS485Command(PressureRequest, sizeof(PressureRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pressure = (response[3] << 8) | response[4];
        pressure /= 10.0;
    }

    // am thanh
    memset(response, 0, sizeof(response));
    sendRS485Command(soundRequest, sizeof(soundRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        sound = (response[3] << 8) | response[4];
        sound /= 10.0;
    }

    // pm2p5
    memset(response, 0, sizeof(response));
    sendRS485Command(pm2p5Request, sizeof(pm2p5Request), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pm2p5 = (response[3] << 8) | response[4];
        pm2p5 /= 10.0;
    }

    //light
    memset(response, 0, sizeof(response));
    sendRS485Command(AmbientLightRequest, sizeof(AmbientLightRequest), response, sizeof(response));
    if (response[1] == 0x03)
    {
        light = (response[3] << 8) | response[4];
        light /= 10.0;
    }

    // p10
    memset(response, 0, sizeof(response));
    sendRS485Command(PM10Request, sizeof(PM10Request), response, sizeof(response));
    if (response[1] == 0x03)
    {
        pm10 = (response[3] << 8) | response[4];
        pm10 /= 10.0;
    }


  memset(response, 0, sizeof(response));
  sendRS485Command(temperatureRequest, sizeof(temperatureRequest), response, sizeof(response));
  if (response[1] == 0x03)
  {
    temperature = (response[3] << 8) | response[4];
    temperature /= 10.0;
  }

  delay(2000);
  memset(response, 0, sizeof(response));
  sendRS485Command(humidityRequest, sizeof(humidityRequest), response, sizeof(response));
  if (response[1] == 0x03)
  {
    humidity = (response[3] << 8) | response[4];
    humidity /= 10.0;
  }

    Serial.println("Temperature: " + String(temperature, 1) + "°C, Humidity: " + String(humidity, 1) + "%");
    Serial.println("Pressure: " + String(pressure, 1) + " hPa, Light: " + String(light, 1) + " lux");
    Serial.println("PM2.5: " + String(pm2p5, 1) + " µg/m3, PM10: " + String(pm10, 1) + " µg/m3");
    Serial.println("Sound Level: " + String(sound, 1) + " dB");
    // publishData("ES35_Temperature", String(temperature));
    // publishData("ES35_Humidity", String(humidity));


}

void TaskES35Sensor(void *pvParameters)
{
    while (true)
    {
        getValueES35();
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void initES35()
{
    RS485Serial.begin(BAUD_RATE_2, SERIAL_8N1, RXD_RS485, TXD_RS485);
    xTaskCreate(TaskES35Sensor, "TaskES35Sensor", 4096, NULL, 1, NULL);
    Serial.println("ES35 RS485 Sensor initialized");
} 