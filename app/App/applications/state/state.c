/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-01     qianqiang   first version
 */
 
#define LOG_TAG      "state"
#define LOG_LVL      LOG_LVL_DBG

#include "state.h"
#include "ulog.h"
#include "stdbool.h"
#include "state_program.h"

#define THREAD_STATE_PRIORITY         (4)
#define THREAD_STATE_STACK_SIZE       (2048)
#define THREAD_STATE_TIMESLICE        (30)

static void thread_state_entry(void *parameter)
{
	rt_thread_mdelay(2000);
	while(1)
    {
        //state_program_process();
		host_state_process();
		light_state_process();
		rt_thread_mdelay(10);
    }
}

int thread_state_init(void)
{
    int result;

	light_state_init();
	host_state_init();
	//state_program_init();

    rt_thread_t tid_state = rt_thread_create("state",
											thread_state_entry, RT_NULL,
											THREAD_STATE_STACK_SIZE,
											THREAD_STATE_PRIORITY, 
											THREAD_STATE_TIMESLICE);
    
    if(RT_EOK == result)
    {
        result = rt_thread_startup(tid_state);
        log_d("thread state init.");
    }
    else
    {
        log_e("thread state init fail.");
    } 
    return result;
}
