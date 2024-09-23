#ifndef _PID_H_
#define _PID_H_

//���ȶ���PID�ṹ�����ڴ��һ��PID������
typedef struct
{
    float kp, ki, kd; //����ϵ��
    float error, lastError; //���ϴ����
    float integral, maxIntegral; //���֡������޷�
    float output, maxOutput; //���������޷�
}PID;


void PID_Init(PID *pid, float p, float i, float d, float maxI, float maxOut);
void PID_Calc(PID *pid, float reference, float feedback);


#endif
