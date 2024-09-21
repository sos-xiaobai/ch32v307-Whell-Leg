
/*
 * Copyright (c) 2006-2024, HiPNUC
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "hipnuc.h"
#include "ch32v30x.h"
/* The driver file for decoding HiPNUC protocol, DO NOT MODIFTY*/

/* legcy support of HI226/HI229 */
#define HIPNUC_ID_USRID         (0x90)
#define HIPNUC_ID_ACC_RAW       (0xA0)
#define HIPNUC_ID_ACC_CAL       (0xA1)
#define HIPNUC_ID_GYR_RAW       (0xB0)
#define HIPNUC_ID_GYR_CAL       (0xB1)
#define HIPNUC_ID_MAG_RAW       (0xC0)
#define HIPNUC_ID_EUL           (0xD0)
#define HIPNUC_ID_QUAT          (0xD1)
#define HIPNUC_ID_PRS           (0xF0)

/* new HiPNUC standard packet */
#define HIPNUC_ID_IMUSOL        (0x91)
#define HIPNUC_ID_IMUBIN        (0x92)
#define HIPNUC_ID_INSSOL        (0x81)

#ifndef D2R
#define D2R (0.0174532925199433F)
#endif

#ifndef R2D
#define R2D (57.2957795130823F)
#endif

#ifndef GRAVITY
#define GRAVITY (9.8F)
#endif

/* Enable/Disable fixed data array decoding example */
#define ENABLE_FIXED_DATA_EXAMPLE 0

/* Enable/Disable DMA for USART reception */
#define ENABLE_USART_DMA 0

#define USART4_BAUD      115200

#define UART_RX_BUF_SIZE (1024)
#define LOG_STRING_SIZE  (512)

/* IMU stream read/control struct */
static hipnuc_raw_t hipnuc_raw = {0};

/* 0: no new data arrived, 1: new data arrived */
static uint8_t new_data_flag = 0;

/* The char buffer used to show result */
static char log_buf[LOG_STRING_SIZE];

static uint8_t uart_rx_buf[UART_RX_BUF_SIZE];
static uint8_t dma_rx_buf[UART_RX_BUF_SIZE];
static uint16_t uart_rx_index = 0;

/* Function prototypes */
static void USART_Configuration(uint32_t usart4_baud);
static void DMA_Configuration(void);
static void handle_usart_rx_idle(void);

static void hipnuc_crc16(uint16_t *inital, const uint8_t *buf, uint32_t len);

/**
 * @brief Print system clock frequencies and reception mode
 *
 * Prints the system clock frequencies and the USART reception mode (DMA or UART Interrupt) to the console.
 */
void printf_welcome_information(void)
{
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);

    printf("HiPNUC IMU data decode example\r\n");
#if ENABLE_USART_DMA
    printf("USART reception: DMA\r\n");
#else
    printf("USART reception: UART Interrupt\r\n");
#endif

    printf("System Clock Frequencies:\r\n");
    printf("SYSCLK: %d Hz\r\n", RCC_Clocks.SYSCLK_Frequency);
    printf("HCLK: %d Hz\r\n", RCC_Clocks.HCLK_Frequency);
}

void app_init(void)
{
    /* Initialize USART1 for debug output */
    /* Initialize USART2 for IMU data reception */
    USART_Configuration(USART4_BAUD);

#if ENABLE_USART_DMA
    /* Initialize DMA for USART2 reception */
    DMA_Configuration();
#endif

    /* Initialize instance */
    memset(&hipnuc_raw, 0, sizeof(hipnuc_raw_t));
    new_data_flag = 0;
}

/**
 * @brief Process IMU data
 *
 * If new data is available, processes the received IMU data by decoding it and printing the results.
 */
void process_data(void)
{
    if (new_data_flag) {
        new_data_flag = 0;

        for (uint16_t i = 0; i < uart_rx_index; i++) {
            if (hipnuc_input(&hipnuc_raw, uart_rx_buf[i])) {
                /* Convert result to strings */
                hipnuc_dump_packet(&hipnuc_raw, log_buf, sizeof(log_buf));
                /* Display result */
                //printf("parse ok, frame len:%d\r\n", hipnuc_raw.len);
                //printf("%s\r\n", log_buf);
                printf("pitch yaw roll:%.3f, %.3f, %.3f\r\n", hipnuc_raw.hi91.pitch, hipnuc_raw.hi91.yaw, hipnuc_raw.hi91.roll);

                /*
                You can use data as for example:

                printf("acc:%.3f, %.3f, %.3f\r\n", hipnuc_raw.hi91.acc[0], hipnuc_raw.hi91.acc[1], hipnuc_raw.hi91.acc[2]);
                */
            }
        }

        uart_rx_index = 0; // Reset buffer index after processing
    }
}

