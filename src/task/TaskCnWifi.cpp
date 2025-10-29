#include "TaskCnWifi.h"

void initConnect(){
      if(WiFi.status() == WL_CONNECTED){
        initGGsheet();
        initMQTT();    
  }
}