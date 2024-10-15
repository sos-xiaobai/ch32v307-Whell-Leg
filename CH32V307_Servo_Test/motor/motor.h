/**
 * @file dvc_LKmotor.h
 * @author lez
 * @brief lk电机配置与操作
 * @version 0.1
 * @date 2024-07-1 0.1 24赛季定稿
 *
 * @copyright ZLLC 2024
 *
 */
#ifndef _MOTOR_H_
#define _MOTOR_H_
/* Includes ------------------------------------------------------------------*/
//#include  "stdio.h"
/* Exported macros -----------------------------------------------------------*/
//typedef uint8_t u8;
#define PI 3.1415926535f
//RPM换算到rad/s
#define RPM_TO_RADPS (2.0f * PI / 60.0f)
//RPM换算到角度/s
#define RPM_TO_DEG (360.0f / 60.0f)
//deg换算到rad
#define DEG_TO_RAD (PI / 180.0f)
//rad换算到deg
#define RAD_TO_DEG (180.0f / PI)
//摄氏度换算到开氏度
#define CELSIUS_TO_KELVIN (273.15f)
//重力加速度
#define GRAVITY_ACCELERATE (9.7933f)
/* Exported types ------------------------------------------------------------*/

/**
 * @brief ????????
 *
 */
typedef enum Enum_LK_Motor_Status
{
    LK_Motor_Status_DISABLE = 0,
    LK_Motor_Status_ENABLE,
}Enum_LK_Motor_Status;

/**
 * @brief ????????ID???????
 *
 */
typedef enum Enum_LK_Motor_ID
{
    LK_Motor_ID_UNDEFINED = 0,
    LK_Motor_ID_0x141,
    LK_Motor_ID_0x142,
    LK_Motor_ID_0x143,
    LK_Motor_ID_0x144,
    LK_Motor_ID_0x145,
    LK_Motor_ID_0x146,
    LK_Motor_ID_0x147,
    LK_Motor_ID_0x148,
}Enum_LK_Motor_ID;

/**
 * @brief ????????????
 *
 */
typedef enum Enum_LK_Motor_Control_Status
{
    LK_Motor_Control_Status_DISABLE = 0,
    LK_Motor_Control_Status_ENABLE,
}Enum_LK_Motor_Control_Status;

/**
 * @brief ??????can????cmd_id
 *
 */
typedef enum Enum_LK_Motor_Control_ID
{
    LK_Motor_Control_Shut_Down = 0x80,
    LK_Motor_Control_Stop = 0x81,
    LK_Motor_Control_Run = 0x88,
    LK_Motor_Control_Torque = 0xA1,
    LK_Motor_Control_Votalge = 0xA0,
}Enum_LK_Motor_Control_ID;

/**
 * @brief ????????????
 *
 */
typedef enum Enum_LK_Motor_Control_Method
{
    LK_Motor_Control_Method_IMU_ANGLE = 0,
    LK_Motor_Control_Method_IMU_OMEGA,
    LK_Motor_Control_Method_ANGLE,
    LK_Motor_Control_Method_OMEGA,
    LK_Motor_Control_Method_TORQUE,
}Enum_LK_Motor_Control_Method;

/**
 * @brief ???????????
 *
 */
typedef struct __attribute__((__packed__))
{
    Enum_LK_Motor_Control_ID CMD_ID : 8;
    uint8_t Temperature_Centigrade;
    uint16_t Current_Reverse;
    uint16_t Omega_Reverse;
    uint16_t Encoder_Reverse;
}Struct_LK_Motor_CAN_Rx_Data;

/**
 * @brief ????????????????????, ???��????��??
 *
 */
#pragma pack(1)
typedef struct Struct_LK_Motor_Rx_Data
{
    Enum_LK_Motor_Control_ID CMD_ID;  //????????ID
    float Now_Angle;  //��ǰ�Ƕ� ���Ƕ��ƣ�
    float Now_Radian;  //��ǰ�Ƕ� �������ƣ�
    float Now_Omega_Angle;  //��ǰ���ٶ� (�Ƕ���)
    float Now_Omega_Radian;  //��ǰ���ٶ� (������)
    float Now_Power;
    float Now_Temperature;
    uint16_t Pre_Encoder;
    int32_t Total_Encoder;
    int32_t Total_Round;
    uint8_t Start_Flag;
}Struct_LK_Motor_Rx_Data;
#pragma pack()

extern uint8_t Start_Flag;
extern uint32_t Flag;
extern uint32_t Pre_Flag;
extern uint32_t Position_Max3;
extern float Current_Max;
extern Enum_LK_Motor_Status LK_Motor_Status;
extern Struct_LK_Motor_Rx_Data Data;
extern Struct_LK_Motor_Rx_Data Motor_Left;
extern Struct_LK_Motor_Rx_Data Motor_Right;
extern float voltage;
void Data_Process(uint8_t* CAN_Rx_Buffer,Struct_LK_Motor_Rx_Data* Data);
//void CAN_RxCpltCallback(uint8_t *Rx_Data);
void TIM_Alive_PeriodElapsedCallback();
float Math_Int_To_Float(int32_t x, int32_t Int_Min, int32_t Int_Max, float Float_Min, float Float_Max);
void Math_Endian_Reverse_16(void *Source, void *Destination);
/* Exported variables --------------------------------------------------------*/
void USB_LP_CAN1_RX0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void CAN1_RX1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
u8 CAN_Send_Msg( u8 *msg, u8 len ,uint32_t id);
void CAN_Mode_Init( u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode );
/* Exported function declarations --------------------------------------------*/

#endif
