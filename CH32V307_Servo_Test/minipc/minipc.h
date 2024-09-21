#ifndef MINIPC_H
#define MINIPC_H

#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MINIPC_START_BYTE    0x0A
#define MINIPC_END_BYTE1 0x0A
#define MINIPC_END_BYTE2 0x0D
#define MINIPC_PACKET_LENGTH 7
#define USART8_BAUD          115200

typedef struct {
    bool is_turn_left; // true for left, false for right
    uint8_t turn_angle;
    bool is_squat; // true for squat, false for stand
} MiniPCCommand;

// 公共函数声明
void MiniPC_UART_IdleInterruptHandler(void);
void MiniPC_UART_RxCpltCallback(uint8_t received_byte);
void Minipc_Init(void);
void UART8_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
#ifdef __cplusplus
}
#endif

#endif // MINIPC_H
