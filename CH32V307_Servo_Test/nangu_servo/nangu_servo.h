#include "ch32v30x.h"

// 定义舵机通道枚举
typedef enum {
    SERVO_CHANNEL_1 = 1,
    SERVO_CHANNEL_2,
    SERVO_CHANNEL_3,
    SERVO_CHANNEL_4
} ServoChannel;

void nangu_servo_init(void);
void set_servo_angle(ServoChannel channel, float angle);
