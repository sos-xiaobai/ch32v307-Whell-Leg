/*
ͬ����֧��SMS/STS�����ͺŶ��
*/

// #include <stdio.h>
#include "ch32v30x.h"
#include "SCServo.h"
#include "uart.h"
#include "wiring.h"

int16_t Position;
int16_t Speed;
uint8_t ID[] = {1, 2};
uint8_t rxPacket[4];

void setup(void)
{
    Uart_Init(115200);
    syncReadBegin(sizeof(ID), sizeof(rxPacket), 1800 * 10); // 10ms
    Delay_Ms(1000);
}

void loop(void)
{
    uint8_t i;

    syncReadPacketTx(ID, sizeof(ID), SMS_STS_PRESENT_POSITION_L, sizeof(rxPacket)); // ͬ����ָ�������
    for (i = 0; i < sizeof(ID); i++) {
        // ����ID[i]ͬ�������ذ�
        if (!syncReadPacketRx(ID[i], rxPacket)) {
            // printf("ID:%d sync read error!\n", ID[i]);
            continue; // ���ս���ʧ��
        }
        Position = syncReadRxPacketToWrod(15); // ���������ֽ� bit15Ϊ����λ,����=0��ʾ�޷���λ
        Speed    = syncReadRxPacketToWrod(15); // ���������ֽ� bit15Ϊ����λ,����=0��ʾ�޷���λ
        // printf("ID:%d Position:%d Speed:%d ", ID[i], Position, Speed);
        Delay_Ms(10);
    }
    // printf("\n");
}
