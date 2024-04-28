/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "sthg"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "ShgThg.h"
/********************************************************************************************
函数名：STHG_Init
功能  ：设置二/三倍频使能引脚
参数  ：无
返回值：无
*********************************************************************************************/ 
void STHG_Init(void)
{
    rt_pin_mode(SHT_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(THG_ENA_PIN, PIN_MODE_OUTPUT);
	SHG_ENA_OFF(); 
	THG_ENA_OFF();
}

/********************************************************************************************
函数名：STHG_TEMP_READ
功能  ：读取二/三倍频温度值
参数  ：：arr[3][8]： 二维数首地址
		row：行
		column：列
返回值：TempBuf
*********************************************************************************************/ 
rt_uint16_t STHG_TEMP_READ(rt_uint16_t Arrval)
{
	rt_uint16_t ValueBuf,TempBuf;
	
	ValueBuf  = Arrval;
	
	TempBuf = ((double)(ValueBuf) / 4096 * 3.3 * 6.67 + 15) * 100;
	
	return TempBuf;
}
/********************************************************************************************
函数名：STHG_TEMP_SET
功能  ：设置二/三倍频温度值
参数  ：value： 温度值
		Channel：DAC7568通道值
返回值：TempBuf
*********************************************************************************************/ 
void STHG_TEMP_SET(rt_uint32_t value,rt_uint8_t Channel)
{
	
}

