#ifndef		_UART_H
#define		_UART_H
//#define     USE_USART5_
#define USE_USART2_
#include <stdint.h>
extern void Uart_Init(uint32_t baudRate);
extern void Uart_Flush(void);
extern int16_t Uart_Read(void);
extern void Uart_Send(uint8_t *buf , uint8_t len);
void USART2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void UART5_IRQHandler(void)__attribute__((interrupt("WCH-Interrupt-fast")));
#endif
