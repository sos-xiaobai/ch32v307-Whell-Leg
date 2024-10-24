#ifndef __PSTWO_H
#define __PSTWO_H
#include "ch32v30x_gpio.h"
#include "debug.h"
/*********************************************************
Copyright (C), 2015-2025, YFRobot.
www.yfrobot.com
File：PS2驱动程序
Author：pinggai    Version:1.0     Data:2015/05/16
Description: PS2驱动程序
**********************************************************/
// 手柄接口初始化
// 输入  DI->PB14
// 输出  DO->PB15    CS->PB12  CLK->PB13

// 输入定义
#define DI_PIN    GPIO_Pin_14
#define DI_PORT   GPIOB
#define DI GPIO_ReadInputDataBit(DI_PORT, DI_PIN)

// DO (PB15) 输出定义
#define DO_PIN  GPIO_Pin_15
#define DO_PORT GPIOB
#define DO_H    GPIO_SetBits(DO_PORT, DO_PIN)
#define DO_L    GPIO_ResetBits(DO_PORT, DO_PIN)

// CS (PB12) 输出定义
#define CS_PIN  GPIO_Pin_12
#define CS_PORT GPIOB
#define CS_H    GPIO_SetBits(CS_PORT, CS_PIN)
#define CS_L    GPIO_ResetBits(CS_PORT, CS_PIN)

// CLK (PB13) 输出定义
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
#define PSS_RX 5                //右摇杆X轴数据
#define PSS_RY 6
#define PSS_LX 7
#define PSS_LY 8

typedef struct {
    uint8_t RX;            // 右摇杆X轴
    uint8_t RY;            // 右摇杆Y轴
    uint8_t LX;            // 左摇杆X轴
    uint8_t LY;            // 左摇杆Y轴
    uint16_t Buttons;          // 按钮状态，每个位代表一个按钮
    uint8_t Pressure_TRIANGLE; // 三角形按钮压力值
    uint8_t Pressure_CIRCLE;   // 圆形按钮压力值
    uint8_t Pressure_CROSS;    // 叉形按钮压力值
    uint8_t Pressure_SQUARE;   // 方形按钮压力值
} PS2_Controller_TypeDef;

typedef struct {
    float linear_velocity;  // 线速度 (m/s)
    float yaw_angle;          // Yaw角度 (rad)
    uint8_t enable;
} Robot_Control_TypeDef;


extern u16 MASK[16];
extern u16 Handkey;
extern Robot_Control_TypeDef Robot_Control_t;

void PS2_Init(void);
u8 PS2_RedLight(void);//判断是否为红灯模式
void PS2_ReadData(void);
void PS2_Cmd(u8 CMD);		  //
u8 PS2_DataKey(void);		  //键值读取
u8 PS2_AnologData(u8 button); //得到一个摇杆的模拟量
void PS2_ClearData(void);	  //清除数据缓冲区
void PS2_TurnOnAnalogMode(void);
#endif