/**
 * @brief Configure USART1 USART2 for IMU data reception
 *
 * Configures GPIO pins, USART1 USART2 settings, and NVIC for USART2 interrupts.
 */
static void USART_Configuration(uint32_t usart4_baud)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable USART4 and GPIOA clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* Configure USART4 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure USART4 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 ;
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
    USART_InitStructure.USART_BaudRate = usart4_baud;
    USART_Init(UART4, &USART_InitStructure);
    USART_Cmd(UART4, ENABLE);

    /* Enable USART2 IDLE line detection interrupt */
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);

/* Enable USART2 Receive interrupt */
#if !ENABLE_USART_DMA
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
#endif

    /* NVIC configuration for USART2 interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief UART4 interrupt handler
 *
 * Handles UART4 interrupts for IDLE line detection and RXNE (Receive Not Empty).
 */
void UART4_IRQHandler(void)
{
    if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET) {
        /* Clear IDLE line detected bit */
        USART_ReceiveData(UART4);

        handle_usart_rx_idle();
    }

#if !ENABLE_USART_DMA
    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) {
        uint8_t ch                   = USART_ReceiveData(UART4);
        uart_rx_buf[uart_rx_index++] = ch;
        if (uart_rx_index >= UART_RX_BUF_SIZE) {
            uart_rx_index = 0; // Prevent buffer overflow
        }
    }
#endif
}

/**
 * @brief Handle USART RX data when IDLE line is detected
 *
 * Processes received data and resets DMA if DMA is used.
 */
static void handle_usart_rx_idle(void)
{
#if ENABLE_USART_DMA
    /* Disable DMA1 Channel6 */
    DMA_Cmd(DMA1_Channel6, DISABLE);

    /* Get number of bytes received */
    uint16_t rx_size = UART_RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);

    /* Process received data */
    for (uint16_t i = 0; i < rx_size; i++) {
        uart_rx_buf[uart_rx_index++] = dma_rx_buf[i];

        if (uart_rx_index >= UART_RX_BUF_SIZE) {
            uart_rx_index = 0; // Prevent buffer overflow
        }
    }

    /* Reset DMA1 Channel6 */
    DMA_SetCurrDataCounter(DMA1_Channel6, UART_RX_BUF_SIZE);

    /* Re-enable DMA1 Channel6 */
    DMA_Cmd(DMA1_Channel6, ENABLE);
#endif

    /* Set flag to indicate new data available */
    new_data_flag = 1;
}

#define I2(p) ((int16_t)((uint8_t)((p)[0]) | ((uint16_t)((uint8_t)((p)[1])) << 8)))

static uint16_t U2(uint8_t *p)
{
    uint16_t u;
    memcpy(&u, p, 2);
    return u;
}

static float R4(uint8_t *p)
{
    float r;
    memcpy(&r, p, 4);
    return r;
}

