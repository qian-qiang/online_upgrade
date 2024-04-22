/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-01     qianqiang   first version
 */
 
#define LOG_TAG      "event.host"
#define LOG_LVL      LOG_LVL_DBG

#include "host_state.h"
#include "ulog.h"
#include "stdbool.h"


rt_mailbox_t mb_host_event;
rt_timer_t time_host_heart;

static void host_heart_cb(void* parameter);

static void host_heart_cb(void* parameter)
{
    host_event_send(HOST_EVENT_HEARTBEAT);
}


void host_event_init(void)
{
    mb_host_event = rt_mb_create("host_event",32,RT_IPC_FLAG_FIFO);
	
	time_host_heart = rt_timer_create("host_heart",
										host_heart_cb,
										RT_NULL,
										HOST_STATE_HEARTBEAT_TIME,
										RT_TIMER_FLAG_PERIODIC);
	rt_timer_start(time_host_heart);
}

void host_event_send(host_event_t event)
{
	RT_ASSERT(mb_host_event != RT_NULL);
    rt_mb_send(mb_host_event,event);
}

host_event_t host_event_check(void)
{
	RT_ASSERT(mb_host_event != RT_NULL);
	host_event_t event = HOST_EVENT_NULL;
	
    if(RT_EOK == rt_mb_recv(mb_host_event, (rt_uint32_t*)&event, 0))
	{
		return event;
	}
	else
	{
		return HOST_EVENT_NULL;
	}
}


