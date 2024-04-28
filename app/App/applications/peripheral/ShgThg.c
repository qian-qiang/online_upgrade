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
��������STHG_Init
����  �����ö�/����Ƶʹ������
����  ����
����ֵ����
*********************************************************************************************/ 
void STHG_Init(void)
{
    rt_pin_mode(SHT_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(THG_ENA_PIN, PIN_MODE_OUTPUT);
	SHG_ENA_OFF(); 
	THG_ENA_OFF();
}

/********************************************************************************************
��������STHG_TEMP_READ
����  ����ȡ��/����Ƶ�¶�ֵ
����  ����arr[3][8]�� ��ά���׵�ַ
		row����
		column����
����ֵ��TempBuf
*********************************************************************************************/ 
rt_uint16_t STHG_TEMP_READ(rt_uint16_t Arrval)
{
	rt_uint16_t ValueBuf,TempBuf;
	
	ValueBuf  = Arrval;
	
	TempBuf = ((double)(ValueBuf) / 4096 * 3.3 * 6.67 + 15) * 100;
	
	return TempBuf;
}
/********************************************************************************************
��������STHG_TEMP_SET
����  �����ö�/����Ƶ�¶�ֵ
����  ��value�� �¶�ֵ
		Channel��DAC7568ͨ��ֵ
����ֵ��TempBuf
*********************************************************************************************/ 
void STHG_TEMP_SET(rt_uint32_t value,rt_uint8_t Channel)
{
	
}

