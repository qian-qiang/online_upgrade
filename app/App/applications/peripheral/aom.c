/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 *
 * FQ:
 * PE10(H)--------------->FQPowerENA
 * FPGAIO17-------------->DIG_MOD
 * CBM128S085(VOUTF)----->ANA_MOD(����û��)
 *
 *
 * SQ:
 * SWR_ALARM___1  6___SQ_Power_24V
 * StandbyENA__2  7__|
 * SQ_FRE_O____3  8___GND
 * SQ_ATTEN_O__4  9___|
 *           __5  10__|\
 *
 * SWR_ALARM---------->PE12(�����֣���һ��û��)
 * StandbyENA--------->PE13
 * FPGAIO7------------>SQ_FRE_O
 * DAC7513------------>SQ_ATTEN_O(�ڿ�/���)
 * PE11--------------->SQ_Power_24V
 */
#define LOG_TAG   "aom"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "aom.h"
 
#define RF_POWER_NUM       15       //AOM2����Ƶ�����ܹ��ĵ�λ��

aom_t aom = {0};
static rt_thread_t tid_aom = RT_NULL;
//��һ�д���ٷֱ�ֵ
//�ڶ��д����Ӧ��DACֵ
//ʹ�÷ֶ��������ʶ�Ӧ��DACֵ
const rt_uint16_t aom2_table[3][RF_POWER_NUM] = {
    { 1000,  950,  900,  850,  800,  700,  600,  500,  400,  300,  200,  150,  100,   50, 0},
	{ 4000, 3600, 3400, 3300, 3200, 2800, 2600, 2400, 2200, 2000, 1800, 1650, 1500, 1300, 0},    //10W����AOM2���⹦�ʿ������� - ֻ��A��������������ݷ���
//	{ 4000, 3200, 2900, 2750, 2600, 2350, 2130, 1900, 1700, 1500, 1280, 1100,  980,  800, 0},    //30W����-����1
};

