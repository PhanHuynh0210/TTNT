#include "taskkc.h"

const int trig = 18;   
const int echo = 21;   

int test(){
    unsigned long duration; 
    int distance;         
    
    digitalWrite(trig,0);  
    delayMicroseconds(2);
    digitalWrite(trig,1);   
    delayMicroseconds(5);
    digitalWrite(trig,0);   
    
    duration = pulseIn(echo,HIGH);  
    distance = int(duration/2/29.412);

    return distance;
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
    pinMode(trig,OUTPUT);   
    pinMode(echo,INPUT); 
    xTaskCreate(kc, "kc", 4096, NULL, 1, NULL);
}