/* parse the payload of a frame and feed into data section */
static int parse_data(hipnuc_raw_t *raw)
{
    int ofs = 0;
    uint8_t *p = &raw->buf[CH_HDR_SIZE];
    
    /* ignore all previous data */
    raw->hi91.tag = 0;
    raw->hi81.tag = 0;
    raw->hi92.tag = 0;

    while (ofs < raw->len)
    {
        switch (p[ofs])
        {
        case HIPNUC_ID_USRID:
            ofs += 2;
            break;
        case HIPNUC_ID_ACC_RAW:
        case HIPNUC_ID_ACC_CAL:
             raw->hi91.tag = HIPNUC_ID_IMUSOL;
             raw->hi91.acc[0] = (float)I2(p + ofs + 1) / 1000.0;
             raw->hi91.acc[1] = (float)I2(p + ofs + 3) / 1000;
             raw->hi91.acc[2] = (float)I2(p + ofs + 5) / 1000;
            ofs += 7;
            break;
        case HIPNUC_ID_GYR_RAW:
        case HIPNUC_ID_GYR_CAL:
            raw->hi91.tag = HIPNUC_ID_IMUSOL;
            raw->hi91.gyr[0] = (float)I2(p + ofs + 1) / 10;
            raw->hi91.gyr[1] = (float)I2(p + ofs + 3) / 10;
            raw->hi91.gyr[2] = (float)I2(p + ofs + 5) / 10;
            ofs += 7;
            break;
        case HIPNUC_ID_MAG_RAW:
            raw->hi91.tag = HIPNUC_ID_IMUSOL;
            raw->hi91.mag[0] = (float)I2(p + ofs + 1) / 10;
            raw->hi91.mag[1] = (float)I2(p + ofs + 3) / 10;
            raw->hi91.mag[2] = (float)I2(p + ofs + 5) / 10;
            ofs += 7;
            break;
        case HIPNUC_ID_EUL:
            raw->hi91.tag = HIPNUC_ID_IMUSOL;
            raw->hi91.pitch = (float)I2(p + ofs + 1) / 100;
            raw->hi91.roll = (float)I2(p + ofs + 3) / 100;
            raw->hi91.yaw = (float)I2(p + ofs + 5) / 10;
            ofs += 7;
            break;
        case HIPNUC_ID_QUAT:
            raw->hi91.tag = HIPNUC_ID_IMUSOL;
            raw->hi91.quat[0] = R4(p + ofs + 1);
            raw->hi91.quat[1] = R4(p + ofs + 5);
            raw->hi91.quat[2] = R4(p + ofs + 9);
            raw->hi91.quat[3] = R4(p + ofs + 13);
            ofs += 17;
            break;
        case HIPNUC_ID_PRS:
            raw->hi91.tag = HIPNUC_ID_IMUSOL;
            raw->hi91.prs = R4(p + ofs + 1);
            ofs += 5;
            break;
        case HIPNUC_ID_IMUSOL:
            memcpy(&raw->hi91, p + ofs, sizeof(hi91_t));
            ofs += sizeof(hi91_t);
            break;
        case HIPNUC_ID_INSSOL:
            memcpy(&raw->hi81, p + ofs, sizeof(hi81_t));
            ofs += sizeof(hi81_t);
            break;
        case HIPNUC_ID_IMUBIN:
            memcpy(&raw->hi92, p + ofs, sizeof(hi92_t));
            ofs += sizeof(hi92_t);
            break;
        default:
            ofs++;
            break;
        }
    }
    return 1;
}

static int decode_hipnuc(hipnuc_raw_t *raw)
{
    uint16_t crc = 0;

    /* checksum */
    hipnuc_crc16(&crc, raw->buf, (CH_HDR_SIZE-2));
    hipnuc_crc16(&crc, raw->buf + CH_HDR_SIZE, raw->len);
    if (crc != U2(raw->buf + (CH_HDR_SIZE-2)))
    {
        // NL_TRACE("ch checksum error: frame:0x%X calcuate:0x%X, len:%d\n", U2(raw->buf + 4), crc, raw->len);
        return -1;
    }

    return parse_data(raw);
}

/* sync code */
static int sync_hipnuc(uint8_t *buf, uint8_t data)
{
    buf[0] = buf[1];
    buf[1] = data;
    return buf[0] == CHSYNC1 && buf[1] == CHSYNC2;
}

/**
* @brief     hipnuc decoder input, read one byte at one time.
 *
 * @param    raw is the decoder struct.
 * @param    data is the one byte read from stram.
 * @param    buf is the log string buffer, ireturn > 0: decoder received a frame successfully, else: receiver not receive a frame successfully.
 *
 */
int hipnuc_input(hipnuc_raw_t *raw, uint8_t data)
{
    /* synchronize frame */
    if (raw->nbyte == 0)
    {
        if (!sync_hipnuc(raw->buf, data))
            return 0;
        raw->nbyte = 2;
        return 0;
    }

    raw->buf[raw->nbyte++] = data;

    if (raw->nbyte == CH_HDR_SIZE)
    {
        if ((raw->len = U2(raw->buf + 2)) > (HIPNUC_MAX_RAW_SIZE - CH_HDR_SIZE))
        {
            // NL_TRACE("ch length error: len=%d\n",raw->len);
            raw->nbyte = 0;
            return -1;
        }
    }

    if (raw->nbyte < CH_HDR_SIZE || raw->nbyte < (raw->len + CH_HDR_SIZE))
    {
        return 0;
    }

    raw->nbyte = 0;

    return decode_hipnuc(raw);
}


/**
 * @brief    convert packet to string, only dump parts of data
 *
 * @param    raw is struct of decoder
 * @param    buf is the log string buffer, make sure buf larger than 256
 * @param    buf_size is the szie of the log buffer
 *
 */
