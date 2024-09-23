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

 */

#include "debug.h"
#include "SMS_STS.h"
#include "hipnuc.h"
#include "minipc.h"
#include "pid.h"
#include "motor.h"

//extern void setup();
//extern void loop();
extern void Leg_Control_Setup();
extern void Leg_Control_Loop();

u8 txbuf_left[8]={0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
u8 txbuf_right[8]={0xA0,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

int16_t PowerControl_left = 0;
int16_t PowerControl_right = 0;

PID PID_Pitch_Angle_Loop = {0};
PID PID_Pitch_Gyro_Loop = {0};
PID PID_LinerSpeed_Loop = {0};
PID PID_Distance_Loop = {0};

#define Zero_Point 19.8f

float Pitch_Angle_Loop_Out;
float Pitch_Gyro_Loop_Out;
float LinerSpeed_Loop_Out;
float Distance_Loop_out;
float Out_Sum;

int main(void)
{
    /* System Clock Configuration */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();

    /* Bps = 333Kbps */  //pclk1=144/2=72mhz 72/6/(5+6+1)=1mhz 1000kbps
    CAN_Mode_Init( CAN_SJW_1tq, CAN_BS2_5tq, CAN_BS1_6tq, 6, CAN_Mode_Normal );

    /*配置调试串口 usart3*/
    USART_Printf_Init(115200);

    /*配置单线半双工模式舵机串口*/
    Leg_Control_Setup();

    /*imu串口*/
    app_init();
    /*打印imu采集数据欢迎信息*/
    printf_welcome_information();

    /*上位机串口接收初始化*/
    Minipc_Init();

    /*角度pid初始化*/
    PID_Init(&PID_Pitch_Angle_Loop, 65, 0, 0, 200, 800);
    PID_Init(&PID_Pitch_Gyro_Loop, 1.5, 0, 0, 200, 800);
    PID_Init(&PID_LinerSpeed_Loop, 1, 0, 0, 200, 800);
    PID_Init(&PID_Distance_Loop, 0, 0, 0, 200, 800);

    while(1)
    {
        /*舵机*/
        Leg_Control_Loop();
        /*imu*/
        process_data();
        PID_Calc(&PID_Pitch_Angle_Loop, Zero_Point, hipnuc_raw.hi91.pitch);
        PID_Calc(&PID_Pitch_Gyro_Loop,0.0f,hipnuc_raw.hi91.gyr[1]);
        Pitch_Angle_Loop_Out = PID_Pitch_Angle_Loop.output;
        Pitch_Gyro_Loop_Out = PID_Pitch_Gyro_Loop.output;
        Out_Sum = Pitch_Angle_Loop_Out + Pitch_Gyro_Loop_Out;
        Out_Sum = 0;
        PowerControl_right = (int16_t)Out_Sum;
        PowerControl_left = -PowerControl_right;
        txbuf_left[4]=(uint8_t)(PowerControl_left&0xff);
        txbuf_left[5]=(uint8_t)(PowerControl_left>>8);
        txbuf_right[4]=(uint8_t)(PowerControl_right&0xff);
        txbuf_right[5]=(uint8_t)(PowerControl_right>>8);
        CAN_Send_Msg(txbuf_left,8,0x141);
        CAN_Send_Msg(txbuf_right,8,0x142);
        printf("tmp:%.f power:%.f speed:%.f angle:%.f\r\n",Motor_Right.Now_Temperature,Motor_Right.Now_Power,Motor_Right.Now_Omega_Angle, Motor_Right.Now_Angle);

    }


}