/********************************************************************************************
��������Aom_Init
����  ����ʼAOM����
����  ����
����ֵ����
*********************************************************************************************/
void Aom_Init(void)
{
    rt_pin_mode(SQ_SWR_ALARM_PIN, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(SQ_ATTEN_SEL_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(SQ_POWER_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(SQ_Standby_PIN, PIN_MODE_OUTPUT);
	SQ_STANDBY_ON();
	SQ_ATTEN_INT();
	SQ_POWER_OFF();
	
    rt_pin_mode(FQ_POWER_ENA_PIN, PIN_MODE_OUTPUT);
	FQ_POWER_OFF();
        
    aom.aom1PowerFlag = false;
}

void aom_monitor(void)
{
    static rt_bool_t aom_error = false;
    
    if(SQ_SWR_ALARM == 0)
	{	
        log_e("SQ PE12 error pin is low");
        rt_thread_mdelay(15000);
        if(SQ_SWR_ALARM == 0) 
        {
            log_e("delay 1.5s, seed PE12 error pin is also low");
            aom_error = true;
            error_insert(ERROR_ID_SQ);
        }
        else  
        {
            log_e("delay 1.5s, seed PE12 error pin change high");
        }
	}
    else
    {
        if(aom_error == true)
        {
            aom_error = false;
            error_delete(ERROR_ID_SQ);
        }
    }
}

static void thread_aom_entry(void *parameter)
{
    while (1)
    {
        aom_monitor();
        rt_thread_mdelay(100);
    }
}

int thread_aom_init(void)
{
    int result;
    
    tid_aom = rt_thread_create("aom",
                                   thread_aom_entry, RT_NULL,
                                   1024,
                                   15, 
                                   10);
    
    result = rt_thread_startup(tid_aom);
    if (result != RT_EOK)
    {
        rt_kprintf("init tid_aom failed.\n");
        return -1;
    }
    
    return result;
}
/******************************************************API***************************************************/

/********************************************************************************************
��������Aom1CtrlAct
����  ��AOM1��/���ж�
����  ��aom1PowerState��AOM1���µ��־
        seedLaserEnergy������Դ���⹦��
����ֵ����
*********************************************************************************************/
void Aom1CtrlAct(rt_uint8_t aom1PowerState,rt_uint8_t seedType, rt_uint8_t seedLaserEnergy)
{	
	if(aom1PowerState == true)					   
	{
		aom.aom1PowerFlag = true;
	}
	
	if(seedType == TOPTICA)
	{
		if((aom1PowerState == false) && (seedLaserEnergy < 10))    //����Դ����رյ�����£�������ִ��AOM1�ض�
		{
			aom.aom1PowerFlag = false;
		}
	}
	else if(seedType == NPI)  
	{
		if((aom1PowerState == false) && (seedLaserEnergy == true) )    //����Դ����رյ�����£�������ִ��AOM1�ض�
		{
			aom.aom1PowerFlag = false;
		}
	}
	Aom1SwAct(aom.aom1PowerFlag);
}
/********************************************************************************************
��������Aom1SwAct
����  ��ִ��AOM1��/�µ�
����  ��aom1PowerFlag����/�µ��־��1-�ϵ磬0-�ϵ�
����ֵ����
*********************************************************************************************/
void Aom1SwAct(rt_uint8_t aom1PowerFlag)
{
    static rt_bool_t aom1PowerFlag_re = 0xff;
    
    if(aom1PowerFlag_re != aom1PowerFlag)
    {
        aom1PowerFlag_re = aom1PowerFlag;
        if(aom1PowerFlag == true)
        {
            log_i("aom1 vcc on");
            FQ_POWER_ON();
        }
        else 
        {
            log_i("aom1 vcc off");
            FQ_POWER_OFF();
        }   
    }
}

/********************************************************************************************
��������Aom1RfSet
����  ��aom1 RF����
����  ��value�� 0~100%
		Channel��DAC7568ͨ��ֵ
����ֵ����
��ע  ��aom1�Ĺ�����������޷����ã�ֻ����aom1�ϵĻ�������������
*********************************************************************************************/
void Aom1RfSet(rt_uint16_t Value, rt_uint8_t Channel)
{
//    rt_uint16_t ValueBuf;
//	
//	ValueBuf = Value * 30;                               //����0~100��Ӧ��ѹ0~3000mV
//	
//	//Dac7568_Channel_Voln(Channel,ValueBuf); 
//	DAC_CBM128_Channel_Vol(Channel,ValueBuf);
}

/********************************************************************************************
��������Aom2CtrlAct
����  ��Aom2�����ж���ִ��
����  ��aom2PowerState    aom2��/�µ�״̬λ
		aom2StandbyState  aom2����״̬λ
		aom2RfInSelState  aom2��/���״̬λ
����ֵ����
*********************************************************************************************/
void Aom2CtrlAct(rt_uint8_t aom2PowerState, rt_uint8_t aom2StandbyState, rt_uint8_t aom2RfInSelState, rt_uint8_t aom2Type) 
{	
    aom.aom2_type = aom2Type;
    aom.aom2StandbyState = aom2StandbyState;
    aom.aom2PowerState = aom2PowerState;
    aom.aom2RfInSelState = aom2RfInSelState;
    
    if(aom.aom2_type != aom2Type)
    {
        aom.aom2_type = aom2Type;
        log_i("aom2 type set %s", (aom.aom2_type == GH)?"GH":"SPY");
    }
    
    if(aom.aom2StandbyState != aom2StandbyState)
    {
        aom.aom2StandbyState = aom2StandbyState;
        if(aom.aom2_type == GH)
        {
            if(aom.aom2StandbyState == false)
            {
                log_i("aom2 disstandby");
            }
            else{
                log_i("aom2 standby");
            }
        }
        else
        {
            if(aom.aom2StandbyState == false)
            {
                log_i("aom2 standby");
            }
            else{
                log_i("aom2 disstandby");
            }
        }            
    }
    
    if(aom.aom2PowerState != aom2PowerState)
    {
        aom.aom2PowerState = aom2PowerState;
        log_i("aom2 vcc %s", aom.aom2PowerState?"on":"off");
    }
    
    if(aom.aom2RfInSelState != aom2RfInSelState)
    {
        aom.aom2RfInSelState = aom2RfInSelState;
        log_i("aom2 power sel %s conctrl", aom.aom2RfInSelState?"out":"in");
    }
    
	if(aom2Type == GH)                //GH
	{
		if(aom2StandbyState == false)    				   
		{
			SQ_STANDBY_OFF();
		} 
		else
		{
			SQ_STANDBY_ON();
		}
	}
	else if(aom2Type == SPY)           //SPY
	{
		if(aom2StandbyState == false)    				   
		{
			SQ_STANDBY_ON();	
		} 
		else
		{
			SQ_STANDBY_OFF();
		}
	}
	
	if(aom2RfInSelState == false)         //�����
	{
		SQ_ATTEN_INT();
	}
	else
	{
		SQ_ATTEN_EXT();
	}
	
	if(aom2PowerState == true)   				   
	{
		SQ_POWER_ON();
	} 
	else
	{
		SQ_POWER_OFF(); 
	}	
}

/********************************************************************************************
��������aom2RfSet
����  ������AOM2��Ƶ����
����  ��factor : 0~1000
����ֵ��revalue, �·���FPGA��DACֵ
*********************************************************************************************/
rt_uint16_t aom2RfSet(rt_uint16_t factor)
{
    float k = 0;        //kֵ
    float b = 0;        //bֵ
	rt_uint16_t tempvalue, revalue;
	tempvalue = factor;
    aom.aom2PcSetPower = factor;
	
    //������ά����
	for(uint8_t i = 0; i < RF_POWER_NUM - 1; i++)// N-1��
	{
		//�ж��Ƿ��ڼ��㷶Χ��
		if(IsInSide(tempvalue, aom2_table[0][i+1], aom2_table[0][i]))
		{
			//����kֵ��bֵ
			k = (float)(aom2_table[1][i] - aom2_table[1][i+1])/(aom2_table[0][i] - aom2_table[0][i+1]);           //��Kֵ
			b = (float)(aom2_table[1][i] - k * aom2_table[0][i]);	                                              //��Bֵ
			break;
		}
	}
	revalue = k * tempvalue + b;                                                                                 //y = kx + b
	if(revalue > 4095)
	{
		revalue = 4095;
	}
    
    aom.aom2FpgaSetPower = revalue; 
	return (revalue);
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void aom_read(uint8_t argc, char **argv)
{
    rt_kprintf("aom.aom1PowerFlag   		= %s      \n", aom.aom1PowerFlag?"on":"off");
    rt_kprintf("aom.aom2_type    		    = %s      \n", (aom.aom2_type == GH)?"GH":"SPY");
    rt_kprintf("aom.aom2StandbyState        = %d      \n", aom.aom2StandbyState);
	rt_kprintf("aom.aom2PowerState          = %s      \n", aom.aom2PowerState?"on":"off");
    rt_kprintf("aom.aom2RfInSelState        = %d      \n", aom.aom2RfInSelState?"out":"in");
    rt_kprintf("aom.aom2PcSetPower          = %d      \n", aom.aom2PcSetPower);
    rt_kprintf("aom.aom2FpgaSetPower        = %d      \n", aom.aom2FpgaSetPower);
}
MSH_CMD_EXPORT(aom_read, aom read);

static void aom_set(uint8_t argc, char **argv)
{
    rt_uint32_t param;
    
    if(argc != 3)
    {
        rt_kprintf("cmd error . please eg : aom_set [aom1_vcc/aom2_vcc/aom2_standby/aom2_rfsel/aom2_power] [param]");   
    }
    else{
        param = atoi(argv[2]);
        
        if(strcmp(argv[1], "aom1_vcc") == 0)
        {
            Aom1SwAct(param);
        }
        else if(strcmp(argv[1], "aom2_vcc") == 0)
        {
            Aom2CtrlAct(param, aom.aom2StandbyState, aom.aom2RfInSelState, aom.aom2_type);
        }
        else if(strcmp(argv[1], "aom2_standby") == 0)
        {
            Aom2CtrlAct(aom.aom2PowerState, param, aom.aom2RfInSelState, aom.aom2_type);
        }
        else if(strcmp(argv[1], "aom2_rfsel") == 0)
        {
            Aom2CtrlAct(aom.aom2PowerState, aom.aom2StandbyState, param, aom.aom2_type);
        }
        else if(strcmp(argv[1], "aom2_power") == 0)
        {
            aom2RfSet(param);
        }
        else{
            rt_kprintf("cmd error . please eg : aom_set [aom1_vcc/aom2_vcc/aom2_standby/aom2_rfsel/aom2_power] [param]");
        }
    }
}
MSH_CMD_EXPORT(aom_set, aom set);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
