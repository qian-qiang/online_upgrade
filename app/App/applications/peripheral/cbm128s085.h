#ifndef CBM128S085_H
#define CBM128S085_H

#include <rtthread.h>
#include <board.h>

/*
	8��ͨ��,VREF�ο���ѹ���ⲿ����
	Vout = Vref * (D/N)
	D:���������ź� �ɴ��нӿ�д���ڲ�DAC�Ĵ���
	N:12λ 4096.
*/ 
//ABCDͨ������VREF1�ο���ѹ
#define     Channel_A           0
#define     Channel_B           1
#define     Channel_C           2
#define     Channel_D           3
//EFGHͨ������VREF2�ο���ѹ
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

