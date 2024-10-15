/**
 * @file dvc_LKmotor.cpp
 * @author lez
 * @brief lk������������
 * @version 0.1
 * @date 2024-07-1 0.1 24��������
 *
 * @copyright ZLLC 2024
 *
 */
/* Includes ------------------------------------------------------------------*/
#include "debug.h"
#include "motor.h"
#include "string.h"
/* Private macros ------------------------------------------------------------*/

/* Private types -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

//������������Ϣ
uint8_t LK_Motor_CAN_Message_Clear_Error[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb};
//ʹ�ܵ��
uint8_t LK_Motor_CAN_Message_Enter[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc};
//ʧ�ܵ��
uint8_t LK_Motor_CAN_Message_Exit[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd};
//���浱ǰ���λ��Ϊ���
uint8_t LK_Motor_CAN_Message_Save_Zero[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe};

uint8_t Start_Flag = 0;
uint32_t Flag = 0;
uint32_t Pre_Flag = 0;
int32_t Position_Max = 65535;
float Current_Max = 850;
Enum_LK_Motor_Status LK_Motor_Status = LK_Motor_Status_DISABLE;
Struct_LK_Motor_Rx_Data Motor_Left;
Struct_LK_Motor_Rx_Data Motor_Right;
float voltage         = 0;
/* Private function declarations ---------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/**

/**
 * @brief ���ݴ������
 *
 */
void Data_Process(uint8_t* CAN_Rx_Buffer,Struct_LK_Motor_Rx_Data* Data)
{
    //���ݴ������
    int32_t delta_encoder;
    uint16_t tmp_encoder;
    int16_t tmp_current, tmp_omega;
    Struct_LK_Motor_CAN_Rx_Data tmp_buffer = {0};
//    printf("%d",sizeof(tmp_buffer));
    memcpy((void *)&tmp_buffer,(void *)CAN_Rx_Buffer,8);

    if(CAN_Rx_Buffer[0]==0x9a)
    {
        voltage=(CAN_Rx_Buffer[3]<<8|CAN_Rx_Buffer[2])/100.0;
    }
    else {
        tmp_encoder = tmp_buffer.Encoder_Reverse;
           tmp_omega = tmp_buffer.Omega_Reverse;
           tmp_current = tmp_buffer.Current_Reverse;

           //����Ȧ�����ܽǶ�ֵ
           if(Data->Start_Flag==1)
           {
               delta_encoder = tmp_encoder - Data->Pre_Encoder;
               if (delta_encoder < -(Position_Max / 2))
               {
                   //������ת����һȦ
                   Data->Total_Round++;
               }
               else if (delta_encoder > (Position_Max / 2))
               {
                   //������ת����һȦ
                   Data->Total_Round--;
               }
           }

           Data->Total_Encoder = Data->Total_Round * Position_Max + tmp_encoder;

           //������������Ϣ
           Data->CMD_ID = tmp_buffer.CMD_ID;
           Data->Now_Angle = (float)Data->Total_Encoder / (float)Position_Max *360.0f;
           Data->Now_Radian = Data->Now_Angle * DEG_TO_RAD;
           Data->Now_Omega_Angle = tmp_omega;
           Data->Now_Omega_Radian = tmp_omega * DEG_TO_RAD;
           Data->Now_Power = tmp_current;
           Data->Now_Temperature = tmp_buffer.Temperature_Centigrade;

           //�洢Ԥ����Ϣ
           Data->Pre_Encoder = tmp_encoder;
           if(Data->Start_Flag==0)   Data->Start_Flag = 1;
    }

}

///**
// * @brief CANͨ�Ž��ջص�����
// *
// * @param Rx_Data ���յ�����
// */
//void CAN_RxCpltCallback(uint8_t *Rx_Data)
//{
//    //��������, �жϵ���Ƿ�����
//    Flag += 1;
//
//    Data_Process(Rx_Data);
//}

///**
// * @brief TIM��ʱ���ж϶��ڼ�����Ƿ���
// *
// */
//void TIM_Alive_PeriodElapsedCallback()
//{
//    //�жϸ�ʱ������Ƿ���չ��������
//    if (Flag == Pre_Flag)
//    {
//        //����Ͽ�����
//        LK_Motor_Status = LK_Motor_Status_DISABLE;
//    }
//    else
//    {
//        //�����������
//        LK_Motor_Status = LK_Motor_Status_ENABLE;
//    }
//
//    Pre_Flag = Flag;
//}

