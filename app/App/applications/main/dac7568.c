/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "dac7568"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "dac7568.h"

/********************************************************************************************
函数名：DAC7568_Init
功能  ：初始化DAC7568控制引脚
参数  ：无
返回值：无
*********************************************************************************************/ 
void DAC7568_Init(void)
{
    rt_pin_mode(DAC_SCLK_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DAC_DIN_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DAC_SYNC_PIN, PIN_MODE_OUTPUT);
    
	DAC7568_Reset();
    rt_pin_write(DAC_SCLK_PIN, PIN_HIGH);
    rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
}

/********************************************************************************************
函数名：WriteByte
功能  ：写入一个字节数据
参数  ：无
返回值：无
*********************************************************************************************/ 
void WriteByte(rt_uint8_t txd)
{
	rt_uint8_t t;
	rt_uint16_t d;
	//下降沿发送数据
    for(t=0;t<8;t++)
    { 
	  rt_pin_write(DAC_SCLK_PIN, PIN_HIGH);	
      rt_pin_write(DAC_DIN_PIN, (txd&0x80)>>7);
      txd<<=1; 	  //左移一位后赋值给txd  
//	  for(d=1;d>0;d--);
	  rt_pin_write(DAC_SCLK_PIN, PIN_LOW);
	  for(d=1;d>0;d--);    //延迟
    }	 
}

/********************************************************************************************
函数名：WriteFrameData
功能  ：往DAC7568选定通道写入DAC值
参数  ：Channel通道值，Data数据值
返回值：无
*********************************************************************************************/ 
void WriteFrameData(rt_uint8_t Channel,rt_uint16_t Data)
{
	rt_pin_write(DAC_SYNC_PIN, PIN_LOW);
	WriteByte(0x03);
	WriteByte( (rt_uint8_t)( Data >> 8 ) | ( Channel << 4 ) );                      
	WriteByte( (rt_uint8_t)Data );
	WriteByte(0x00);
	rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
}

/********************************************************************************************
函数名：Dac7568_Channel_Vol
功能  ：设置DAC7568选定通道电压值
参数  ：Channel通道值，Vol电压值
返回值：无
*********************************************************************************************/
void Dac7568_Channel_Vol(rt_uint8_t Channel,rt_uint16_t Vol)
{
	rt_uint16_t temp=Vol;
	temp=(float)temp/3300*2048;
	WriteFrameData(Channel,temp);
}

/********************************************************************************************
函数名：DAC7568_Reset
功能  ：执行DAC7568复位
参数  ：无
返回值：无
*********************************************************************************************/
void DAC7568_Reset(void) 
{
	rt_pin_write(DAC_SYNC_PIN, PIN_LOW);
	WriteByte(0x07);
	WriteByte(0x00);                      
	WriteByte(0x00);
	WriteByte(0x00);
	rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
}


