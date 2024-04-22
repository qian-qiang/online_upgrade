/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-17     qianqinag        the first version
 */
 
#define LOG_TAG      "state.event"
#define LOG_LVL      LOG_LVL_DBG

#include "light_state.h"
#include "ulog.h"
#include "stdbool.h"
#include "param.h"

rt_mailbox_t mb_light_event;
rt_timer_t time_light_heart;


static void light_heart_cb(void* parameter)
{
    light_event_send(INJECT_EVENT_HEARTBEAT);
}

void light_event_init(void)
{
    mb_light_event = rt_mb_create("light_event",32,RT_IPC_FLAG_FIFO);
	
	time_light_heart = rt_timer_create("light_heart",
										light_heart_cb,
										RT_NULL,
										100,
										RT_TIMER_FLAG_PERIODIC);
	rt_timer_start(time_light_heart);
}

void light_event_send(light_event_t event)
{
	RT_ASSERT(mb_light_event != RT_NULL);
    rt_mb_send(mb_light_event,event);
}

light_event_t light_event_check(void)
{
	RT_ASSERT(mb_light_event != RT_NULL);
	light_event_t event = INJECT_EVENT_NULL;
	
    if(RT_EOK == rt_mb_recv(mb_light_event, (rt_uint32_t*)&event, 0))
	{
		return event;
	}
	else
	{
		return INJECT_EVENT_NULL;
	}
}