/**
 * @brief ������ӳ�䵽������
 *
 * @param x ����
 * @param Int_Min ������Сֵ
 * @param Int_Max �������ֵ
 * @param Float_Min ��������Сֵ
 * @param Float_Max ���������ֵ
 * @return float ������
 */
float Math_Int_To_Float(int32_t x, int32_t Int_Min, int32_t Int_Max, float Float_Min, float Float_Max)
{
    float tmp = (float)(x - Int_Min) / (float)(Int_Max - Int_Min);
    float out = tmp * (Float_Max - Float_Min) + Float_Min;
    return (out);
}

/**
 * @brief 16λ��С��ת��
 *
 * @param Source Դ���ݵ�ַ
 * @param Destination Ŀ��洢��ַ
 */
void Math_Endian_Reverse_16(void *Source, void *Destination)
{
    uint8_t *temp_source, *temp_destination;
    temp_source = (uint8_t *)Source;
    temp_destination = (uint8_t *)Destination;

    temp_destination[0] = temp_source[1];
    temp_destination[1] = temp_source[0];
}

/*********************************************************************
 * @fn      CAN_Mode_Init
 *
 * @brief   Initializes CAN communication test mode.
 *          Bps =Fpclk1/((tpb1+1+tbs2+1+1)*brp)
 *
 * @param   tsjw - CAN synchronisation jump width.
 *          tbs2 - CAN time quantum in bit segment 1.
 *          tbs1 - CAN time quantum in bit segment 2.
 *          brp - Specifies the length of a time quantum.
 *          mode - Test mode.
 *            CAN_Mode_Normal.
 *            CAN_Mode_LoopBack.
 *            CAN_Mode_Silent.
 *            CAN_Mode_Silent_LoopBack.
 *
 * @return  none
 */
void CAN_Mode_Init( u8 tsjw, u8 tbs2, u8 tbs1, u16 brp, u8 mode )
{
    GPIO_InitTypeDef GPIO_InitSturcture={0};
    CAN_InitTypeDef CAN_InitSturcture={0};
    CAN_FilterInitTypeDef CAN_FilterInitSturcture={0};
    NVIC_InitTypeDef NVIC_InitStructure={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE );
    //RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_CAN1, ENABLE );

    GPIO_PinRemapConfig( GPIO_Remap1_CAN1, ENABLE);

    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_9;
    //GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitSturcture.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init( GPIOB, &GPIO_InitSturcture);
    //GPIO_Init( GPIOA, &GPIO_InitSturcture);

    GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_8;
    //GPIO_InitSturcture.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitSturcture.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init( GPIOB, &GPIO_InitSturcture);
    //GPIO_Init( GPIOA, &GPIO_InitSturcture);

    CAN_InitSturcture.CAN_TTCM = DISABLE;
    CAN_InitSturcture.CAN_ABOM = DISABLE;
    CAN_InitSturcture.CAN_AWUM = DISABLE;
    CAN_InitSturcture.CAN_NART = ENABLE;
    CAN_InitSturcture.CAN_RFLM = DISABLE;
    CAN_InitSturcture.CAN_TXFP = DISABLE;
    CAN_InitSturcture.CAN_Mode = mode;
    CAN_InitSturcture.CAN_SJW = tsjw;
    CAN_InitSturcture.CAN_BS1 = tbs1;
    CAN_InitSturcture.CAN_BS2 = tbs2;
    CAN_InitSturcture.CAN_Prescaler = brp;
    CAN_Init( CAN1, &CAN_InitSturcture );

  /* identifier/mask mode, One 32-bit filter, ExtId: 0x12124567 */
    CAN_FilterInitSturcture.CAN_FilterNumber = 0;
    CAN_FilterInitSturcture.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitSturcture.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitSturcture.CAN_FilterIdHigh = 0x0000;
    CAN_FilterInitSturcture.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitSturcture.CAN_FilterMaskIdHigh = 0x0000;
    CAN_FilterInitSturcture.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitSturcture.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    CAN_FilterInitSturcture.CAN_FilterActivation = ENABLE;
    CAN_FilterInit( &CAN_FilterInitSturcture );
    CAN_TTComModeCmd( CAN1, ENABLE );

    CAN_ClearITPendingBit( CAN1, CAN_IT_FMP0 );
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    CAN_ITConfig( CAN1, CAN_IT_FMP0, ENABLE );

}


