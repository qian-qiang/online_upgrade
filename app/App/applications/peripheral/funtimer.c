
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "timer"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "funtimer.h"
#include "ShgThg.h"
#include <rtthread.h>
#include <stdbool.h>

/* 定时器的控制块 */
rt_timer_t timer_led;
rt_timer_t timer_beep;
rt_timer_t timer_cool_sync;
static rt_thread_t tid1 = RT_NULL;
static int cnt = 0;
static rt_bool_t shg_sync = false;
static rt_bool_t thg_sync = false;

rt_uint8_t USER_TIME_FLAG[3]={0,0,0}; 
rt_uint8_t LASER_ON_FLAG=0;

/* 定时器 1 超时函数 */
static void timeout1(void *parameter)
{
    static rt_uint16_t timer3_cnt[5]={0,0,0,0,0};
    timer3_cnt[0]++;
			
    if(GD_BUF[STARTING])                     
    {
        if(timer3_cnt[0] % 10 == 0)             //初始化完成以1HZ频率闪烁
        {
            rt_pin_write(LED_RUN_PIN,!rt_pin_read(LED_RUN_PIN));            
        }	
    }
    else
    {
        if(timer3_cnt[0] % 2== 0)               //初始化时以5HZ频率闪烁
        {
            rt_pin_write(LED_RUN_PIN,!rt_pin_read(LED_RUN_PIN));   	
        }	
    }
    
     //记录种子源出光时长
    if(GD_BUF[SEED_LASER_ENERGY_MON] > 90) //种子源功率 > 90%
    {
        timer3_cnt[1]++;
        if(timer3_cnt[1] > 3600)                 //0.1H更新一次
        {
            timer3_cnt[1] = 0;
            
            GD_BUF[SEED_TIME]++;     
            
            USER_TIME_FLAG[0]=1;
        }
    }
    //记录出红外激光时长
    if(GD_BUF[IR_POWER] > 50)              //红外功率 > 10mW
    {
        timer3_cnt[2]++;
        if(timer3_cnt[2] > 3600)		         //0.1H更新一次
        {
            timer3_cnt[2] = 0;
            
            GD_BUF[IR_TIME]++;    
            
            USER_TIME_FLAG[1]=1;
        }
    }	
    //记录出紫外激光时长
    if(GD_BUF[UV_POWER] > 50)              //紫外功率 > 10mW
    {
        timer3_cnt[3]++;
        if(timer3_cnt[3] > 3600)                 //0.1H更新一次
        {
            timer3_cnt[3] = 0;
            
            GD_BUF[UV_TIME]++;     
            
            USER_TIME_FLAG[2]=1;
        }
    }

    if(timer3_cnt[0] >= 60000)
    {
        timer3_cnt[0] = 0;
    }
}

/* 定时器 2 超时函数 */
static void timeout2(void *parameter)
{
    static rt_uint16_t timer5Cnt=0;
    
    timer5Cnt++;
			
    if(timer5Cnt < 2)
    {
        BEEP_OFF();    
    }
    else if(timer5Cnt >= 3)                                      
    {
        timer5Cnt = 0;
    }
    else if(timer5Cnt >= 2)
    {
        BEEP_ON();	 
    }
    
    if(ALARM_EN)           //外部进行告警屏蔽时，关断蜂鸣器
    {
        timer5Cnt = 0;
    }		
}

static void timeout3(void *parameter)
{
    shg_sync= true;
    thg_sync= true;
     //STHG_TEMP_SET(GD_BUF[THG_SET], Channel_H);
     //STHG_TEMP_SET(GD_BUF[SHG_SET], Channel_G);
}

static void time_sync_entry(void *param)
{
    while(1)
    {
        if(thg_sync == true)
        {
            STHG_TEMP_SET(GD_BUF[THG_SET], Channel_H);
            thg_sync= false;
        }
        if(shg_sync == true)
        {
            STHG_TEMP_SET(GD_BUF[SHG_SET], Channel_G);
            shg_sync= false;
        }
        rt_thread_mdelay(500);
    }
}

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

int timer_init(void)
{
    rt_pin_mode(BEEP_PIN, PIN_MODE_OUTPUT);
    
    timer_led = rt_timer_create("timer1", timeout1,
                             RT_NULL, 100,
                             RT_TIMER_FLAG_PERIODIC);

    /* 启动定时器 1 */
    if (timer_led != RT_NULL) rt_timer_start(timer_led);

    /* 创建定时器 2 单次定时器 */
    timer_beep = rt_timer_create("timer2", timeout2,
                             RT_NULL,  300,
                             RT_TIMER_FLAG_PERIODIC);
    
    timer_cool_sync = rt_timer_create("timer2", timeout3,
                             RT_NULL,  1000,
                             RT_TIMER_FLAG_PERIODIC);
    
    /* 启动定时器 1 */
    if (timer_cool_sync != RT_NULL) 
        rt_timer_start(timer_cool_sync);
    
    tid1 = rt_thread_create("time_sync",
                            time_sync_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* 如果获得线程控制块，启动这个线程 */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
    return 0;
}


