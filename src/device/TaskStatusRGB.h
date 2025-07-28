#ifndef INC_TASKSTATUSRGB_H_
#define INC_TASKSTATUSRGB_H_

#include "globals.h"

enum SystemStatus {
    STATUS_BOOTING,           // Khởi động
    STATUS_NORMAL,            // Hoạt động bình thường
    STATUS_ERROR,             // Lỗi kết nối
    STATUS_AP_MODE            // Chế độ Access Point
};

extern void TaskStatusRGB(void *pvParameters);
extern void initStatusRGB();
extern void setStatus(SystemStatus status);
extern SystemStatus currentStatus;
extern void updateSystemStatus(); // Function mới để kiểm tra trạng thái tổng thể

#endif /* INC_TASKSTATUSRGB_H_ */ 