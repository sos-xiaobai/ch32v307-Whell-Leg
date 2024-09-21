#include "minipc.h"
#include <string.h>
#include "ch32v30x.h"
typedef enum {
    MINIPC_PARSE_START,
    MINIPC_PARSE_CHECKSUM,
    MINIPC_PARSE_TURN_DIRECTION,
    MINIPC_PARSE_TURN_ANGLE,
    MINIPC_PARSE_POSTURE,
    MINIPC_PARSE_END1,
    MINIPC_PARSE_END2
} MiniPCParseState;

static uint8_t minipc_rx_buffer[MINIPC_PACKET_LENGTH];
static uint8_t minipc_rx_index             = 0;
static MiniPCParseState minipc_parse_state = MINIPC_PARSE_START;

// 私有函数声明
static uint8_t minipc_calculate_checksum(const uint8_t *data, uint8_t length);
static void minipc_execute_command(const MiniPCCommand *cmd);
static void USART_Configuration(uint32_t usart8_baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable USART4 and GPIOA clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART8, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* Configure USART4 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure USART4 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* USART configuration */
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

    /* Enable USART2 */
    USART_InitStructure.USART_BaudRate = usart8_baud;
    USART_Init(UART8, &USART_InitStructure);
    USART_Cmd(UART8, ENABLE);

    /* Enable USART2 IDLE line detection interrupt */
    USART_ITConfig(UART8, USART_IT_IDLE, ENABLE);

/* Enable USART2 Receive interrupt */
#if !ENABLE_USART_DMA
    USART_ITConfig(UART8, USART_IT_RXNE, ENABLE);
#endif

    /* NVIC configuration for USART2 interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = UART8_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void MiniPC_UART_IdleInterruptHandler(void)
{
    MiniPCCommand cmd;
    bool packet_valid = true;

    for (uint8_t i = 0; i < MINIPC_PACKET_LENGTH; i++) {
        uint8_t byte = minipc_rx_buffer[i];

        switch (minipc_parse_state) {
            case MINIPC_PARSE_START:
                if (byte != MINIPC_START_BYTE) {
                    packet_valid = false;
                }
                minipc_parse_state = MINIPC_PARSE_CHECKSUM;
                break;

            case MINIPC_PARSE_CHECKSUM:
                if (byte != minipc_calculate_checksum(minipc_rx_buffer + 2, 3)) {
                    packet_valid = false;
                }
                minipc_parse_state = MINIPC_PARSE_TURN_DIRECTION;
                break;

            case MINIPC_PARSE_TURN_DIRECTION:
                cmd.is_turn_left   = (byte == 0x00);
                minipc_parse_state = MINIPC_PARSE_TURN_ANGLE;
                break;

            case MINIPC_PARSE_TURN_ANGLE:
                cmd.turn_angle     = byte;
                minipc_parse_state = MINIPC_PARSE_POSTURE;
                break;

            case MINIPC_PARSE_POSTURE:
                cmd.is_squat       = (byte == 0x00);
                minipc_parse_state = MINIPC_PARSE_END1;
                break;

            case MINIPC_PARSE_END1:
                if (byte != MINIPC_END_BYTE1) {
                    packet_valid = false;
                }
                minipc_parse_state = MINIPC_PARSE_END2;
                break;

            case MINIPC_PARSE_END2:
                if (byte != MINIPC_END_BYTE2) {
                    packet_valid = false;
                }
                break;
        }

        if (!packet_valid) {
            break;
        }
    }

    if (packet_valid) {
        minipc_execute_command(&cmd);
    }

    // 重置解析状态
    minipc_parse_state = MINIPC_PARSE_START;
    minipc_rx_index    = 0;
    memset(minipc_rx_buffer, 0, MINIPC_PACKET_LENGTH);

    // UART_ClearIdleFlag();
}

void UART8_IRQHandler(void)
{
    if (USART_GetITStatus(UART8, USART_IT_IDLE) != RESET) {
        /* Clear IDLE line detected bit */
        USART_ReceiveData(UART8);

        MiniPC_UART_IdleInterruptHandler();
    }

    if (USART_GetITStatus(UART8, USART_IT_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData(UART8);
        MiniPC_UART_RxCpltCallback(ch);
    }
}

void Minipc_Init(void)
{
    USART_Configuration(USART8_BAUD);
}

void MiniPC_UART_RxCpltCallback(uint8_t received_byte)
{
    if (minipc_rx_index < MINIPC_PACKET_LENGTH) {
        minipc_rx_buffer[minipc_rx_index++] = received_byte;
    }
}

static uint8_t minipc_calculate_checksum(const uint8_t *data, uint8_t length)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum;
}

static void minipc_execute_command(const MiniPCCommand *cmd)
{
    // 这里实现执行命令的具体逻辑
    if (cmd->is_turn_left) {
        // 执行左转
        // minipc_turn_left(cmd->turn_angle);
    } else {
        // 执行右转
        // minipc_turn_right(cmd->turn_angle);
    }

    if (cmd->is_squat) {
        // 执行蹲下
        // minipc_squat();
    } else {
        // 执行站立
        // minipc_stand();
    }
}
