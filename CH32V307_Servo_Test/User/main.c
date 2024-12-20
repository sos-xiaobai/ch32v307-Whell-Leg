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
#include "fashion_star_uart_servo.h"
#include "hipnuc.h"
#include "minipc.h"
#include "pid.h"
#include "motor.h"
#include "pstwo.h"
#include "nangu_servo.h"

// #define pid
#define lqr

#define torque_current_to_out 2048.0 / 16.5f
#define Torque_Constant       0.08
#define lqr_k                 torque_current_to_out / Torque_Constant

#define min_1                 -10.0f
#define max_1                 30.0f
#define min_2                 -30.0f
#define max_2                 10.0f
#define LIMIT_left(x)         ((x) < (min_1) ? (min_1) : ((x) > (max_1) ? (max_1) : (x)))
#define LIMIT_right(x)        ((x) < (min_2) ? (min_2) : ((x) > (max_2) ? (max_2) : (x)))

extern void
setup();
extern void loop();

extern void Leg_Control_Setup();

u8 txbuf_left[8]           = {0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
u8 txbuf_right[8]          = {0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
u8 ask_v[8]                = {0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int32_t PowerControl_left  = 0;
int32_t PowerControl_right = 0;

PID PID_Pitch_Angle_Loop = {0};
PID PID_Pitch_Gyro_Loop  = {0};
PID PID_LinerSpeed_Loop  = {0};
PID PID_Distance_Loop    = {0};
PID PID_Yaw_Loop         = {0}; // 左转为yaw增
PID PID_Roll_Loop        = {0};

int position = 3;
// 中60，低0
float Zero_Point = 17.5;
// 中13.5 低19.5
int pose = -1;
// 1高0中-1低
float compensation_right = 1.0f;

float Pitch_Angle_Loop_Out;
float Pitch_Gyro_Loop_Out;
float LinerSpeed_Loop_Out;
float Distance_Loop_out;
float Yaw_Loop_out;
float roll_Loop_out;
float roll_Loop_out_sum;
float Out_Sum;

// 定义PID参数变量

float pitch_angle_kp = 13.0f;
float pitch_angle_ki = 0.0f;
float pitch_angle_kd = 0.0f;
float pitch_gyro_kp  = 1.2f;
float pitch_gyro_ki  = 0.0f;
float pitch_gyro_kd  = 0.0f;
float liner_speed_kp = 750.0f;
float liner_speed_ki = 0.15f;
float liner_speed_kd = 0.0f;
float yaw_kp         = -1.0;
float yaw_ki         = -0.00000f;
float yaw_kd         = 0.0f;
float distance_kp    = 450.0f;
float distance_ki    = 0.0f;
float distance_kd    = -0.0f;


float lqr_distance   = 0.0f;
float lqr_lin_speed  = 0.0f; // 0.539f;
float lqr_angle      = 0.0f;
float lqr_anglespeed = 0.0f; // 0.2046f;

float lqr_yaw_angle       = -5.5f;
float lqr_yaw_angle_speed = -0.62f;

float lqr_roll_angle       = 0.003f;
float lqr_roll_angle_speed = 0.0f;

float lqr_distance_out   = 0;
float lqr_lin_speed_out  = 0;
float lqr_angle_out      = 0;
float lqr_anglespeed_out = 0;

float pid_integral_limit = 500.0f;
float pid_output_limit   = 800.0f;

float distance          = 0;
float speed             = 0; // 单位：dps
float speed_mps         = 0;
float distance_m        = 0;
float target_distance_m = 0;

float voltage_k = 0;

float max_speed = 0.7;

// 定义状态枚举
typedef enum {
    STATE_NORMAL,
    STATE_WHEEL_OFF_GROUND
} WheelState;

// 状态机变量
static WheelState current_state     = STATE_NORMAL;
static uint32_t state_timer         = 0;
static const uint32_t RECOVERY_TIME = 2000; // 1秒 = 1000次(1000Hz)
float Threshold                     = 1.6;
float jump_outPut=0;
int load              = 0;
int cnt               = 0;
bool wheel_off_ground = 0;

int out_left  = 0;
int out_right = 0;

int16_t test         = 1;
uint16_t cnt_time    = 0;
float test_speed     = 0.0;
float last_speed_mps = 0;
void TIM1_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure           = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); // 使能TIM1时钟

    TIM_TimeBaseStructure.TIM_Period            = arr;                // 指定下次更新事件时要加载到活动自动重新加载寄存器中的周期值。
    TIM_TimeBaseStructure.TIM_Prescaler         = psc;                // 指定用于划分TIM时钟的预分频器值。
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;       // 时钟分频因子
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up; // TIM计数模式，向上计数模式
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); // 根据指定的参数初始化TIMx的时间基数单位

    // 初始化TIM NVIC，设置中断优先级分组
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM1_UP_IRQn; // TIM1中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;            // 设置抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;            // 设置响应优先级3
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;       // 使能通道1中断
    NVIC_Init(&NVIC_InitStructure);                                      // 初始化NVIC

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE); // 使能TIM1中断，允许更新中断

    TIM_Cmd(TIM1, ENABLE); // TIM1使能
}
 int offset=-4;
