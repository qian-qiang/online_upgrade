#ifndef _PD_H_
#define _PD_H_
 
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

typedef struct
{
    rt_uint16_t _adcMax;
	rt_uint16_t _adcMin;
    rt_uint16_t _powerMax;
	rt_uint16_t _powerMin;
    rt_uint16_t _power;
	rt_uint16_t _adcVal;
	
}TydefStruct;

rt_uint16_t LaserPowerRead(rt_uint16_t PD_AdcVal,rt_uint16_t SQ_state, rt_uint16_t SQ_RF_scale, rt_uint16_t SQ_div, rt_uint16_t factor); 

rt_uint16_t UvLaserPowerRead(rt_uint16_t Adcval, rt_uint16_t Adcmax, rt_uint16_t Adcmin, rt_uint16_t Pmax, rt_uint16_t Pmin, rt_uint32_t divider);

rt_uint16_t IRLaserPowerRead(rt_uint16_t Adcval, rt_uint16_t Adcmax, rt_uint16_t Adcmin, rt_uint16_t Pmax, rt_uint16_t Pmin);

#endif

