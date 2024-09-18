/*
 * uart.c
 * UART接口
 * 日期: 2019.9.7
 * 作者: 
 */

#include <stdio.h>
#include "uart.h"
#include "ch32v30x.h"
#ifndef __IO
#define __IO volatile
#endif
//UART 读数据缓冲区
__IO uint8_t uartBuf[128];
__IO int head = 0;
__IO int tail  = 0;

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
使用USE_USART1_ 宏定义
配置USART1，端口映射(TX)PA9/(RX)PA10
USART1作为舵机串口
------------------*/
#ifdef USE_USART1_
void Uart_Init(uint32_t baudRate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	//UART1 GPIO 配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;//PA9 UART1_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10 UART1_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	//UART 数据格式配置
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
	USART_InitStructure.USART_BaudRate = baudRate;
	USART_Init(USART1, &USART_InitStructure);
	
	//中断配置
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//打开UART使能开关
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
使用USE_USART2_宏定义
配置USART2，端口映射(TX)PA2/(RX)PA3
USART2作为舵机串口
------------------*/
#ifdef USE_USART2_
void Uart_Init(uint32_t baudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;
        NVIC_InitTypeDef NVIC_InitStructure;

        // 启用 GPIOA 和 USART2 时钟
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

        // 配置 USART2 Tx (PA2) 为推挽复用输出
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        // 配置 USART2
        USART_InitStructure.USART_BaudRate = baudRate;
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        USART_InitStructure.USART_Parity = USART_Parity_No;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_Init(USART2, &USART_InitStructure);

        // 启用半双工模式
        USART_HalfDuplexCmd(USART2, ENABLE);

        // 启用 USART2 接收中断
        USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

        // 配置 USART2 中断
        NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        // 启用 USART2
        USART_Cmd(USART2, ENABLE);
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
}

#endif
