/*
中位校准例子
*/

#include "ch32v30x.h"
#include "SCServo.h"
#include "uart.h"
#include "debug.h"

void setup()
{
    Uart_Init(1000000);
    Delay_Ms(1000);
}

void loop()
{
    CalibrationOfs(1);
    while (1);
}
