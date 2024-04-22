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
 
#define ADC_DEV_NAME        "adc1"      /* ADC 设备名称 */
#define REFER_VOLTAGE       3300         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 12)   /* 转换位数为12位 */
#define PRESSURE_FILTER_BUF_NUM         (3)
rt_adc_device_t adc_dev;
static rt_thread_t adc_read_task = RT_NULL;
cd74_t cd74 = {0};

/********************************************************************************************
函数名：MY_ADC_Init
功能  ：ADC引脚初始化
参数  ：无
返回值：无
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
函数名：Get_Adc2
功能  ：获得ADC值
参数  ：通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
返回值：转换结果
*********************************************************************************************/ 	
rt_uint16_t Get_Adc2(rt_uint8_t ch)   
{
    rt_err_t ret = RT_EOK;
    rt_uint16_t value;
    uint8_t i;
    ret = rt_adc_enable(adc_dev, ch);

    value = 0;
    
    for(i = 0; i < PRESSURE_FILTER_BUF_NUM; i++)
    {
        value += rt_adc_read(adc_dev, ch);
    }
    
    ret = rt_adc_disable(adc_dev, ch);
  
    value = value / PRESSURE_FILTER_BUF_NUM;

    return value;
}
/********************************************************************************************
函数名W_AD_Channel
功能  ：切换ADC采样通道
参数  ：通道值 0~7
返回值：无
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


/*********************************************************************
*监测任务
*adcArr[0][8]={PUMP1_I,PUMP2_I,PUMP3_I,PUMP4_I,PUMP1_TEMP,PUMP2_TEMP,PUMP3_TEMP,PUMP4_TEMP,}
*adcArr[1][8]={PUMP1_VOLT,PUMP2_VOLT,PUM3_VOLT,PUM4_VOLT,TEMP_A,TEMP_B,SeedPowerMon,GND}
*adcArr[2][8]={PD_A,PD_B,PD_C,PD_D,SHG_TEMP,THG_TEMP,GND,GND}
*********************************************************************/
#define ADC_CHECK_NUM   4
static void Adc_Read_Task(void* parameter)
{
    static rt_uint16_t adcArr1[CD74_USE_CHANNEL][ADC_CHECK_NUM],adcArr2[CD74_USE_CHANNEL][ADC_CHECK_NUM],adcArr3[CD74_USE_CHANNEL][ADC_CHECK_NUM];           //记录ADC采样值
	rt_uint8_t adc_check_nums=0;
	rt_uint32_t tmp[3];
    rt_uint8_t i,j;
	
	while(1)
	{		
		if(adc_check_nums < ADC_CHECK_NUM)   //采样次数
		{
			for(i = 0; i < CD74_USE_CHANNEL; i++)
			{ 
				SW_AD_Channel(i);
                rt_thread_mdelay(10);                
				adcArr1[i][adc_check_nums] = Get_Adc2(ADCA_DEV_CHANNEL_10);                  
			    //adcArr1[adc_check_nums][i] = Get_Adc2(ADCB_DEV_CHANNEL_12);
			    //adcArr1[adc_check_nums][i] = Get_Adc2(ADCC_DEV_CHANNEL_13);
			}
			adc_check_nums++;
		}
		else
		{
            adc_check_nums = 0;
            for(i = 0; i < CD74_USE_CHANNEL; i++)
            {
                for(j = 0; j < ADC_CHECK_NUM; j++)
                {
                    tmp[0] += adcArr1[i][j];
                    tmp[1] += adcArr2[i][j];
                    tmp[2] += adcArr3[i][j];
                }
                cd74.adcArr[0][i] = tmp[0] / ADC_CHECK_NUM;	
                cd74.adcArr[1][i] = tmp[1] / ADC_CHECK_NUM;
                cd74.adcArr[2][i] = tmp[2] / ADC_CHECK_NUM;
                tmp[0] = 0;
                tmp[1] = 0;
                tmp[2] = 0;
            }
			
            //获取SHG/THG值
            //GD_BUF[SHG_REAL] = STHG_TEMP_READ(cd74.adcArr[2][4]);
            //GD_BUF[THG_REAL] = STHG_TEMP_READ(cd74.adcArr[2][5]);
                
    //			//获取准直器的红外功率值
    //			GD_BUF[COLLIMATOR_IR] = adcArr[2][0];
    //			
    //			//检查准直器处的种子光
    //			if(GD_BUF[COLLIMATOR_IR] > collimatorIrThreshold)
    //			{
    //				GD_BUF[COLLIMATOR_IR_STATE] = 1;
    //			}
    //			else
    //			{
    //				GD_BUF[COLLIMATOR_IR_STATE] = 0;
    //			}
                
            //获取IR/UV的PD采样值
            //GD_BUF[IR_A] = cd74.adcArr[2][1];
            //GD_BUF[UV_A] = cd74.adcArr[2][2];
                 
             //GD_BUF[IR_A] = CS1180_ReadAdValue();//读AD值cs1180s
            //获取紫外输出功率
            //GD_BUF[UV_POWER] = UvLaserPowerRead(GD_BUF[UV_A], GD_BUF[UV_A_MAX], GD_BUF[UV_A_MIN], GD_BUF[UV_P_MAX], GD_BUF[UV_P_MIN], GD_BUF[MODULO_DIVIDER]);
                
    //		GD_BUF[UV_POWER] = GD_BUF[COLLIMATOR_IR];
                
            //获取红外i输出功率
            //GD_BUF[IR_POWER] = IRLaserPowerRead(GD_BUF[IR_A], GD_BUF[IR_A_MAX], GD_BUF[IR_A_MIN], GD_BUF[IR_P_MAX], GD_BUF[IR_P_MIN]);
                   
            //获取种子源功率
            GD_BUF[SEED_LASER_ENERGY_MON] = ACTIVE_SEED_ENERGY_READ(cd74.adcArr[1][6], GD_BUF[SEED_TYPE]);
    //		if (GD_BUF[SEED_LASER_ENERGY_MON] < 10)
    //			GD_BUF[SEED_LASER_ENERGY_MON] = 0;
                
            //获取LD和腔体温度
            //GD_BUF[CAVITY2_TEMP_READ] = TEMP_READ(cd74.adcArr[1][5], 0); 
            //GD_BUF[CAVITY_TEMP_READ]  = TEMP_READ(cd74.adcArr[1][4], 0);
            //GD_BUF[PUMP1_TEMP_READ]   = TEMP_READ(cd74.adcArr[0][4], 0);
            //GD_BUF[PUMP2_TEMP_READ]   = TEMP_READ(cd74.adcArr[0][5], 0);
    //      GD_BUF[PUMP3_TEMP_READ]   = TEMP_READ(adcArr[0][6], 0);
                
            //获取LD电流
            GD_BUF[PUMP1_I_READ] = PUMP_I_READ(cd74.adcArr[0][0]);
            GD_BUF[PUMP2_I_READ] = PUMP_I_READ(cd74.adcArr[0][1]);
    //      GD_BUF[PUMP3_I_READ] = PUMP_I_READ(adcArr[0][2]); 
    //	    GD_BUF[PUMP4_I_READ] = PUMP_I_READ(adcArr[0][3]);	
		}
		rt_thread_mdelay(500);
	}
}

