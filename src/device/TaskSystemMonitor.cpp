#include "TaskSystemMonitor.h"
#include "../connect/TaskAccesspoint.h"

static bool lastSystemStatus = false;

void TaskSystemMonitor(void *pvParameters)
{
    while (1)
    {
        if (isAPMode()) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }
        
        bool currentWiFiStatus = isWiFiConnected();
        bool currentMQTTStatus = isMQTTConnected();
        bool currentGSheetStatus = isGSheetConnected();
        bool currentMailStatus = isMailConnected();
        
        bool currentSystemStatus = currentWiFiStatus && currentMQTTStatus && 
                                   currentGSheetStatus && currentMailStatus;
        
        if (currentSystemStatus != lastSystemStatus) {
            setStatus(currentSystemStatus ? STATUS_NORMAL : STATUS_ERROR);
            lastSystemStatus = currentSystemStatus;
        }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void initSystemMonitor()
{
    bool currentWiFiStatus = isWiFiConnected();
    bool currentMQTTStatus = isMQTTConnected();
    bool currentGSheetStatus = isGSheetConnected();
    bool currentMailStatus = isMailConnected();
    
    lastSystemStatus = currentWiFiStatus && currentMQTTStatus && 
                      currentGSheetStatus && currentMailStatus;
    
    xTaskCreate(TaskSystemMonitor, "TaskSystemMonitor", 4096, NULL, 1, NULL);
} 