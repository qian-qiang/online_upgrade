#ifndef _TIMER_H
#define _TIMER_H

#include <rtthread.h>
#include "protocol_id.h"
#include "board.h"

#define BEEP_ON()   rt_pin_write(BEEP_PIN, PIN_HIGH)
#define BEEP_OFF()	rt_pin_write(BEEP_PIN, PIN_LOW)

extern rt_uint8_t LASER_ON_FLAG; 
extern rt_uint8_t USER_TIME_FLAG[3];
extern rt_timer_t timer_led;
extern rt_timer_t timer_beep;

int timer_init(void);
#endif
