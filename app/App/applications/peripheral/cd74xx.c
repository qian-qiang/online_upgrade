/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "cd74"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "cd74xx.h"
#include "thermistor.h" 

#define ADC1_DEV_NAME        "adc1"     
#define ADC2_DEV_NAME        "adc2"     
#define ADC3_DEV_NAME        "adc3"    
#define REFER_VOLTAGE       330         /* �ο���ѹ 3.3V,���ݾ��ȳ���100����2λС��*/
#define CONVERT_BITS        (1 << 12)   /* ת��λ��Ϊ12λ */
#define PRESSURE_FILTER_BUF_NUM         (3)
rt_adc_device_t adc_dev1;
rt_adc_device_t adc_dev2;
rt_adc_device_t adc_dev3;
static rt_thread_t adc_read_task = RT_NULL;
cd74_t cd74 = {0};

/********************************************************************************************
��������MY_ADC_Init
����  ��ADC���ų�ʼ��
����  ����
����ֵ����
*********************************************************************************************/ 	
rt_err_t  cd74xx_adc_init(void)
{    
    rt_err_t ret = RT_EOK;
    cd74.s1_pin = false;
    cd74.s2_pin = false;
    cd74.s3_pin = false;
    rt_pin_mode(A_IN_SEL0_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(A_IN_SEL1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(A_IN_SEL2_PIN, PIN_MODE_OUTPUT);
}			

/********************************************************************************************
��������Get_Adc2
����  �����ADCֵ
����  ��ͨ��ֵ 0~16ȡֵ��ΧΪ��ADC_Channel_0~ADC_Channel_16
����ֵ��ת�����
*********************************************************************************************/ 	
rt_uint16_t Get_Adc1(rt_uint8_t ch)   
{
    rt_err_t ret = RT_EOK;
    rt_uint16_t value;
    uint8_t i;
    ret = rt_adc_enable(adc_dev1, ch);

    value = 0;
    
    for(i = 0; i < PRESSURE_FILTER_BUF_NUM; i++)
    {
        value += rt_adc_read(adc_dev1, ch);
    }
    
    ret = rt_adc_disable(adc_dev1, ch);
  
    value = value / PRESSURE_FILTER_BUF_NUM;

    return value;
}

rt_uint16_t Get_Adc2(rt_uint8_t ch)   
{
    rt_err_t ret = RT_EOK;
    rt_uint16_t value;
    uint8_t i;
    ret = rt_adc_enable(adc_dev2, ch);

    value = 0;
    
    for(i = 0; i < PRESSURE_FILTER_BUF_NUM; i++)
    {
        value += rt_adc_read(adc_dev2, ch);
    }
    
    ret = rt_adc_disable(adc_dev2, ch);
  
    value = value / PRESSURE_FILTER_BUF_NUM;

    return value;
}

rt_uint16_t Get_Adc3(rt_uint8_t ch)   
{
    rt_err_t ret = RT_EOK;
    rt_uint16_t value;
    uint8_t i;
    ret = rt_adc_enable(adc_dev3, ch);

    value = 0;
    
    for(i = 0; i < PRESSURE_FILTER_BUF_NUM; i++)
    {
        value += rt_adc_read(adc_dev3, ch);
    }
    
    ret = rt_adc_disable(adc_dev3, ch);
  
    value = value / PRESSURE_FILTER_BUF_NUM;

    return value;
}
/********************************************************************************************
�������SW_AD_Channel
����  ���л�ADC����ͨ��
����  ��ͨ��ֵ 0~7
����ֵ����
*********************************************************************************************/ 	
void SW_AD_Channel(rt_uint8_t Channel)
{
	switch(Channel)
	{
		case 0:
				rt_pin_write(A_IN_SEL0_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL1_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL2_PIN, PIN_LOW);
				break; 
		case 1:
				rt_pin_write(A_IN_SEL0_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL1_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL2_PIN, PIN_LOW);	
				break; 
		case 2:
				rt_pin_write(A_IN_SEL0_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL1_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL2_PIN, PIN_LOW);	
				break; 
		case 3:
				rt_pin_write(A_IN_SEL0_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL1_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL2_PIN, PIN_LOW);
				break;
		case 4:
				rt_pin_write(A_IN_SEL0_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL1_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL2_PIN, PIN_HIGH);
		        break;
		case 5:
				rt_pin_write(A_IN_SEL0_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL1_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL2_PIN, PIN_HIGH);
				break;
		case 6:
				rt_pin_write(A_IN_SEL0_PIN, PIN_LOW);
				rt_pin_write(A_IN_SEL1_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL2_PIN, PIN_HIGH);
				break;
		case 7:
				rt_pin_write(A_IN_SEL0_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL1_PIN, PIN_HIGH);
				rt_pin_write(A_IN_SEL2_PIN, PIN_HIGH);
				break;	
	}
    
    cd74.s1_pin = rt_pin_read(A_IN_SEL0_PIN);
    cd74.s2_pin = rt_pin_read(A_IN_SEL1_PIN);
    cd74.s3_pin = rt_pin_read(A_IN_SEL2_PIN);
}

/********************************************************************************************
��������TEMP_READ
����  ���������¶�
����  ��arr[3][8]�� ��ά���׵�ַ
		row����
		column����
����ֵ��RealTemp-�¶�ֵ
*********************************************************************************************/
int TEMP_READ(rt_uint16_t Arrval, rt_uint8_t cntmeans)
{
	int RealTemp;
	rt_uint16_t ValueBuf;
	ValueBuf =  Arrval;
	
	RealTemp = Temp_Cnt(ValueBuf,cntmeans);                  //��ADC����ֵת��Ϊ�¶�ֵ
	if(RealTemp < 0)                                        
	{
		RealTemp= RealTemp * (-1)+1000;				        //��1000����ֵ��ʾ���¶�
	}
	return RealTemp;  
} 

/*********************************************************************
*�������
*adcArr[0][8]={PUMP1_I,PUMP2_I,PUMP3_I,PUMP4_I,PUMP5_I,GND,GND,GND,}
*adcArr[1][8]={TEMP_A,TEMP_B,PUMP1_TEMP,PUMP2_TEMP,PUMP3_TEMP,PUMP4_TEMP,PUMP5_TEMP,GND}
*adcArr[2][8]={PD_A,PD_B,PD_C,SEED_MONITOR,SHG_TEMP,THG_TEMP,GND,GND}
*********************************************************************/
#define ADC_CHECK_NUM   8
static void Adc_Read_Task(void* parameter)
{
    static rt_uint16_t adcArr1[CD74_USE_CHANNEL][ADC_CHECK_NUM],adcArr2[CD74_USE_CHANNEL][ADC_CHECK_NUM],adcArr3[CD74_USE_CHANNEL][ADC_CHECK_NUM];           //��¼ADC����ֵ
	rt_uint8_t adc_check_nums=0;
	rt_uint32_t tmp[3];
    rt_uint8_t i,j;
	
	while(1)
	{		
		if(adc_check_nums < ADC_CHECK_NUM)   //��������
		{
			for(i = 0; i < CD74_USE_CHANNEL; i++)
			{ 
				SW_AD_Channel(i);
                rt_thread_mdelay(10);                
				adcArr1[i][adc_check_nums] = Get_Adc1(ADCA_DEV_CHANNEL_10);                  
			    adcArr2[i][adc_check_nums] = Get_Adc2(ADCB_DEV_CHANNEL_11);
			    adcArr3[i][adc_check_nums] = Get_Adc3(ADCC_DEV_CHANNEL_12);
			}
			adc_check_nums++;
		}
		else
		{
            adc_check_nums = 0;
            for(i = 0; i < CD74_USE_CHANNEL; i++)
            {
                tmp[0] = 0;
                tmp[1] = 0;
                tmp[2] = 0;
                for(j = 0; j < ADC_CHECK_NUM; j++)
                {
                    tmp[0] += adcArr1[i][j];
                    tmp[1] += adcArr2[i][j];
                    tmp[2] += adcArr3[i][j];
                }
                cd74.adcArr[0][i] = tmp[0] / ADC_CHECK_NUM;	
                cd74.adcArr[1][i] = tmp[1] / ADC_CHECK_NUM;
                cd74.adcArr[2][i] = tmp[2] / ADC_CHECK_NUM;
            }
			
            //��ȡSHG/THGֵ
            GD_BUF[SHG_REAL] = STHG_TEMP_READ(cd74.adcArr[2][4]);
            GD_BUF[THG_REAL] = STHG_TEMP_READ(cd74.adcArr[2][5]);
                
            //��ȡIR/UV��PD����ֵ
            GD_BUF[IR_A] = cd74.adcArr[2][1];
            GD_BUF[UV_A] = cd74.adcArr[2][2];
                 
            //����д�����ڲ��ɼ����ʵ���һ������ȫ��д��
            if((GD_BUF[IR_A_MAX] == false) || (GD_BUF[IR_A_MIN] == false)|| (GD_BUF[IR_P_MAX] == false)|| (GD_BUF[IR_P_MIN] == false))
            {
                GD_BUF[UV_POWER] = UvLaserPowerRead(GD_BUF[IR_A], GD_BUF[UV_A_MAX], GD_BUF[UV_A_MIN], GD_BUF[UV_P_MAX], GD_BUF[UV_P_MIN], GD_BUF[MODULO_DIVIDER]);
                GD_BUF[IR_POWER] = 0;
            }
			//GD_BUF[UV_POWER] = UvLaserPowerRead(GD_BUF[UV_A], GD_BUF[UV_A_MAX], GD_BUF[UV_A_MIN], GD_BUF[UV_P_MAX], GD_BUF[UV_P_MIN], GD_BUF[MODULO_DIVIDER]);
			if((GD_BUF[UV_A_MAX] == false) || (GD_BUF[UV_A_MIN] == false)|| (GD_BUF[UV_P_MAX] == false)|| (GD_BUF[UV_P_MIN] == false))
            {
                GD_BUF[IR_POWER] = IRLaserPowerRead(GD_BUF[IR_A], GD_BUF[IR_A_MAX], GD_BUF[IR_A_MIN], GD_BUF[IR_P_MAX], GD_BUF[IR_P_MIN]);
                GD_BUF[UV_POWER] = 0;
            }
                   
            //��ȡ����Դ����
            GD_BUF[SEED_LASER_ENERGY_MON] = ACTIVE_SEED_ENERGY_READ(cd74.adcArr[2][3], GD_BUF[SEED_TYPE]);
    //		if (GD_BUF[SEED_LASER_ENERGY_MON] < 10)
    //			GD_BUF[SEED_LASER_ENERGY_MON] = 0;
                
            //��ȡLD��ǻ���¶�
            GD_BUF[CAVITY2_TEMP_READ] = TEMP_READ(cd74.adcArr[1][1], 0);    
            GD_BUF[CAVITY_TEMP_READ]  = TEMP_READ(cd74.adcArr[1][0], 0);	
            GD_BUF[PUMP1_TEMP_READ]   = TEMP_READ(cd74.adcArr[1][2], 0);
            GD_BUF[PUMP2_TEMP_READ]   = TEMP_READ(cd74.adcArr[1][3], 0);
            GD_BUF[PUMP3_TEMP_READ]   = TEMP_READ(cd74.adcArr[1][4], 0);
            //GD_BUF[PUMP4_TEMP_READ]   = TEMP_READ(cd74.adcArr[1][5], 0);

                
            //��ȡLD����
            GD_BUF[PUMP1_I_READ] = PUMP_I_READ(cd74.adcArr[0][0]);
            GD_BUF[PUMP2_I_READ] = PUMP_I_READ(cd74.adcArr[0][1]);
            GD_BUF[PUMP3_I_READ] = PUMP_I_READ(cd74.adcArr[0][2]); 
    	    //GD_BUF[PUMP4_I_READ] = PUMP_I_READ(cd74.adcArr[0][3]);	
		}
		rt_thread_mdelay(500);
	}
}

int cd74_thread_init(void)
{
    /* �����豸 */
    adc_dev1 = (rt_adc_device_t)rt_device_find(ADC1_DEV_NAME);
    if (adc_dev1 == RT_NULL)
    {
        rt_kprintf("adc run failed! can't find %s device!\n", ADC1_DEV_NAME);
    }
    adc_dev2 = (rt_adc_device_t)rt_device_find(ADC2_DEV_NAME);
    if (adc_dev2 == RT_NULL)
    {
        rt_kprintf("adc run failed! can't find %s device!\n", ADC2_DEV_NAME);
    }
    adc_dev3 = (rt_adc_device_t)rt_device_find(ADC3_DEV_NAME);
    if (adc_dev3 == RT_NULL)
    {
        rt_kprintf("adc run failed! can't find %s device!\n", ADC3_DEV_NAME);
    }
    adc_read_task = rt_thread_create("cd74",
                            Adc_Read_Task, RT_NULL,
                            2048,
                            15,20);
    if (adc_read_task != RT_NULL)
        rt_thread_startup(adc_read_task);
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void cd74_read(uint8_t argc, char **argv)
{
    rt_uint32_t vol;
    const char *adc_name[CD74_NUM][CD74_USE_CHANNEL]={ \
            {"PUMP1_I","PUMP2_I","PUMP3_I","PUMP4_I","PUMP5_I","GND","GND","GND"},
            {"TEMP_A","TEMP_B","PUMP1_TEMP","PUMP2_TEMP","PUM3_TEMP","PUM4_TEMP","GND","GND"},
            {"PD_A","PD_B","PD_C","SHG_TEMP","THG_TEMP","GND","GND","GND"}};

    rt_kprintf("cd74.s1_pin               = %s      \n", (cd74.s1_pin == PIN_HIGH)?"PIN_HIGH":"PIN_LOW");
    rt_kprintf("cd74.s2_pin               = %s      \n", (cd74.s2_pin == PIN_HIGH)?"PIN_HIGH":"PIN_LOW");
    rt_kprintf("cd74.s3_pin               = %s      \n", (cd74.s3_pin == PIN_HIGH)?"PIN_HIGH":"PIN_LOW");
    for(int i = 0; i < CD74_NUM; i++)
    {
        for(int j = 0; j < CD74_USE_CHANNEL; j++)
        {
            vol = cd74.adcArr[i][j] * REFER_VOLTAGE / CONVERT_BITS;
            rt_kprintf("cd74.adcArr[%s \t\t]         :%d.%02d v      \n", adc_name[i][j], vol / 100, vol % 100); 
        }
    }  
}
MSH_CMD_EXPORT(cd74_read, cd74 read);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */