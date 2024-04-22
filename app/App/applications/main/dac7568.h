#ifndef _DAC7568_H
#define _DAC7568_H

#include <rtthread.h>
#include <board.h>

#define     Channel_A           0
#define     Channel_B           1
#define     Channel_C           2
#define     Channel_D           3
#define     Channel_E           4
#define     Channel_F           5
#define     Channel_G           6
#define     Channel_H           7

void DAC7568_Init(void);
void Dac7568_Channel_Vol(rt_uint8_t Channel, rt_uint16_t Vol);
void DAC7568_Reset(void);
#endif
