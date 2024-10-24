#ifndef __PSTWO_H
#define __PSTWO_H
#include "ch32v30x_gpio.h"
#include "debug.h"
/*********************************************************
Copyright (C), 2015-2025, YFRobot.
www.yfrobot.com
File��PS2��������
Author��pinggai    Version:1.0     Data:2015/05/16
Description: PS2��������
**********************************************************/
// �ֱ��ӿڳ�ʼ��
// ����  DI->PB14
// ���  DO->PB15    CS->PB12  CLK->PB13

// ���붨��
#define DI_PIN    GPIO_Pin_14
#define DI_PORT   GPIOB
#define DI GPIO_ReadInputDataBit(DI_PORT, DI_PIN)

// DO (PB15) �������
#define DO_PIN  GPIO_Pin_15
#define DO_PORT GPIOB
#define DO_H    GPIO_SetBits(DO_PORT, DO_PIN)
#define DO_L    GPIO_ResetBits(DO_PORT, DO_PIN)

// CS (PB12) �������
#define CS_PIN  GPIO_Pin_12
#define CS_PORT GPIOB
#define CS_H    GPIO_SetBits(CS_PORT, CS_PIN)
#define CS_L    GPIO_ResetBits(CS_PORT, CS_PIN)

// CLK (PB13) �������
#define CLK_PIN  GPIO_Pin_13
#define CLK_PORT GPIOB
#define CLK_H    GPIO_SetBits(CLK_PORT, CLK_PIN)
#define CLK_L    GPIO_ResetBits(CLK_PORT, CLK_PIN)


//These are our button constants
#define PSB_SELECT      1
#define PSB_L3          2
#define PSB_R3          3
#define PSB_START       4
#define PSB_PAD_UP      5
#define PSB_PAD_RIGHT   6
#define PSB_PAD_DOWN    7
#define PSB_PAD_LEFT    8
#define PSB_L2          9
#define PSB_R2          10
#define PSB_L1          11
#define PSB_R1          12
#define PSB_GREEN       13
#define PSB_RED         14
#define PSB_BLUE        15
#define PSB_PINK        16
#define PSB_TRIANGLE    13
#define PSB_CIRCLE      14
#define PSB_CROSS       15
#define PSB_SQUARE      26

//#define WHAMMY_BAR		8

//These are stick values
#define PSS_RX 5                //��ҡ��X������
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

typedef struct {
    uint8_t RX;            // ��ҡ��X��
    uint8_t RY;            // ��ҡ��Y��
    uint8_t LX;            // ��ҡ��X��
    uint8_t LY;            // ��ҡ��Y��
    uint16_t Buttons;          // ��ť״̬��ÿ��λ����һ����ť
    uint8_t Pressure_TRIANGLE; // �����ΰ�ťѹ��ֵ
    uint8_t Pressure_CIRCLE;   // Բ�ΰ�ťѹ��ֵ
    uint8_t Pressure_CROSS;    // ���ΰ�ťѹ��ֵ
    uint8_t Pressure_SQUARE;   // ���ΰ�ťѹ��ֵ
} PS2_Controller_TypeDef;

typedef struct {
    float linear_velocity;  // ���ٶ� (m/s)
    float yaw_angle;          // Yaw�Ƕ� (rad)
    uint8_t enable;
} Robot_Control_TypeDef;


extern u16 MASK[16];
extern u16 Handkey;
extern Robot_Control_TypeDef Robot_Control_t;

void PS2_Init(void);
u8 PS2_RedLight(void);//�ж��Ƿ�Ϊ���ģʽ
void PS2_ReadData(void);
void PS2_Cmd(u8 CMD);		  //
u8 PS2_DataKey(void);		  //��ֵ��ȡ
u8 PS2_AnologData(u8 button); //�õ�һ��ҡ�˵�ģ����
void PS2_ClearData(void);	  //������ݻ�����
void PS2_TurnOnAnalogMode(void);
#endif





