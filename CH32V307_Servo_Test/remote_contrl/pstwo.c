#include "pstwo.h"
#include "usart.h"
/*********************************************************
Copyright (C), 2015-2025, YFRobot.
www.yfrobot.com
File：PS2驱动程序
Author：pinggai    Version:1.0     Data:2015/05/16
Description: PS2驱动程序
**********************************************************/
u16 Handkey;
u8 Comd[2] = {0x01, 0x42};                                           // 开始命令。请求数据
u8 Data[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 数据存储数组

PS2_Controller_TypeDef PS2_Controller_t = {0};

Robot_Control_TypeDef   Robot_Control_t = {0};

u16 MASK[] = {
    PSB_SELECT,
    PSB_L3,
    PSB_R3,
    PSB_START,
    PSB_PAD_UP,
    PSB_PAD_RIGHT,
    PSB_PAD_DOWN,
    PSB_PAD_LEFT,
    PSB_L2,
    PSB_R2,
    PSB_L1,
    PSB_R1,
    PSB_GREEN,
    PSB_RED,
    PSB_BLUE,
    PSB_PINK}; // 按键值与按键明

// 手柄接口初始化    输入  DI->PB14
//                   输出  DO->PB15    CS->PB12  CLK->PB13
void PS2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 输入  DI->PB14
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能PORTB时钟

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; // 设置成上拉、下拉、浮空输入皆可
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 输出  DO->PB15    CS->PB12  CLK->PB13
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13 | GPIO_Pin_15 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; // 设置成推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 指示灯	PE2输出低电平亮
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); // 使能PORTE时钟
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; // 设置成推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    Delay_Ms(100);
    PS2_EnterConfing();
    Delay_Ms(10);
    PS2_TurnOnAnalogMode();
    Delay_Ms(10);
    PS2_ExitConfing();
}

// 向手柄发送命令
void PS2_Cmd(u8 CMD)
{
    volatile u16 ref = 0x01;
    Data[1]          = 0;
    for (ref = 0x01; ref < 0x0100; ref <<= 1) {
        if (ref & CMD) {
            DO_H; // 输出以为控制位
        } else
            DO_L;

        CLK_H; // 时钟拉高
        Delay_Us(50);
        CLK_L;
        Delay_Us(50);
        CLK_H;
        if (DI)
            Data[1] = ref | Data[1];
    }
}

// 判断是否为红灯模式
// 返回值；0，红灯模式
//		  其他，其他模式
u8 PS2_RedLight(void)
{
    CS_L;
    PS2_Cmd(Comd[0]); // 开始命令
    PS2_Cmd(Comd[1]); // 请求数据
    CS_H;
    if (Data[1] == 0X73)
        return 0;
    else
        return 1;
}
// 读取手柄数据
void PS2_ReadData(void)
{
    volatile u8 byte = 0;
    volatile u16 ref = 0x01;

    CS_L;

    PS2_Cmd(Comd[0]); // 开始命令
    PS2_Cmd(Comd[1]); // 请求数据

    for (byte = 2; byte < 9; byte++) // 开始接受数据
    {
        for (ref = 0x01; ref < 0x100; ref <<= 1) {
            CLK_H;
            CLK_L;
            Delay_Us(50);
            CLK_H;
            if (DI)
                Data[byte] = ref | Data[byte];
        }
        Delay_Us(50);
    }
    CS_H;
}

// 对读出来的PS2的数据进行处理               默认数据是红灯模式  只有一个按键按下时
// 按下为0， 未按下为1
u8 PS2_DataKey()
{
    u8 index;

    PS2_ClearData();
    PS2_ReadData();

    if (Data[1] == 0x73) {
        GPIO_ResetBits(GPIOE, GPIO_Pin_2); // 指示灯
        Robot_Control_t.enable = 1;

    } else {
        GPIO_SetBits(GPIOE, GPIO_Pin_2); // 指示灯
        Robot_Control_t.enable = 0;
        Robot_Control_t.linear_velocity=0;
    }

	PS2_Controller_t.LX=PS2_AnologData(PSS_LX);
	PS2_Controller_t.LY=PS2_AnologData(PSS_LY);
	PS2_Controller_t.RX=PS2_AnologData(PSS_RX);
	PS2_Controller_t.RY=PS2_AnologData(PSS_RY);

    Robot_Control_t.linear_velocity = -0.2*((float)PS2_Controller_t.LY - 128.0f) / 128.0f;

    Handkey = (Data[4] << 8) | Data[3]; // 这是16个按键  按下为0， 未按下为1
    for (index = 0; index < 16; index++) {
        if ((Handkey & (1 << (MASK[index] - 1))) == 0)
            return index + 1;
    }
    return 0; // 没有任何按键按下
}

// 得到一个摇杆的模拟量	 范围0~256
u8 PS2_AnologData(u8 button)
{
    return Data[button];
}

// 清除数据缓冲区
void PS2_ClearData()
{
    u8 a;
    for (a = 0; a < 9; a++)
        Data[a] = 0x00;
}

void PS2_EnterConfing(void)
{
    CS_L;
    Delay_Us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43);
    PS2_Cmd(0X00);
    PS2_Cmd(0x01);
    PS2_Cmd(0x00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    CS_H;
    Delay_Us(16);
}

// 完成并保存配置
void PS2_ExitConfing(void)
{
    CS_L;
    Delay_Us(16);
    PS2_Cmd(0x01);
    PS2_Cmd(0x43);
    PS2_Cmd(0X00);
    PS2_Cmd(0x00);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    PS2_Cmd(0x5A);
    CS_H;
    Delay_Us(16);
}

// 发送模式设置
void PS2_TurnOnAnalogMode(void)
{
    CS_L;
    PS2_Cmd(0x01);
    PS2_Cmd(0x44);
    PS2_Cmd(0X00);
    PS2_Cmd(0x00); // analog=0x01;digital=0x00  软件设置发送模式
    PS2_Cmd(0x03); // Ox03锁存设置，即不可通过按键“MODE”设置模式。
                   // 0xEE不锁存软件设置，可通过按键“MODE”设置模式。
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    PS2_Cmd(0X00);
    CS_H;
    Delay_Us(16);
}
