/*
ͬ��д����
*/

#include "ch32v30x.h"
#include "SCServo.h"
#include "uart.h"
#include "wiring.h"

uint8_t ID[2];
int16_t Position[2];
uint16_t Speed[2];
uint8_t ACC[2];

void setup(void)
{
    Uart_Init(1000000);
    Delay_Ms(1000);
    ID[0] = 1;
    ID[1] = 2;
}

void loop(void)
{
    Speed[0]    = 1500;
    Speed[1]    = 1500;
    ACC[0]      = 50;
    ACC[1]      = 50;
    Position[0] = 4095;
    Position[1] = 4095;
    SyncWritePosEx(ID, 2, Position, Speed, ACC); // ���(ID1/ID2),������ٶ�V=1500��/��,,���ٶ�A=50(50*100��/��^2),������P1=4095
    Delay_Ms(2270);                              //((P1-P0)/V)*1000+(V/(A*100))*1000

    Position[0] = 0;
    Position[1] = 0;
    SyncWritePosEx(ID, 2, Position, Speed, ACC); // ���(ID1/ID2),������ٶ�V=1500��/��,���ٶ�A=50(50*100��/��^2),������P1=0
    Delay_Ms(2270);                              //((P1-P0)/V)*1000+(V/(A*100))*1000
}
