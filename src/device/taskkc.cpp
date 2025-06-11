#include "taskkc.h"

const int trig = 18;     // chân trig của HC-SR04
const int echo = 21;     // chân echo của HC-SR04void test() 

void test(){
    unsigned long duration; // biến đo thời gian
    int distance;           // biến lưu khoảng cách
    
    /* Phát xung từ chân trig */
    digitalWrite(trig,0);   // tắt chân trig
    delayMicroseconds(2);
    digitalWrite(trig,1);   // phát xung từ chân trig
    delayMicroseconds(5);   // xung có độ dài 5 microSeconds
    digitalWrite(trig,0);   // tắt chân trig
    
    /* Tính toán thời gian */
    // Đo độ rộng xung HIGH ở chân echo. 
    duration = pulseIn(echo,HIGH);  
    // Tính khoảng cách đến vật.
    distance = int(duration/2/29.412);
    publishData("khoangcach", String(distance));

    /* In kết quả ra Serial Monitor */
    Serial.print(distance);
    Serial.println("cm");
}
void kc(void *pvParameters)
{
    while (true)
    {
        test();
        vTaskDelay(delay_time / portTICK_PERIOD_MS);
    }
}

void initkc()
{
    pinMode(trig,OUTPUT);   // chân trig sẽ phát tín hiệu
    pinMode(echo,INPUT); 
    xTaskCreate(kc, "kc", 4096, NULL, 1, NULL);

}



