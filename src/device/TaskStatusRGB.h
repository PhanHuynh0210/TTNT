#ifndef INC_TASKSTATUSRGB_H_
#define INC_TASKSTATUSRGB_H_

#include "globals.h"

enum SystemStatus {
    STATUS_BOOTING,           // Khởi động
    STATUS_NORMAL,            // Hoạt động bình thường
    STATUS_ERROR              // Lỗi kết nối
};

extern void TaskStatusRGB(void *pvParameters);
extern void initStatusRGB();
extern void setStatus(SystemStatus status);
extern SystemStatus currentStatus;

#endif /* INC_TASKSTATUSRGB_H_ */ 