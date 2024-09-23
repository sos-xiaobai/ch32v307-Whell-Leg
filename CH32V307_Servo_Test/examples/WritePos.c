/*
��ͨд����
*/

#include "ch32v30x.h"
#include "SCServo.h"
#include "uart.h"
#include "debug.h"

void setup(void)
{
    Uart_Init(1000000);
    Delay_Ms(1000);
}
int target_pos = 0;
int Pos;
int Speed;
void loop(void)
{
    WritePosEx(2, target_pos, 200, 20); // ���(ID1),������ٶ�V=2250��/��,���ٶ�A=50(50*100��/��^2),������P1=4095
    Delay_Ms(2);
    WritePosEx(1, 1023-target_pos, 200, 20); // ���(ID1),������ٶ�V=2250��/��,���ٶ�A=50(50*100��/��^2),������P1=4095
//    Delay_Ms(20);
//    Pos = ReadPos(2);
//    Speed = ReadSpeed(2);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
//    printf("Pos:%d,Speed:%d\r\n", Pos,Speed);
}

void Leg_Control_Setup(void)
{
    Uart_Init(1000000);
    Delay_Ms(1000);
}

void Leg_Control_Loop(void)
{
    WritePosEx(2, target_pos, 200, 20);
    WritePosEx(1, 1023-target_pos, 200, 20);
}
