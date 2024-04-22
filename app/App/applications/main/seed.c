/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 *
 *  SeedPower_12V___1   5__GND
 *  GND_V___________2   6__Seed_ENA
 *  AGND____________3   7__SeedErrorFlag
 *  SeedPowerCtrl___4   8__SeedPowerMonitor
 *
 *  PE7(H)------------->SeedPower_12V
 *  CBM128S085(DAC)---->SeedPowerCtrl
 *  PE8(H)------------->Seed_ENA
 *  SeedErrorFlag------>PE9
 *  SeedPowerMonitor--->CD74HB-CH6---->PC2(ADC1-12)
 */

#define LOG_TAG   "seed"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "seed.h"

seed_t seed = {0};
static rt_thread_t tid_seed = RT_NULL;
/********************************************************************************************
函数名：Seed_Init
功能  ：初始化种子源引脚
参数  ：无
返回值：无
*********************************************************************************************/
void Seed_Init(void)
{
    log_d("seed PE7/8/9 pin init");
    seed.pumpRealIoff = PUMPREALIOFF;
    seed.seedLaserFlag = false;
    seed.seedPowerFlag = false;
    seed.Seed_PowerUp_Delay_cnt = false;
    seed.readSeedEnergy = false;
    seed.writeSeedEnergy = false;
    rt_pin_mode(SEED_POWER_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(SEED_LASER_ENA_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(SEED_ALARM_PIN, PIN_MODE_INPUT_PULLUP);
}

void seed_monitor(void)
{
    static rt_bool_t seed_error = false;
    
   	if(GD_BUF[SEED_TYPE] == TOPTICA)
	{
		if((SEED_ALARM == 0) )  //低电平持续时间> 400ms 有效
		{	
            log_e("seed PE9 error pin is low");
            rt_thread_mdelay(400);
            if(SEED_ALARM == 0) 
            {
                log_e("delay 1.7s, seed PE9 error pin is also low");
                seed_error = true;
                error_insert(ERROR_ID_SEED);
            }
		}
        else
        {
            if(seed_error == true)
            {
                seed_error = false;
                error_delete(ERROR_ID_SEED);
            }
        }
	}
	else if(GD_BUF[SEED_TYPE] == NPI)
	{
		if(ReadSeedLaserPinState)
		{
			if((SEED_ALARM == 0))  //低电平持续时间> 17s 有效
			{	
                log_e("seed PE9 error pin is low");
				rt_thread_mdelay(15000);	
                if(SEED_ALARM == 0) 
                {
                    log_e("delay 1.5s, seed PE9 error pin is also low");
                    seed_error = true;
                    error_insert(ERROR_ID_SEED);
                }
                else  
                {
                    log_e("delay 1.5s, seed PE9 error pin change high");
                }
			}
            else
            {
                if(seed_error == true)
                {
                    seed_error = false;
                    error_delete(ERROR_ID_SEED);
                }
            }
        }
        else if(seed_error == true)
        {
             rt_thread_mdelay(15000);	
             seed_error = false;
             error_delete(ERROR_ID_SEED);
        }
    }
}

static void thread_seed_entry(void *parameter)
{
    while (1)
    {
        seed_monitor();
        rt_thread_mdelay(100);
    }
}

int thread_seed_init(void)
{
    int result;
    
    tid_seed = rt_thread_create("pump",
                                   thread_seed_entry, RT_NULL,
                                   1024,
                                   15, 
                                   10);
    
    result = rt_thread_startup(tid_seed);
    if (result != RT_EOK)
    {
        rt_kprintf("init tid_seed failed.\n");
        return -1;
    }
    
    return result;
}
/******************************************************API***************************************************/

/********************************************************************************************
函数名：SeedCtrlAct
功能  ：种子源电源和出光控制
参数  ：
		seedPowerState，种子源上电使能
		seedLaserState，种子源出光使能
		seedLaserEnergy，种子源出光功率
		seedType，种子源类型
		Pump1RealI，泵1实际电流值
		Pump2RealI，泵2实际电流值
		Pump3RealI，泵3实际电流值
		Pump4RealI，泵4实际电流值
返回值：无
*********************************************************************************************/
void SeedCtrlAct(rt_uint8_t seedPowerState, rt_uint8_t seedLaserState, rt_uint8_t seedLaserEnergy,rt_uint8_t seedType, rt_uint16_t Pump1RealI,rt_uint16_t Pump2RealI, rt_uint16_t Pump3RealI, rt_uint16_t Pump4RealI)  
{
    seed.seedType = seedType;
    
	if(seedPowerState == true)		   				   
	{
		seed.seedPowerFlag = true;
		seed.Seed_PowerUp_Delay_cnt++;
		if(seed.Seed_PowerUp_Delay_cnt >= 40) //延迟8S，等待种子源上电复位后再使能出激光 
		{
			seed.Seed_PowerUp_Delay_cnt = 40;
		}
	} 
	
	if(seedType == TOPTICA) //Toptica
	{	
		if((seedPowerState == false) && (seedLaserEnergy < 10) ) //种子源激光关断后再关闭电源
		{
			seed.Seed_PowerUp_Delay_cnt = false;
			seed.seedPowerFlag = false;
		}
		
		if((seedLaserState ==true) && (seed.Seed_PowerUp_Delay_cnt >= 16)) //种子源上电延迟8S后，允许开激光操作
		{		
			seed.seedLaserFlag = true;	    
		}
		else if((seedLaserState == false) && (Pump1RealI < seed.pumpRealIoff) && (Pump2RealI < seed.pumpRealIoff) && ( Pump3RealI < seed.pumpRealIoff) && ( Pump4RealI < seed.pumpRealIoff) )  //在泵全部关断的情况下才允许关种子源激光
		{
			seed.seedLaserFlag = false;
		}
	}
	else if(seedType == NPI) //NPI
	{
	    if((seedPowerState == false) && (ReadSeedLaserPinState == true)) 
		{
			seed.Seed_PowerUp_Delay_cnt = false;
			seed.seedPowerFlag = false;
		}
		if((seedLaserState == true) && (seed.Seed_PowerUp_Delay_cnt >= 32)) //种子源上电延迟16S后，允许开激光操作
		{		
			seed.seedLaserFlag = true;	    
		}
		else if( (seedLaserState == false) && (Pump1RealI < seed.pumpRealIoff) && (Pump2RealI < seed.pumpRealIoff) && ( Pump3RealI < seed.pumpRealIoff) && ( Pump4RealI < seed.pumpRealIoff) )  //在泵全部关断的情况下才允许关种子源激光
		{
			seed.seedLaserFlag = false;
		}
	}
	
	SeedSwAct(seed.seedPowerFlag, seed.seedLaserFlag); 	
}

/********************************************************************************************
函数名：SeedSwAct
功能  ：执行种子源开/关电，执行种子源开/关光
参数  ：seedPowerFlag，种子源开/关电标志位，
        seedLaserFlag，种子源开/关激光标志位
返回值：无
*********************************************************************************************/
void SeedSwAct(rt_uint8_t seedPowerFlag, rt_uint8_t seedLaserFlag)
{
    static rt_uint8_t seedPowerFlag_re = 0xff;
    static rt_uint8_t seedLaserFlag_re = 0xff;
    
    if(seedPowerFlag_re != seedPowerFlag)
    {
        seedPowerFlag_re = seedPowerFlag;
        if(seedPowerFlag == true)		   				   
        {
            log_i("seed vcc on");
            SEED_POWER_ON();
        } 
        else
        {
            log_i("seed vcc off");
            SEED_POWER_OFF();
        }
    }
	
    if(seedLaserFlag_re != seedLaserFlag)
    {
        seedLaserFlag_re = seedLaserFlag;
        if(seedLaserFlag == true) 
        {
            log_i("seed laser on");
            SEED_LASER_ON();
        }
        else
        {
            log_i("seed laser off");
            SEED_LASER_OFF();
        }
    }
}

/********************************************************************************************
函数名：ACTIVE_SEED_ENERGY_READ
功能  ：读取种子源功率，toptica种子源反馈功率0~4.5V对应功率0~100%; NPI种子源0~1.5V对应功率0~100%
参数  ：arrVal，种子源功率采样的ADC值
返回值：reSeedEnergy,取值0~100对应种子源功率0~100%
*********************************************************************************************/
rt_uint8_t ACTIVE_SEED_ENERGY_READ(rt_uint16_t arrVal, rt_uint8_t seedType) 
{
	rt_uint8_t reSeedEnergy;
	rt_uint16_t valBuf;
	
	valBuf =arrVal;
	
	if(seedType == TOPTICA)
	{
		reSeedEnergy = valBuf * 110 / 4096;    // 0~100 表示 0%~100%
	}
	else if(seedType == NPI)
	{
		reSeedEnergy = valBuf * 220 / 4096;    // 0~1.5V 对应 0%~100%
	}
	else
	{
		reSeedEnergy = 5;
	}

	if(reSeedEnergy < 5)
		reSeedEnergy = 0;                    //清除偏置电压
	
    seed.readSeedEnergy = reSeedEnergy;
	return reSeedEnergy;
}

/********************************************************************************************
函数名：ACTIVE_SEED_ENERGY_CTRL
功能  ：设置种子源功率，0~2.5V对应功率90%~100%
参数  ：Value： 0-100%
		Channel：DAC7568通道值
返回值：无 
*********************************************************************************************/
void ACTIVE_SEED_ENERGY_CTRL(rt_uint16_t Value,rt_uint8_t Channel)
{
	rt_uint16_t TempBuf;
	if(Value >= 90)
	{
		TempBuf = (Value-90) * 250;                         // 0~2500mv =  90% ~ 100%;
		//Dac7568_Channel_Vol(Channel,TempBuf); 
		DAC_CBM128_Channel_Vol(Channel,TempBuf);   //100-54mv  90耶一样
	}
    log_i("seed power power set %d", TempBuf);
    seed.writeSeedEnergy = TempBuf;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void seed_read(uint8_t argc, char **argv)
{
    rt_kprintf("seed.seedType               = %s      \n", (seed.seedType == 1)?"TOPTICA":"NPI");
    rt_kprintf("seed.pumpRealIoff           = %d      \n", seed.pumpRealIoff);
    rt_kprintf("seed.readSeedEnergy         = %d      \n", seed.readSeedEnergy);
    rt_kprintf("seed.Seed_PowerUp_Delay_cnt = %d      \n", seed.Seed_PowerUp_Delay_cnt);
	rt_kprintf("seed.seedLaserFlag          = %d      \n", seed.seedLaserFlag);
    rt_kprintf("seed.seedPowerFlag          = %d      \n", seed.seedPowerFlag);
    rt_kprintf("seed.writeSeedEnergy        = %d      \n", seed.writeSeedEnergy);
}
MSH_CMD_EXPORT(seed_read, seed read);

static void seed_set(uint8_t argc, char **argv)
{
    rt_uint32_t param;
    
    if(argc != 3)
    {
        rt_kprintf("cmd error . please eg : seed_set [vcc/en/power] [param]");   
    }
    else{
        param = atoi(argv[2]);
        
        if(strcmp(argv[1], "vcc") == 0)
        {
            if(param == true)		   				   
            {
                SeedSwAct(true, false);
            } 
            else
            {
                SeedSwAct(false, false);
            }
        }
        else if(strcmp(argv[1], "en") == 0)
        {
            if(param == true)
            {
                SeedSwAct(true, false);
                rt_thread_mdelay(5000);
                SeedSwAct(true, param);                
            }
            else{
                SeedSwAct(false, false);
            }
        }
        else if(strcmp(argv[1], "power") == 0)
        {
            ACTIVE_SEED_ENERGY_CTRL(param,Channel_E);  //设置种子源功率
        }
        else{
            rt_kprintf("cmd error . please eg : seed_set [vcc/en/power] [param]");  
        }
    }
}
MSH_CMD_EXPORT(seed_set, seed set);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