int main(void)
{
    /* System Clock Configuration */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();

    /* Bps = 333Kbps */ // pclk1=144/2=72mhz 72/6/(5+6+1)=1mhz 1000kbps
    CAN_Mode_Init(CAN_SJW_1tq, CAN_BS2_5tq, CAN_BS1_6tq, 6, CAN_Mode_Normal);

    /*定时器中断初始化*/
    TIM1_Int_Init(999, 143);

    /*配置调试串口 usart2*/
    USART_Printf_Init(115200);

    /*配置单线半双工模式舵机串口,头超前，左舵机id为1，右舵机id为0*/
    // Leg_Control_Setup();
    Usart_Init();
    // SysTick_Init();
    /*imu串口*/
    app_init();
    /*打印imu采集数据欢迎信息*/
    printf_welcome_information();

    /*上位机串口接收初始化*/
    Minipc_Init();

    /*遥控器初始化*/
    PS2_Init();

    /*南古舵机初始化*/
    nangu_servo_init();

    // PID初始化
    /*角度pid初始化*/
    PID_Init(&PID_Pitch_Angle_Loop, pitch_angle_kp, pitch_angle_ki, pitch_angle_kd, pid_integral_limit, pid_output_limit);
    PID_Init(&PID_Pitch_Gyro_Loop, pitch_gyro_kp, pitch_gyro_ki, pitch_gyro_kd, pid_integral_limit, pid_output_limit);
    PID_Init(&PID_LinerSpeed_Loop, liner_speed_kp, liner_speed_ki, liner_speed_kd, pid_integral_limit, 500);
    PID_Init(&PID_Distance_Loop, distance_kp, 0, 0, pid_integral_limit, pid_output_limit);
    // PID_Init(&PID_Yaw_Loop, yaw_kp, yaw_ki, yaw_kd, pid_integral_limit, pid_output_limit);
    // PID_Init(&PID_Roll_Loop, roll_kp, roll_ki, roll_kd, 60, 60);

    while (1) {

        /*imu*/
        process_data();

        /*ps2读取*/
        //PS2_DataKey();


//        //左腿
//        FSUS_SetServoAngle(&usart5, 1, LIMIT_left(jump_outPut+position+roll_Loop_out_sum ), 10, 0, 0);
//        Delay_Ms(10);
//        //右腿
//        FSUS_SetServoAngle(&usart5, 0, LIMIT_right(-jump_outPut-position+roll_Loop_out_sum ), 10, 0, 0);
//        /*南古舵机函数*/
//
//        set_servo_angle(SERVO_CHANNEL_1, 0+offset);
//        set_servo_angle(SERVO_CHANNEL_2, 0);
        //Delay_Ms(1000);

//        set_servo_angle(SERVO_CHANNEL_1, -30+offset);
//        set_servo_angle(SERVO_CHANNEL_2, 30);
//        Delay_Ms(80);
//        set_servo_angle(SERVO_CHANNEL_1, 0+offset);
//        set_servo_angle(SERVO_CHANNEL_2, 0);

        //Delay_Ms(2000);
        /*SCS15舵机函数，暂时弃用*/
        // voltage=ReadVoltage(1);
        //  WritePosEx(2, out_left, 100, 250);
        //  WritePosEx(1, 1023-60, 100, 250);

#ifdef lqr
        printf("angle:%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
               hipnuc_raw.hi91.pitch,
               Out_Sum,
               lqr_angle_out * lqr_k,
               lqr_anglespeed_out * lqr_k,
               lqr_lin_speed_out * lqr_k,
               lqr_distance_out * lqr_k,
               speed_mps,
               test_speed,
               Motor_Left.Now_Omega_Angle,
               -Motor_Right.Now_Omega_Angle,
               Motor_Left.Now_Power,
               -Motor_Right.Now_Power,
               hipnuc_raw.hi91.gyr[2],
               Robot_Control_t.linear_velocity);
#endif //

#ifdef pid
        printf("angle:%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d\n",
               hipnuc_raw.hi91.pitch,
               Out_Sum,
               Pitch_Angle_Loop_Out,
               Pitch_Gyro_Loop_Out,
               LinerSpeed_Loop_Out,
               Distance_Loop_out,
               voltage,
               voltage_k,
               speed_mps,
               PID_LinerSpeed_Loop.integral,
               pose,
               cnt_time);

#endif // DEBUG
    }
}