int cd74_thread_init(void)
{
    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
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
            {"PUMP1_I","PUMP2_I","PUMP3_I","PUMP4_I","PUMP1_TEMP","PUMP2_TEMP","PUMP3_TEMP","PUMP4_TEMP"},
            {"PUMP1_VOLT","PUMP2_VOLT","PUM3_VOLT","PUM4_VOLT","TEMP_A","TEMP_B","SeedPowMon","GND"},
            {"PD_A","PD_B","PD_C","PD_D","SHG_TEMP","THG_TEMP","GND","GND"}};

    rt_kprintf("cd74.s1_pin               = %s      \n", (cd74.s1_pin == PIN_HIGH)?"PIN_HIGH":"PIN_LOW");
    rt_kprintf("cd74.s2_pin               = %s      \n", (cd74.s2_pin == PIN_HIGH)?"PIN_HIGH":"PIN_LOW");
    rt_kprintf("cd74.s3_pin               = %s      \n", (cd74.s3_pin == PIN_HIGH)?"PIN_HIGH":"PIN_LOW");
    for(int i = 0; i < CD74_NUM; i++)
    {
        for(int j = 0; j < CD74_USE_CHANNEL; j++)
        {
            vol = cd74.adcArr[i][j] * REFER_VOLTAGE / CONVERT_BITS;
            rt_kprintf("cd74.adcArr[%s \t\t]         :%d.%02d mv      \n", adc_name[i][j], vol / 100, vol % 100); 
        }
    }  
}
MSH_CMD_EXPORT(cd74_read, cd74 read);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */