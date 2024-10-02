/*
 * uart.c
 * UART�ӿ�
 * ����: 2019.9.7
 * ����: 
 */

#include <stdio.h>
#include "uart.h"
#include "ch32v30x.h"
#ifndef __IO
#define __IO volatile
#endif
//UART �����ݻ�����
__IO uint8_t uartBuf[128];
__IO int head = 0;
__IO int tail  = 0;

uint8_t *TxBuffer;

void Uart_Flush(void)
{
	head = tail = 0;
}

int16_t Uart_Read(void)
{
	if(head!=tail){
		uint8_t Data = uartBuf[head];
		head =  (head+1)%128;
		return Data;
	}else{
		return -1;
	}
}

/*---------------
ʹ��USE_USART1_ �궨��
����USART1���˿�ӳ��(TX)PA9/(RX)PA10
USART1��Ϊ�������
------------------*/
#ifdef USE_USART1_
void Uart_Init(uint32_t baudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	//UART1 GPIO ����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//PA9 UART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10 UART1_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	//UART ���ݸ�ʽ����
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	USART_InitStructure.USART_BaudRate = baudRate;
	USART_Init(USART1, &USART_InitStructure);
	
	//�ж�����
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//��UARTʹ�ܿ���
	USART_Cmd(USART1, ENABLE);
}

void USART1_IRQHandler(void)
{
	USART_ClearITPendingBit(USART1, USART_IT_ORE);
	uartBuf[tail] = USART_ReceiveData(USART1);
	tail = (tail+1)%128;
}

void Uart_Send(uint8_t *buf , uint8_t len)
{
	uint8_t i=0;
	for(i=0; i<len; i++){
		USART_SendData(USART1, buf[i]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
}

#endif

/*---------------
ʹ��USE_USART2_�궨��
����USART2���˿�ӳ��(TX)PA2/(RX)PA3
USART2��Ϊ�������
------------------*/
DMA_InitTypeDef DMA_InitStructure = {0};
#ifdef USE_USART2_
void Uart_Init(uint32_t baudRate)
{



    	GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        // ���� GPIOA �� USART2 ʱ��
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        // ���� USART2 Tx (PA2) Ϊ���츴�����
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        // ���� USART2
        USART_InitStructure.USART_BaudRate = baudRate;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(USART2, &USART_InitStructure);

        // ���ð�˫��ģʽ
        USART_HalfDuplexCmd(USART2, ENABLE);

        // ���� USART2 �����ж�
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

        // ���� USART2 �ж�
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);



        // ���� USART2
        USART_Cmd(USART2, ENABLE);


//
//        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//
//        DMA_DeInit(DMA1_Channel7);
//        DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DATAR); /* USART2->DATAR:0x40004404 */
//        DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TxBuffer;
//        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
//        DMA_InitStructure.DMA_BufferSize = 128;
//        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
//        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
//        DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//        DMA_Init(DMA1_Channel7, &DMA_InitStructure);
//        /*����dma����*/
//        DMA_Cmd(DMA1_Channel7, ENABLE); /* USART2 Tx */
//        USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
}

void USART2_IRQHandler(void)
{
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	uartBuf[tail] = USART_ReceiveData(USART2);
	tail = (tail+1)%128;
	NVIC_EnableIRQ(USART2_IRQn);


	
}

void Uart_Send(uint8_t *buf , uint8_t len)
{
   // USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);

   uint8_t i=0;
   for(i=0; i<len; i++){
       USART_SendData(USART2, buf[i]);
       while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
   }
   while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);

   // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

//    TxBuffer = buf;
//    DMA_DeInit(DMA1_Channel7);  //��ʼ��
//    DMA_Cmd(DMA1_Channel7, DISABLE);  //��ʧ�ܣģͣ�
//    DMA_InitStructure.DMA_BufferSize=len;  //���͵���������С
//    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
//    DMA_Cmd(DMA1_Channel7, ENABLE);  //��ʹ�� DMA
//    //while(DMA_GetFlagStatus(DMA1_FLAG_TC7) == RESET) {;};

}

#endif

/*---------------
ʹ��USE_USART5_�궨��
����USART5���˿�ӳ��(TX)PC12/(RX)PD2
USART5��Ϊ�������
------------------*/
#ifdef USE_USART5_
void Uart_Init(uint32_t baudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;


        // ���� GPIOC �� USART5 ʱ��
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

        // ���� USART5 Tx (PC12) Ϊ���츴�����
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOC, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;

        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOD, &GPIO_InitStructure);

        // ���� USART5
        USART_InitStructure.USART_BaudRate = baudRate;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(UART5, &USART_InitStructure);

        // ���ð�˫��ģʽ ֻ����Tx PC12
        USART_HalfDuplexCmd(UART5, ENABLE);

        // ���� USART5 �����ж�
        USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

        // ���� USART5 �ж�
        NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        // ���� USART5
        USART_Cmd(UART5, ENABLE);
}

void UART5_IRQHandler(void)
{
    USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    uartBuf[tail] = USART_ReceiveData(UART5);
    tail = (tail+1)%128;
    NVIC_EnableIRQ(UART5_IRQn);
}



void Uart_Send(uint8_t *buf , uint8_t len)
{
    USART_ITConfig(UART5, USART_IT_RXNE, DISABLE);

   uint8_t i=0;
   for(i=0; i<len; i++){
       USART_SendData(UART5, buf[i]);
       while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
   }
   while(USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET);

    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
}

#endif