void pose_contrl()
{
    switch (pose) {
        case 0:
            position   = 20;
            Zero_Point = 7.4;

            lqr_distance   = 0.0645;
            lqr_lin_speed  = 0.0823;
            lqr_angle      = 0.3869;
            lqr_anglespeed = 0.0448;
            break;
        case -1:
            position   = 0;
            Zero_Point = 16.5;
            lqr_distance   = 0.0645;
            lqr_lin_speed  = 0.0841;
            lqr_angle      = 0.3292;
            lqr_anglespeed = 0.0340;
            break;
        default:
            break;
    }
}

float map_voltage_to_coefficient(float voltage)
{
    // 定义输入范围
    const float in_min = 7.4f;
    const float in_max = 8.4f;

    // 定义输出范围
    const float out_min = 1.1f;
    const float out_max = 0.7f;

    // 确保输入在有效范围内
    if (voltage < in_min) voltage = in_min;
    if (voltage > in_max) voltage = in_max;

    // 执行线性映射
    float coefficient = (voltage - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    return coefficient;
}

// 在1000Hz的周期函数中调用此函数
void wheel_state_machine(float last_speed_mps, float speed_mps)
{
    // 检查轮子是否离地
    wheel_off_ground = (last_speed_mps - speed_mps > Threshold) || (last_speed_mps - speed_mps < -Threshold);

    switch (current_state) {
        case STATE_NORMAL:
            if (wheel_off_ground) {
                // 立即进入轮子离地状态
                current_state = STATE_WHEEL_OFF_GROUND;
                state_timer   = 0;
            }
            break;

        case STATE_WHEEL_OFF_GROUND:
            if (!wheel_off_ground) {
                // 轮子回到地面,开始计时
                state_timer++;
                Distance_Loop_out            = 0;
                PID_Distance_Loop.integral   = 0;
                LinerSpeed_Loop_Out          = 0;
                PID_LinerSpeed_Loop.integral = 0;
                if (state_timer >= RECOVERY_TIME) {
                    // 恢复到正常状态
                    current_state = STATE_NORMAL;
                }
            } else {
                // 轮子仍然离地,重置计时器
                state_timer                  = 0;
                Distance_Loop_out            = 0;
                PID_Distance_Loop.integral   = 0;
                LinerSpeed_Loop_Out          = 0;
                PID_LinerSpeed_Loop.integral = 0;
            }
            break;
    }
}

// 定义跳跃状态
typedef enum {
    JUMP_IDLE,
    JUMP_PREPARE,
    JUMP_THRUST,
    JUMP_AIR,
    JUMP_LAND
} JumpState;

// 跳跃状态机变量
static JumpState jump_state             = JUMP_IDLE;
static uint32_t jump_timer              = 0;
static const uint32_t JUMP_PREPARE_TIME = 2000; // 准备时间（毫秒）
static const uint32_t JUMP_THRUST_TIME  = 100;   // 推力时间（毫秒）
static const uint32_t JUMP_AIR_TIME     = 300;  // 空中时间（毫秒）
static const uint32_t JUMP_LAND_TIME    = 200;  // 着陆时间（毫秒）

bool jump_trigger_condition = 1;

// 腿部状态机函数
void leg_state_machine(int target_pos, int position)
{
    switch (jump_state) {
        case JUMP_IDLE:
            // 检测跳跃触发条件（例如，通过上位机命令）
            if (jump_trigger_condition) {
                jump_state = JUMP_PREPARE;
                jump_timer = 0;

            } else {
                jump_outPut=0;
            }
            break;

        case JUMP_PREPARE:
            jump_timer++;
            if (jump_timer >= JUMP_PREPARE_TIME) {
                jump_state = JUMP_THRUST;
                jump_timer = 0;
                // 设置腿部舵机到准备姿态

                jump_outPut=50;
            }
            break;

        case JUMP_THRUST:
            jump_timer++;
            if (jump_timer >= JUMP_THRUST_TIME) {
                jump_state = JUMP_AIR;
                jump_timer = 0;

                jump_outPut=0;
                target_distance_m=0;
                distance_m=0;
                Out_Sum = 0;
            }
            break;

        case JUMP_AIR:
            jump_timer++;
            if (jump_timer >= JUMP_AIR_TIME) {
                jump_state = JUMP_LAND;
                jump_timer = 0;
                // 准备着陆，可能需要调整腿部姿态
                jump_outPut=0;
                target_distance_m=0;
                distance_m=0;
                Out_Sum = 0;
            }
            break;

        case JUMP_LAND:
            jump_timer++;
            if (jump_timer >= JUMP_LAND_TIME) {
                jump_state = JUMP_IDLE;
                // 恢复正常的平衡控制
                target_distance_m=0;
                distance_m=0;
            }
            break;
    }
}

// 定时器2中断服务函数
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    cnt++;
    cnt_time++;
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);



        /*更新状态*/
        speed_mps = (Motor_Left.Now_Omega_Angle - Motor_Right.Now_Omega_Angle) / 2 * DEG_TO_RAD * 0.03;
        distance_m += speed_mps * 0.001;
        target_distance_m += test_speed * 0.001;

        /*舵机*/
        //leg_state_machine(roll_Loop_out, position);
        /*姿态控制*/
        pose_contrl();

        /*轮部离地检测*/
        // wheel_state_machine(last_speed_mps, speed_mps);

        /*避免电压影响*/
        voltage_k = map_voltage_to_coefficient(voltage);

        /*测试蹲起*/

        /*南古舵机函数*/
        if(cnt_time<2000)
        {
            set_servo_angle(SERVO_CHANNEL_1, -5+offset);
            set_servo_angle(SERVO_CHANNEL_2, 5);
        }
        else if(cnt_time>2000 && cnt_time<2080)
        {
            set_servo_angle(SERVO_CHANNEL_1, -30+offset);
            set_servo_angle(SERVO_CHANNEL_2, 30);
        }
        else if(cnt_time>2080)
        {
            cnt_time = 0;
        }

