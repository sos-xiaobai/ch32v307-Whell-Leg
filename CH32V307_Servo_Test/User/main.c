/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2024/1/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 *@Note
 *single wire half duplex mode, master/slave mode transceiver routine:
 *Master:USART2_Tx(PA2)
 *Slave:USART3_Tx(PB10)
 *
 *This example demonstrates UART2 and USART3 single-wire half-duplex
 *mode data transmission and reception.
 *
 *   Hardware connection:PA2 -- PB10
 * Note: The pin should be GPIO_Mode_AF_OD in single-wire half-duplex mode.
 *       The pin needs to connected a pull_up resistor
 */

#include "debug.h"
#include "uart.h"
#include "SMS_STS.h"
/* Global typedef */
/*********************************************************************
 * @fn      DMA_INIT
 *
 * @brief   Configures the DMA for USART2 & USART3.
 *
 * @return  none
 */
extern void setup();
extern void loop();


int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
    setup();

    while(1)
    {
        loop();
    }


}
