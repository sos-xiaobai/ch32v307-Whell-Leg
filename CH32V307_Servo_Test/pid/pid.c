#include "pid.h"

//��������pid�ṹ��ĺ���
void PID_Clear(PID *pid)
{
    pid->error = 0;
    pid->lastError = 0;
    pid->integral = 0;
    pid->output = 0;
}

//���ڳ�ʼ��pid�����ĺ���
void PID_Init(PID *pid, float p, float i, float d, float maxI, float maxOut)
{
    pid->kp = p;
    pid->ki = i;
    pid->kd = d;
    pid->maxIntegral = maxI;
    pid->maxOutput = maxOut;
    PID_Clear(pid);
}

//����һ��pid����
//����Ϊ(pid�ṹ��,Ŀ��ֵ,����ֵ)������������pid�ṹ���output��Ա��
void PID_Calc(PID *pid, float reference, float feedback)
{
    //��������
    pid->lastError = pid->error; //����error������
    pid->error = reference - feedback; //������error
    //����΢��
    pid->dout = (pid->error - pid->lastError) * pid->kd;
    //�������
    float pout = pid->error * pid->kp;
    //�������
    pid->integral += pid->error * pid->ki;
    //�����޷�
    if(pid->integral > pid->maxIntegral) pid->integral = pid->maxIntegral;
    else if(pid->integral < -pid->maxIntegral) pid->integral = -pid->maxIntegral;
    //�������
    pid->output = pout+pid->dout + pid->integral;
    //����޷�
    if(pid->output > pid->maxOutput) pid->output =   pid->maxOutput;
    else if(pid->output < -pid->maxOutput) pid->output = -pid->maxOutput;
}


