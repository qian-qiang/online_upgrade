/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "cbm128"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "cbm128s085.h"

/********************************************************************************************
函数名：DAC_CBM128S085_Init
功能  ：初始化CBM128S085控制引脚
参数  ：无
返回值：无
*********************************************************************************************/ 
void DAC_CBM128S085_Init(void)
{
    rt_pin_mode(DAC_SCLK_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DAC_DIN_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DAC_SYNC_PIN, PIN_MODE_OUTPUT);
 
	//CBM128S085/CBM108S085 内部同时含有上电复位和断电复位电路
	CBM_WriteControlModeSelect(WTM);
	
}
/********************************************************************************************
函数名：CBM_WriteByte
功能  ：写入一个字节数据
参数  ：一个字节数据
返回值：无
*********************************************************************************************/ 
void CBM_WriteByte(rt_uint8_t txd)
{
    rt_uint8_t t;
	rt_uint16_t d;
    for(t=0;t<8;t++)
    { 
        //CLK下降沿发送数据
        rt_pin_write(DAC_SCLK_PIN, PIN_HIGH);
        rt_pin_write(DAC_DIN_PIN, (txd&0x80)>>7);
        txd<<=1; 	    //txd = txd<<1;
		rt_pin_write(DAC_SCLK_PIN, PIN_LOW);
		for(d=1;d>0;d--);    //延迟
    }	 
}
/********************************************************************************************
函数名：DAC_CBM128_Channel_Vol
功能  ：设置CBM128S选定通道电压值
参数  ：Channel通道值，Vol电压值0-3300
返回值：无
*********************************************************************************************/
void DAC_CBM128_Channel_Vol(rt_uint8_t Channel,rt_uint16_t Vol)
{
	rt_uint16_t tmp = Vol;//2500
	tmp = (float)tmp/3300*4095;  //12位
	CBM_WriteFrameData(Channel,tmp);
}

/********************************************************************************************
函数名：CBM_WriteFrameData
功能  ：往CBM128选定通道写入DAC值
参数  ：Channel通道值，Data数据值
返回值：无
*********************************************************************************************/ 
void CBM_WriteFrameData(rt_uint8_t Channel,rt_uint16_t Data)
{
    rt_pin_write(DAC_SYNC_PIN, PIN_LOW);
	//CBM_WriteByte((u8(Data>>8)) | (Channel<<4));
	CBM_WriteByte((Data>>8)|(Channel<<4));//DB[15:12] 0-7代表通道  0000 xxxx | xxxx 0000
	CBM_WriteByte((rt_uint8_t)Data);//数据的低8位 DB[7:0]
    rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
}
/********************************************************************************************
函数名：CBM_WriteControlModeSelect
功能  ：模式控制方式选择
参数  ：ControlMode
				arg:WTM->直接更新ChannelDAC寄存器的值
						WRM->仅写入Channel数据寄存器
返回值：无
*********************************************************************************************/ 
void CBM_WriteControlModeSelect(rt_uint8_t ControlMode)
{
		rt_uint8_t Mode = ControlMode;
		if(Mode == WTM)
		{
			rt_pin_write(DAC_SYNC_PIN, PIN_LOW);
			CBM_WriteByte(0x90);//[15:12]-->1001
			CBM_WriteByte(0x00);
			rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
		}
		else if(Mode == WRM)
		{
			rt_pin_write(DAC_SYNC_PIN, PIN_LOW);
			CBM_WriteByte(0x80);//[15:12]-->1000
			CBM_WriteByte(0x00);
			rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
		}
		
}
