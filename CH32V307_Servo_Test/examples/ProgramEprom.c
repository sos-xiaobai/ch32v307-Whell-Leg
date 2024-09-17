/*
舵机参数编程
*/

#include "ch32v30x.h"
#include "SCServo.h"
#include "uart.h"
#include "wiring.h"

void setup()
{
    Uart_Init(1000000);
    Delay_Ms(1000);
    unLockEprom(1);              // 打开EPROM保存功能
    writeByte(1, SMS_STS_ID, 2); // ID
    LockEprom(2);                // 关闭EPROM保存功能
}

void loop()
{
}
