/*
Ping指令测试,测试总线上相应ID舵机是否就绪,广播指令只适用于总线只有一个舵机情况
*/

// #include <stdio.h>
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
    int ID = Ping(1);
    if (ID != -1) {
         printf("Servo ID:%d\n", ID);
        Delay_Ms(100);
    } else {
         printf("Ping servo ID error!\n");
        Delay_Ms(2000);
    }
}
