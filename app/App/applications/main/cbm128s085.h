#ifndef CBM128S085_H
#define CBM128S085_H

#include <rtthread.h>
#include <board.h>

/*
	8个通道,VREF参考电压由外部输入
	Vout = Vref * (D/N)
	D:输入数字信号 由串行接口写入内部DAC寄存器
	N:12位 4096.
*/ 
//ABCD通道采用VREF1参考电压
#define     Channel_A           0
#define     Channel_B           1
#define     Channel_C           2
#define     Channel_D           3
//EFGH通道采用VREF2参考电压
#define     Channel_E           4
#define     Channel_F           5
#define     Channel_G           6
#define     Channel_H           7

#define 	WRM                  1
#define 	WTM                	2

void CBM_WriteControlModeSelect(rt_uint8_t ControlMode);
void DAC_CBM128S085_Init(void);
void CBM_WriteByte(rt_uint8_t txd);
void CBM_WriteFrameData(rt_uint8_t Channel,rt_uint16_t Data);
void DAC_CBM128_Channel_Vol(rt_uint8_t Channel,rt_uint16_t Vol);

#endif

