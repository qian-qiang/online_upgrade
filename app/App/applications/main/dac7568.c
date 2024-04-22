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
��������DAC7568_Init
����  ����ʼ��DAC7568��������
����  ����
����ֵ����
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
��������WriteByte
����  ��д��һ���ֽ�����
����  ����
����ֵ����
*********************************************************************************************/ 
void WriteByte(rt_uint8_t txd)
{
	rt_uint8_t t;
	rt_uint16_t d;
	//�½��ط�������
    for(t=0;t<8;t++)
    { 
	  rt_pin_write(DAC_SCLK_PIN, PIN_HIGH);	
      rt_pin_write(DAC_DIN_PIN, (txd&0x80)>>7);
      txd<<=1; 	  //����һλ��ֵ��txd  
//	  for(d=1;d>0;d--);
	  rt_pin_write(DAC_SCLK_PIN, PIN_LOW);
	  for(d=1;d>0;d--);    //�ӳ�
    }	 
}

/********************************************************************************************
��������WriteFrameData
����  ����DAC7568ѡ��ͨ��д��DACֵ
����  ��Channelͨ��ֵ��Data����ֵ
����ֵ����
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
��������Dac7568_Channel_Vol
����  ������DAC7568ѡ��ͨ����ѹֵ
����  ��Channelͨ��ֵ��Vol��ѹֵ
����ֵ����
*********************************************************************************************/
void Dac7568_Channel_Vol(rt_uint8_t Channel,rt_uint16_t Vol)
{
	rt_uint16_t temp=Vol;
	temp=(float)temp/3300*2048;
	WriteFrameData(Channel,temp);
}

/********************************************************************************************
��������DAC7568_Reset
����  ��ִ��DAC7568��λ
����  ����
����ֵ����
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