//                        if(cnt_time%5000==0)
//                        {
//                            if(pose==0)
//                            {
//                                pose=-1;
//                            }
//                            else if(pose==-1)
//                                {
//                                    pose=0;
//                                }
//                        }


        //        if(cnt_time%50==0)
        //        {
        //            test_speed+=0.01;
        //            if(test_speed>=0.3)
        //                test_speed=0.3;
        //        }
        // PID_Calc(&PID_Yaw_Loop, 0.0f, hipnuc_raw.hi91.yaw);
        //PID_Calc(&PID_Roll_Loop, 0.0f, hipnuc_raw.hi91.roll);
        
        Yaw_Loop_out = (0 - hipnuc_raw.hi91.yaw) * lqr_yaw_angle + (0 - hipnuc_raw.hi91.gyr[2]) * lqr_yaw_angle_speed;
        if(fabs(hipnuc_raw.hi91.roll)>0.2f)
        {
            roll_Loop_out = (0-hipnuc_raw.hi91.roll)*lqr_roll_angle + (0-hipnuc_raw.hi91.gyr[0])*lqr_roll_angle_speed;
            roll_Loop_out_sum+=roll_Loop_out;
            if(roll_Loop_out_sum>=10.0f)
            {
                roll_Loop_out_sum = 10.0f;
            }
            if(roll_Loop_out_sum<=-10.0f)
            {
                roll_Loop_out_sum = -10.0f;
            }
        }