int hipnuc_dump_packet(hipnuc_raw_t *raw, char *buf, size_t buf_size)
{
    int written = 0;
    int ret;

    /* dump 0x91 packet */
    if(raw->hi91.tag == HIPNUC_ID_IMUSOL)
    {
        ret = snprintf(buf + written, buf_size - written, "%-16s0x%X\r\n", "tag:", raw->hi91.tag);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%d\r\n", "sync_time(ms):", raw->hi91.pps_sync_ms);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "acc(m/s^(2)):", raw->hi91.acc[0]*GRAVITY, raw->hi91.acc[1]*GRAVITY, raw->hi91.acc[2]*GRAVITY);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "gyr(deg/s):", raw->hi91.gyr[0], raw->hi91.gyr[1], raw->hi91.gyr[2]);
        if (ret > 0) written += ret;

        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "mag(uT):", raw->hi91.mag[0], raw->hi91.mag[1], raw->hi91.mag[2]);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "Roll/Pitch/Yaw(deg):", raw->hi91.roll, raw->hi91.pitch, raw->hi91.yaw);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f %.3f\r\n", "quat:", raw->hi91.quat[0], raw->hi91.quat[1], raw->hi91.quat[2], raw->hi91.quat[3]);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%d\r\n", "timestamp(ms):", raw->hi91.ts);
        if (ret > 0) written += ret;
    }
    
    /* dump 0x92 packet */
    if(raw->hi92.tag == HIPNUC_ID_IMUBIN)
    {
        ret = snprintf(buf + written, buf_size - written, "%-16s0x%X\r\n", "tag:", raw->hi92.tag);
        if (ret > 0) written += ret;
    
        ret = snprintf(buf + written, buf_size - written, "%-16s%d\r\n", "temperature", raw->hi92.temperature);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%d\r\n", "sync_time(ms):", raw->hi92.sync_time);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "acc(m/s^(2)):", raw->hi92.acc_b[0]*0.0048828, raw->hi92.acc_b[1]*0.0048828, raw->hi92.acc_b[2]*0.0048828);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "gyr(deg/s):", raw->hi92.gyr_b[0]*(0.001*R2D), raw->hi92.gyr_b[1]*(0.001*R2D), raw->hi92.gyr_b[2]*(0.001*R2D));
        if (ret > 0) written += ret;

        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "mag(uT):", raw->hi92.mag_b[0]*0.030517, raw->hi92.mag_b[1]*0.030517, raw->hi92.mag_b[2]*0.030517);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "Roll/Pitch/Yaw(deg):", raw->hi92.roll*0.001, raw->hi92.pitch*0.001, raw->hi92.yaw*0.001);
        if (ret > 0) written += ret;
    }

    /* dump 0x81 packet */
    if(raw->hi81.tag == HIPNUC_ID_INSSOL)
    {
        ret = snprintf(buf + written, buf_size - written, "%-16s0x%X\r\n", "tag:", raw->hi81.tag);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%d\r\n", "solq_pos:", raw->hi81.solq_pos);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%d\r\n", "sat number:", raw->hi81.nv_pos);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.7f %.7f\r\n", "Lat/Lon(deg):", raw->hi81.ins_lat*1e-7, raw->hi81.ins_lon*1e-7);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f\r\n", "height(m):", raw->hi81.ins_msl*1e-3);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "acc(m/s^(2)):", raw->hi81.acc_b[0]*0.0048828, raw->hi81.acc_b[1]*0.0048828, raw->hi81.acc_b[2]*0.0048828);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "gyr(deg/s):", raw->hi81.gyr_b[0]*(0.001*R2D), raw->hi81.gyr_b[1]*(0.001*R2D), raw->hi81.gyr_b[2]*(0.001*R2D));
        if (ret > 0) written += ret;

        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "mag(uT):", raw->hi81.mag_b[0]*0.030517, raw->hi81.mag_b[1]*0.030517, raw->hi81.mag_b[2]*0.030517);
        if (ret > 0) written += ret;
        
        ret = snprintf(buf + written, buf_size - written, "%-16s%.3f %.3f %.3f\r\n", "Roll/Pitch/Yaw(deg):", raw->hi81.roll*0.01, raw->hi81.pitch*0.01, raw->hi81.yaw*0.01);
        if (ret > 0) written += ret;
    }
    
    return written;
}

/**
 * @brief    calcuate hipnuc_crc16
 *
 * @param    inital is intial value
 * @param    buf    is input buffer pointer
 * @param    len    is length of the buffer
 *
 */
static void hipnuc_crc16(uint16_t *inital, const uint8_t *buf, uint32_t len)
{
    uint32_t crc = *inital;
    uint32_t j;
    for (j=0; j < len; ++j)
    {
        uint32_t i;
        uint32_t byte = buf[j];
        crc ^= byte << 8;
        for (i = 0; i < 8; ++i)
        {
            uint32_t temp = crc << 1;
            if (crc & 0x8000)
            {
                temp ^= 0x1021;
            }
            crc = temp;
        }
    } 
    *inital = crc;
}
