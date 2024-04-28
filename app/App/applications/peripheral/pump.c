/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "pump"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "pump.h"

static rt_thread_t tid_pump = RT_NULL;
static struct rt_mailbox pump_mb;
static char mb_pool[128];
pump pump_t = {0};

typedef struct 
{
    rt_uint8_t PUMP_OFF_BIT[4];   //�洢PUMP���ؼ����־λ
	rt_uint16_t PUMP_I[4];        //�洢PUMP����ADCֵ
		
}PumpStruct; 


static PumpStruct Pump={
	{1,1,1,1},
	{0,0,0,0}
};

/********************************************************************************************
��������PUMP_Init
����  ����ʼ������ʹ������
����  ����
����ֵ����
*********************************************************************************************/
void PUMP_Init(void)
{
    rt_pin_mode(PUMP1_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(PUMP2_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(PUMP3_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(PUMP4_ENA_PIN, PIN_MODE_OUTPUT);
    PUMP1_ENA_OFF();
    PUMP2_ENA_OFF();
    PUMP3_ENA_OFF();
    PUMP4_ENA_OFF();
}

void pump_temp_sync(void)
{
    pump_temp pump_temp_t;
    if(rt_mb_recv(&pump_mb, (rt_ubase_t *)&pump_temp_t.pump_mb, RT_WAITING_NO) == RT_EOK)
    {
        pump_t.pump_tem[pump_temp_t.temp.pump_id].temp.pump_id = pump_temp_t.temp.pump_id;
        pump_t.pump_tem[pump_temp_t.temp.pump_id].temp.pump_t = pump_temp_t.temp.pump_t;
    }
}

void pump_temp_error_insert(rt_uint16_t id, rt_uint8_t temp)
{
    if(IsInSide(temp, GD_BUF[PUMP_TEMP_H_ALARM], 255))
    {
        if(id == PUMP1_ID)
        {
            error_insert(ERROR_ID_PUMP1_HTEMP); 
        }
        else if(id == PUMP2_ID)
        {
            error_insert(ERROR_ID_PUMP2_HTEMP); 
        }  
    }
    else
    {
        if(id == PUMP1_ID)
        {
            error_insert(ERROR_ID_PUMP1_LTEMP); 
        }
        else if(id == PUMP2_ID)
        {
            error_insert(ERROR_ID_PUMP2_LTEMP); 
        }  
    }
}

void pump_temp_error_delete(rt_uint16_t id)
{
    if(id ==PUMP1_ID)
    {
        error_delete(ERROR_ID_PUMP1_HTEMP);
        error_delete(ERROR_ID_PUMP1_LTEMP);     
    }
    else if(id ==PUMP2_ID)
    {
        error_delete(ERROR_ID_PUMP2_HTEMP);   
        error_delete(ERROR_ID_PUMP2_LTEMP);
    }
}

void pump_temp_monitor(void)
{
    for(int i = 0; i < PUMP_NUM; i++)
    {
        if(IsOutSide(pump_t.pump_tem[i].temp.pump_t, GD_BUF[PUMP_TEMP_L_ALARM], GD_BUF[PUMP_TEMP_H_ALARM]))
        {
            pump_temp_error_insert(i, pump_t.pump_tem[i].temp.pump_t);
        }
        else 
        {
            pump_temp_error_delete(i);
        }
    }
}

static void thread_pump_entry(void *parameter)
{
    while (1)
    {
        pump_temp_sync();
        pump_temp_monitor();
        rt_thread_mdelay(100);
    }
}

int thread_pump_init(void)
{
    int result;
    
    result = rt_mb_init(&pump_mb,
                        "mb_pump",                 
                        &mb_pool[0],               
                        sizeof(pump_temp),       
                        RT_IPC_FLAG_FIFO);     
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
    
    tid_pump = rt_thread_create("pump",
                                   thread_pump_entry, RT_NULL,
                                   1024,
                                   15, 
                                   10);
    
    result = rt_thread_startup(tid_pump);
    if (result != RT_EOK)
    {
        rt_kprintf("init tid_pump failed.\n");
        return -1;
    }
    
    return result;
}

/******************************************************API***************************************************/


/********************************************************************************************
��������ACTIVE_PUMP_I_SET
����  ��ִ�б��ֵ�����/��
����  ��pumpSwFlag��1-ON,0-OFF
        Pump_I_Set�����õĵ���ֵ
		Channel��PUMPX��Ӧ��DACͨ��
����ֵ����
*********************************************************************************************/
void ACTIVE_PUMP_I_SET(rt_uint8_t pumpSwFlag, rt_uint16_t Pump_I_Set,rt_uint8_t Channel) 
{
	if(pumpSwFlag == 1)                  
	{
		PUMP_I_SET_ON(Pump_I_Set,Channel);    //��
	}
	else
	{
		PUMP_I_SET_OFF(Pump_I_Set,Channel);   //��
	}
}

/********************************************************************************************
��������PUMP_I_SET_ON
����  �����ñ����ϵ����ֵDAC
����  ��Pump_I_Set�� PUMPX���õ���ֵ
		Channel: PUMPX��Ӧ��DACͨ��
����ֵ����
*********************************************************************************************/
void PUMP_I_SET_ON(rt_uint16_t Pump_I_Set,rt_uint8_t Channel)
{
	rt_uint8_t i;
	rt_uint16_t n,PUMP_I_TEMP;
       
    //����LD����
    //----------LD����
    //�ﵽ�������������������½�����;���Ըĵ���	
	i = Channel;
	
	if(Pump.PUMP_OFF_BIT[i] == 1)                                      
	{
		Pump.PUMP_OFF_BIT[i] = 0;									       //����һ�ι�󼤻�ر��ּ����־λ
		
		switch(Channel)
		{
			case 0: PUMP1_ENA_ON(); 
				    LASER_LED_ON();					                    //1��LD�������̵����𣬱�ʾ�м���
					rt_thread_mdelay(500);   //�ȴ�0.5S����Ϊ����
			        break;      
			
			case 1: PUMP2_ENA_ON();
					rt_thread_mdelay(500); 
					break;
			
			case 2: PUMP3_ENA_ON();
					rt_thread_mdelay(500); 
					break;
			
			case 3: PUMP4_ENA_ON();
					rt_thread_mdelay(500); 
					break;
			default:break;
		}
	}      
	
	PUMP_I_TEMP  = Pump_I_Set * 14.8;                                      //���趨����ֵXX.XA����ΪADCֵ ,��14.3����14.8  
	
	if(PUMP_I_TEMP > Pump.PUMP_I[i])
	{
		for(n = 1; n <= PUMP_I_TEMP - Pump.PUMP_I[i]; n+=2)                 //������������
		{	
			//Dac7568_Channel_Vol(Channel,Pump.PUMP_I[i]+n);
			DAC_CBM128_Channel_Vol(Channel,Pump.PUMP_I[i]+n);					
			rt_thread_mdelay(10);                     
		}
	}
	else if(PUMP_I_TEMP < Pump.PUMP_I[i])
	{
		for(n = 1; n <= Pump.PUMP_I[i] - PUMP_I_TEMP; n+=2)                //���������½�
		{		
			//Dac7568_Channel_Vol(Channel,Pump.PUMP_I[i]-n);	
			DAC_CBM128_Channel_Vol(Channel,Pump.PUMP_I[i]-n);										
			rt_thread_mdelay(10);                			
		}	
	}	
	
	Pump.PUMP_I[i]= PUMP_I_TEMP;  									       //��¼��һ�εĵ���ֵ
}

/********************************************************************************************
��������PUMP_I_SET_OFF
����  ���ر��ֵ���
����  ��Pump_I_Set�� PUMPX���õ���ֵ
		Channel: PUMPX��Ӧ��DACͨ��
����ֵ����
*********************************************************************************************/
void PUMP_I_SET_OFF(rt_uint16_t Pump_I_Set,rt_uint8_t Channel)
{
	rt_uint8_t i;
	rt_uint16_t n,PUMP_I_TEMP;
	
	i = Channel;
	if(!Pump.PUMP_OFF_BIT[i])                                         //�ؼ����־λ��һ�ιر��ּ����Ӧһ�ο����ּ���
	{			
		Pump.PUMP_OFF_BIT[i] = 1;	
		
		PUMP_I_TEMP  = Pump_I_Set * 14.8; 
		
		for(n = 1; n <= PUMP_I_TEMP; n+=4)                             //���������½����ض��ٶȱȴ򿪿�1��
		{					
			//Dac7568_Channel_Vol(Channel,PUMP_I_TEMP-n);	
			DAC_CBM128_Channel_Vol(Channel,PUMP_I_TEMP-n);	

			rt_thread_mdelay(10);                     
		}
		
		//Dac7568_Channel_Vol(Channel,0);
		DAC_CBM128_Channel_Vol(Channel,0);

		switch(Channel)
		{
			case 0: 
                rt_thread_mdelay(500); 
				PUMP1_ENA_OFF();
				LASER_LED_OFF(); 				                 //1��LD�رգ��̵��𣬱�ʾû����
				break;          
			
			case 1: 
                rt_thread_mdelay(500); 
				PUMP2_ENA_OFF();
				break;
			
			case 2: 
                rt_thread_mdelay(500); 
				PUMP3_ENA_OFF(); 
				break;
			
			case 3: 
                rt_thread_mdelay(500); 
				PUMP4_ENA_OFF();
				break;
			default:break;
		}
		Pump.PUMP_I[i] = 0;		 		
	}	 	 		
}

/********************************************************************************************
��������PUMP_I_READ
����  �������ֵ���
����  ��arr[3][8]�� ��ά���׵�ַ
		row����
		column����
����ֵ��TempBuf ����ֵ
*********************************************************************************************/
rt_uint16_t PUMP_I_READ(rt_uint16_t Arrval)
{
	rt_uint16_t TempBuf,ValueBuf; 
	
	ValueBuf = Arrval;
	
	TempBuf = 0.048 * ValueBuf;     //ADC����ֵ*0.048(ϵ��)��ת��ΪXX.X Aֵ
	if(TempBuf < 5)
	{
		TempBuf = 0;
	}
	return TempBuf;
}

/********************************************************************************************
��������pump_temp_get
����  �����ñ����¶�
����  ��id�� �õı��
		temp���õ��¶�
����ֵ��void
*********************************************************************************************/
void pump_temp_get(rt_uint8_t id, rt_uint16_t temp)
{
	pump_temp pump_temp_t;
    pump_temp_t.temp.pump_id = id;
    pump_temp_t.temp.pump_t = temp;
    
    rt_mb_send(&pump_mb, (rt_uint32_t)pump_temp_t.pump_mb);
} 

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void pump_set(uint8_t argc, char **argv)
{
    rt_uint32_t param;
    
    if(argc != 4)
    {
        rt_kprintf("cmd error . please eg : pump_set [pump1/pump2/pump3/pump4] [vcc/i] [param]");   
    }
    else{
        param = atoi(argv[3]);
        
        if(strcmp(argv[1], "pump1") == 0)
        {
            if(strcmp(argv[2], "vcc") == 0)
            {
               	ACTIVE_PUMP_I_SET(param, 10, 0);
            }
            else if(strcmp(argv[2], "i") == 0)
            {
                if(param > 30)
                {
                    rt_kprintf("pump i too big set to 10");
                    param = 10;
                }
                ACTIVE_PUMP_I_SET(true, param, 0);
            }
        }
        else if(strcmp(argv[1], "pump2") == 0)
        {
            if(strcmp(argv[2], "vcc") == 0)
            {
               	ACTIVE_PUMP_I_SET(param, 10, 1);
            }
            else if(strcmp(argv[2], "i") == 0)
            {
                if(param > 30)
                {
                    rt_kprintf("pump i too big set to 10");
                    param = 10;
                }
                ACTIVE_PUMP_I_SET(true, param, 1);
            }
        } 
        else{
            rt_kprintf("cmd error . please eg : pump_set [pump1/pump2/pump3/pump4] [vcc/i] [param]");  
        }
    }
}
MSH_CMD_EXPORT(pump_set, pump set);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
