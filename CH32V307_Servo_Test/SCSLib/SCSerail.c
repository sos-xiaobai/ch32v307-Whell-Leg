/*
 * SCServo.c
 * 飞特舵机硬件接口层程序
 * 日期: 2022.3.29
 * 作者: 
 */

#include "uart.h"

uint32_t IOTimeOut = 5000;//通信超时
uint8_t wBuf[128];
uint8_t wLen = 0;

int readSCSTimeOut(unsigned char *nDat, int nLen, uint32_t TimeOut)
{
	int Size = 0;
	int ComData;
	uint32_t t_user = 0;
	while(1){
		ComData = Uart_Read();
		if(ComData!=-1){
			if(nDat){
				nDat[Size] = ComData;
			}
			Size++;
		}
		if(Size>=nLen){
			break;
		}
		t_user++;
		if(t_user>TimeOut){
			break;
		}
	}
	return Size;
}

//UART 接收数据接口
int readSCS(unsigned char *nDat, int nLen)
{
	int Size = 0;
	int ComData;
	uint32_t t_user = 0;
	while(1){
		ComData = Uart_Read();
		if(ComData!=-1){
			if(nDat){
				nDat[Size] = ComData;
			}
			Size++;
			t_user = 0;
		}
		if(Size>=nLen){
			break;
		}
		t_user++;
		if(t_user>IOTimeOut){
			break;
		}
	}
	return Size;
}

//UART 发送数据接口
int writeSCS(unsigned char *nDat, int nLen)
{
	while(nLen--){
		if(wLen<sizeof(wBuf)){
			wBuf[wLen] = *nDat;
			wLen++;
			nDat++;
		}
	}
	return wLen;
}

int writeByteSCS(unsigned char bDat)
{
	if(wLen<sizeof(wBuf)){
		wBuf[wLen] = bDat;
		wLen++;
	}
	return wLen;
}

//总线切换延时
void SCSDelay(void)
{
	uint8_t i = 180;
	while(i--){}//0.056*i(us)
}

//接收缓冲区刷新
void rFlushSCS()
{
	SCSDelay();
	Uart_Flush();
}

//发送缓冲区刷新
void wFlushSCS()
{
	if(wLen){
		Uart_Send(wBuf, wLen);
		wLen = 0;
	}
}

