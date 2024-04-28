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
��������DAC_CBM128S085_Init
����  ����ʼ��CBM128S085��������
����  ����
����ֵ����
*********************************************************************************************/ 
void DAC_CBM128S085_Init(void)
{
    rt_pin_mode(DAC_SCLK_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DAC_DIN_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(DAC_SYNC_PIN, PIN_MODE_OUTPUT);
 
	//CBM128S085/CBM108S085 �ڲ�ͬʱ�����ϵ縴λ�Ͷϵ縴λ��·
	CBM_WriteControlModeSelect(WTM);
	
}
/********************************************************************************************
��������CBM_WriteByte
����  ��д��һ���ֽ�����
����  ��һ���ֽ�����
����ֵ����
*********************************************************************************************/ 
void CBM_WriteByte(rt_uint8_t txd)
{
    rt_uint8_t t;
	rt_uint16_t d;
    for(t=0;t<8;t++)
    { 
        //CLK�½��ط�������
        rt_pin_write(DAC_SCLK_PIN, PIN_HIGH);
        rt_pin_write(DAC_DIN_PIN, (txd&0x80)>>7);
        txd<<=1; 	    //txd = txd<<1;
		rt_pin_write(DAC_SCLK_PIN, PIN_LOW);
		for(d=1;d>0;d--);    //�ӳ�
    }	 
}
/********************************************************************************************
��������DAC_CBM128_Channel_Vol
����  ������CBM128Sѡ��ͨ����ѹֵ
����  ��Channelͨ��ֵ��Vol��ѹֵ0-3300
����ֵ����
*********************************************************************************************/
void DAC_CBM128_Channel_Vol(rt_uint8_t Channel,rt_uint16_t Vol)
{
	rt_uint16_t tmp = Vol;//2500
	tmp = (float)tmp/3300*4095;  //12λ
	CBM_WriteFrameData(Channel,tmp);
}

/********************************************************************************************
��������CBM_WriteFrameData
����  ����CBM128ѡ��ͨ��д��DACֵ
����  ��Channelͨ��ֵ��Data����ֵ
����ֵ����
*********************************************************************************************/ 
void CBM_WriteFrameData(rt_uint8_t Channel,rt_uint16_t Data)
{
    rt_pin_write(DAC_SYNC_PIN, PIN_LOW);
	//CBM_WriteByte((u8(Data>>8)) | (Channel<<4));
	CBM_WriteByte((Data>>8)|(Channel<<4));//DB[15:12] 0-7����ͨ��  0000 xxxx | xxxx 0000
	CBM_WriteByte((rt_uint8_t)Data);//���ݵĵ�8λ DB[7:0]
    rt_pin_write(DAC_SYNC_PIN, PIN_HIGH);
}
/********************************************************************************************
��������CBM_WriteControlModeSelect
����  ��ģʽ���Ʒ�ʽѡ��
����  ��ControlMode
				arg:WTM->ֱ�Ӹ���ChannelDAC�Ĵ�����ֵ
						WRM->��д��Channel���ݼĴ���
����ֵ����
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
