#ifndef _FAN_H
#define _FAN_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define FAN_EN_ON()     rt_pin_write(FAN_PIN,PIN_HIGH)
#define FAN_EN_OFF()    rt_pin_write(FAN_PIN,PIN_LOW)

void Fan_Init(void); 
void Set_FanFreq(void);
void Set_FanDuty(void);

#endif