/*********************************************************************
 * @fn      CAN_Send_Msg
 *
 * @brief   CAN Transmit function.
 *
 * @param   msg - Transmit data buffer.
 *          len - Data length.
 *
 * @return  0 - Send successful.
 *          1 - Send failed.
 */
u8 CAN_Send_Msg( u8 *msg, u8 len ,uint32_t id)
{
    u8 mbox;
    u16 i = 0;

    CanTxMsg CanTxStructure;

    CanTxStructure.StdId = id;
    CanTxStructure.IDE = CAN_Id_Standard;
    CanTxStructure.RTR = CAN_RTR_Data;
    CanTxStructure.DLC = len;

    for( i=0; i<len; i++ )
    {
        CanTxStructure.Data[i] = msg[i];
    }

    mbox = CAN_Transmit( CAN1, &CanTxStructure);
    i = 0;

    while( ( CAN_TransmitStatus( CAN1, mbox ) != CAN_TxStatus_Ok ) && (i < 0xFFF) )
    {
        i++;
    }

    if( i == 0xFFF )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



/*********************************************************************
 * @fn      CAN_Receive_Msg
 *
 * @brief   CAN Receive function.
 *
 * @param   buf - Receive data buffer.
 *          len - Data length.
 *
 * @return  CanRxStructure.DLC - Receive data length.
 */
u8 CAN_Receive_Msg( u8 *buf )
{
    u8 i;

    CanRxMsg CanRxStructure;

    if( CAN_MessagePending( CAN1, CAN_FIFO0 ) == 0)
    {
        return 0;
    }

    CAN_Receive( CAN1, CAN_FIFO0, &CanRxStructure );

    for( i=0; i<8; i++ )
    {
        buf[i] = CanRxStructure.Data[i];
    }

    return CanRxStructure.DLC;

}

/*********************************************************************
 * @fn      USB_LP_CAN1_RX0_IRQHandler
 *
 * @brief   This function handles CAN receive FIFO0 Handler.
 *
 * @return  none
 */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    u8 rxbuf[8];
    CanRxMsg CanRxStructure;
    //����fifo��ʣ�����Ϣ������Ϊ0
    if( CAN_MessagePending( CAN1, CAN_FIFO0 ) != 0)
    {
        //ȡ����һ����Ϣ
        CAN_Receive( CAN1, CAN_FIFO0, &CanRxStructure);
        //ȡ����Ϣ
        for(int i=0;i<CanRxStructure.DLC;i++){
            rxbuf[i] = CanRxStructure.Data[i];
        }
        //������Ϣid����ͬ�������Ϣ
        switch (CanRxStructure.StdId) {
            case 0x141:
                //�������/memcpy

                    Data_Process(rxbuf, &Motor_Left);

                break;
            case 0x142:
                //�������/memcpy


                Data_Process(rxbuf, &Motor_Right);

                break;
            case 0x200:
                //����id

                break;
        }
    }
    CAN_ClearITPendingBit( CAN1, CAN_IT_FMP0 );
}

//void CAN1_RX1_IRQHandler(void)
//{
//    u8 rxbuf[8];
//    CanRxMsg CanRxStructure;
//    //����fifo��ʣ�����Ϣ������Ϊ0
//    if( CAN_MessagePending( CAN1, CAN_FIFO0 ) != 0)
//    {
//        //ȡ����һ����Ϣ
//        CAN_Receive( CAN1, CAN_FIFO0, &CanRxStructure);
//        //ȡ����Ϣ
//        for(int i=0;i<CanRxStructure.DLC;i++){
//            rxbuf[i] = CanRxStructure.Data[i];
//        }
//        //������Ϣid����ͬ�������Ϣ
//        switch (CanRxStructure.StdId) {
//            case 0x141:
//                //�������/memcpy
//                break;
//            case 0x142:
//                //�������/memcpy
//                break;
//        }
//    }
//    CAN_ClearITPendingBit( CAN1, CAN_IT_FMP0 );
//}

/************************ COPYRIGHT(C) USTC-ROBOWALKER **************************/

