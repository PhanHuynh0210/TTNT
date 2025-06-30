#include "TaskES35.h"

#define BAUD_RATE_2 9600
#define RXD_RS485 17  // Chân RX cho RS485 - thay đổi theo board của bạn
#define TXD_RS485 10  // Chân TX cho RS485 - thay đổi theo board của bạn

HardwareSerial RS485Serial(1);

void sendRS485Command(byte *command, int commandSize, byte *response, int responseSize)
{
    RS485Serial.write(command, commandSize);
    RS485Serial.flush();
    delay(100);
    if (RS485Serial.available() >= responseSize)
    {
        RS485Serial.readBytes(response, responseSize);
    }
}

void getValueES35()
{
    /*Đọc cảm biến nhiệt độ độ ẩm ES35*/
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

    // In ra Serial và publish qua MQTT
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
    // Khởi tạo RS485 Serial
    RS485Serial.begin(BAUD_RATE_2, SERIAL_8N1, RXD_RS485, TXD_RS485);
    
    // Tạo task cho cảm biến ES35
    xTaskCreate(TaskES35Sensor, "TaskES35Sensor", 4096, NULL, 1, NULL);
    
    Serial.println("ES35 RS485 Sensor initialized");
} 