#ifdef pid
        /*pid计算*/
        PID_Calc(&PID_Pitch_Angle_Loop, Zero_Point, hipnuc_raw.hi91.pitch);
        PID_Calc(&PID_Pitch_Gyro_Loop, 0.0f, hipnuc_raw.hi91.gyr[1]);
        PID_Calc(&PID_LinerSpeed_Loop, test_speed, speed_mps);
        PID_Calc(&PID_Distance_Loop, target_distance_m, distance_m);

        Pitch_Angle_Loop_Out = PID_Pitch_Angle_Loop.output;
        Pitch_Gyro_Loop_Out  = PID_Pitch_Gyro_Loop.output;
        LinerSpeed_Loop_Out  = PID_LinerSpeed_Loop.output;
        Distance_Loop_out    = PID_Distance_Loop.output;

        // 上电2s后打开速度环i输出
        if (cnt_time > 1800) {
            PID_LinerSpeed_Loop.ki = liner_speed_ki;
            // test_speed+=0.0001*test;

            if (test_speed >= max_speed)
                test_speed = max_speed;
        } else {
            // PID_LinerSpeed_Loop.ki=0;
        }

        Out_Sum = voltage_k * (Pitch_Angle_Loop_Out + Pitch_Gyro_Loop_Out + Yaw_Loop_out + Distance_Loop_out + LinerSpeed_Loop_Out);
#endif // DEBUG
#ifdef lqr
        /*lqr*/
        lqr_distance_out   = lqr_distance * (target_distance_m - distance_m);
        lqr_lin_speed_out  = lqr_lin_speed * (test_speed - speed_mps);
        lqr_angle_out      = lqr_angle * (Zero_Point - hipnuc_raw.hi91.pitch) * DEG_TO_RAD;
        lqr_anglespeed_out = lqr_anglespeed * (0 - hipnuc_raw.hi91.gyr[1]) * DEG_TO_RAD;


        lqr_distance_out=0;

//        if(lqr_lin_speed_out>100)
//        {
//            lqr_lin_speed_out=100;
//        }
//        if(lqr_lin_speed_out<-100)
//        {
//            lqr_lin_speed_out=-100;
//        }
        //float lqr_angle_out_limit=200;
//                if(lqr_angle_out>lqr_angle_out_limit)
//                {
//                    lqr_angle_out=lqr_angle_out_limit;
//                }
//                if(lqr_angle_out<-lqr_angle_out_limit)
//                {
//                    lqr_angle_out=-lqr_angle_out_limit;
//                }

        //
        //        lqr_distance_out   = -lqr_distance * (0 - distance_m);
        //        lqr_lin_speed_out  = -lqr_lin_speed * (0 - speed_mps);
        //        lqr_angle_out      = 0;
        //        lqr_anglespeed_out = 0;

//        if(Robot_Control_t.enable)
//        {
//            test_speed=Robot_Control_t.linear_velocity;
//        }

        // 上电2s后打开速度输出
//        if (cnt_time > 1800) {
//            test_speed += 0.0001 * test;
//
//            if (test_speed >= max_speed) {
//                 test_speed = max_speed;
//                //test_speed -= 0.0001 * test;
//            }
//
//        } else {
//            // PID_LinerSpeed_Loop.ki=0;
//        }

        Out_Sum = lqr_k * (lqr_distance_out + lqr_lin_speed_out + lqr_angle_out + lqr_anglespeed_out) / 2;

        // Out_Sum=-100;
#endif // DEBUG

        /*测试用*/
         //Out_Sum = 0;
        // Yaw_Loop_out=0;

//        if (hipnuc_raw.hi91.pitch > 80 || hipnuc_raw.hi91.pitch < -80||jump_state==JUMP_THRUST) {
//            Out_Sum = 0;
//        }

        PowerControl_right = (int16_t)(compensation_right * (Out_Sum + Yaw_Loop_out));
        PowerControl_left  = (int16_t)(-Out_Sum + Yaw_Loop_out);

        txbuf_left[4]  = (uint8_t)(PowerControl_left & 0xff);
        txbuf_left[5]  = (uint8_t)(PowerControl_left >> 8);
        txbuf_right[4] = (uint8_t)(PowerControl_right & 0xff);
        txbuf_right[5] = (uint8_t)(PowerControl_right >> 8);

        //        uint8_t t[8]={0x32, 0,100,100,40,14,0x3c,0x28};
        //        CAN_Send_Msg(t, 8, 0x142);
         CAN_Send_Msg(txbuf_left, 8, 0x141);
         CAN_Send_Msg(txbuf_right, 8, 0x142);

        //        if (cnt > 2000) {
        //            CAN_Send_Msg(ask_v, 8, 0x142);
        //            cnt = 0;
        //        }
    }
}
