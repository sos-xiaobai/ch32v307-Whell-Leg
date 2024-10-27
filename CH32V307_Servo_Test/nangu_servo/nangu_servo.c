#include "nangu_servo.h"

/* PWM Output Mode Definition */
#define PWM_MODE1   0
#define PWM_MODE2   1

/* PWM Output Mode Selection */
#define PWM_MODE PWM_MODE1
//#define PWM_MODE PWM_MODE2


/*********************************************************************
 * @fn      TIM2_OutCompare_Init
 *
 * @brief   Initializes TIM2 output compare.
 *          PA0-CH1 左舵机
 *          PA1-CH2 右舵机
 *
 * @param   arr - the period value.
 *          psc - the prescaler value.
 *          ccp - the pulse value.
 *
 * @return
 */
void TIM2_PWMOut_Init( u16 psc,u16 arr, u16 ccp)
{
    GPIO_InitTypeDef GPIO_InitStructure               = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure             = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_Period        = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler     = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);


    TIM_ARRPreloadConfig(TIM2, ENABLE);


#if (PWM_MODE == PWM_MODE1)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;

#elif (PWM_MODE == PWM_MODE2)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;

#endif

    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = ccp;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);

    //TIM_CtrlPWMOutputs(TIM2, ENABLE);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);



    TIM_Cmd(TIM2, ENABLE);
}



void nangu_servo_init(void)
{
    TIM2_PWMOut_Init( 144-1, 20000-1, 1000 );

}



// 角度转换为PWM值的函数保持不变
uint16_t angle_to_pwm(float angle)
{
    return 1500 + (angle * 2000.0 / 180.0);
}

// 修改后的设置舵机角度函数
void set_servo_angle(ServoChannel channel, float angle)
{
    if (angle > 180) angle = 180;
    uint16_t pwm_value = angle_to_pwm(angle);

    switch (channel) {
        case SERVO_CHANNEL_1:
            TIM_SetCompare1(TIM2, pwm_value);
            break;
        case SERVO_CHANNEL_2:
            TIM_SetCompare2(TIM2, pwm_value);
            break;
        case SERVO_CHANNEL_3:
            TIM_SetCompare3(TIM2, pwm_value);
            break;
        case SERVO_CHANNEL_4:
            TIM_SetCompare4(TIM2, pwm_value);
            break;
        default:
            // 处理错误情况
            break;
    }
}
