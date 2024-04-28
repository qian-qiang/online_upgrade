/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-01     qianqiang   first version
 */
 
#define LOG_TAG      "param"
#define LOG_LVL      LOG_LVL_DBG
 
#include "param.h"
#include "ulog.h"
#include "easyflash.h"

#define THREAD_PARAM_PRIORITY         (19)
#define THREAD_PARAM_STACK_SIZE       (1028)
#define THREAD_PARAM_TIMESLICE        (400)

rt_mutex_t mutex_param;
rt_thread_t thread_param;
rt_mq_t     mq_param_set;

typedef struct
{
    char *key;
    uint32_t value;
        
}param_set_t;


void param_key_value_init(char* key, uint32_t *value, uint32_t value_default)
{
	RT_ASSERT(key != RT_NULL);
	RT_ASSERT(value != RT_NULL);
    RT_ASSERT(mutex_param != RT_NULL);
    size_t len;
	uint32_t temp;
	
	rt_mutex_take(mutex_param, RT_WAITING_FOREVER);
    
    len = ef_get_env_blob((const char*)key, &temp, sizeof(temp) , NULL);
    if(0 == len)
    {
        temp = value_default;
        ef_set_env_blob(key, &temp, sizeof(temp));
		*value = temp;
    }
    else
    {
        len = ef_get_env_blob((const char*)key, &temp, sizeof(temp) , NULL);
        *value = temp;
    }
	rt_mutex_release(mutex_param);
}

void param_key_value_set(char* key, uint32_t *value)
{
	RT_ASSERT(key != RT_NULL);
    RT_ASSERT(mq_param_set != RT_NULL);
    param_set_t param_set;
    
    param_set.key = key;
    param_set.value = *value;
    
    rt_mq_send(mq_param_set, &param_set, sizeof(param_set_t));
}

static void param_key_value_write(char* key, uint32_t *value)
{
	RT_ASSERT(key != RT_NULL);
    RT_ASSERT(mutex_param != RT_NULL);
    
	rt_mutex_take(mutex_param, RT_WAITING_FOREVER);
    if( EF_NO_ERR != ef_set_env_blob((const char*)key, value, sizeof(value)))
    {
        log_e("env set error.");
    }
	rt_mutex_release(mutex_param);
    log_i("param %s set %d.", key, *value);
}

static void thread_param_entry(void *parameter)
{
	rt_err_t result;
    param_set_t param_set;
    rt_thread_mdelay(1000);
	while(1)
    {
        result = rt_mq_recv(mq_param_set, &param_set, sizeof(param_set_t), RT_WAITING_FOREVER);
        if(RT_EOK == result)
        {
            param_key_value_write(param_set.key, &param_set.value);
        }
    }
}

int param_init(void)
{
    int result;
    
	mutex_param = rt_mutex_create("param", RT_IPC_FLAG_PRIO);
    mq_param_set = rt_mq_create("param", sizeof(param_set_t), 16, RT_IPC_FLAG_FIFO);
    
    thread_param = rt_thread_create("param",
							    thread_param_entry, RT_NULL,
							    THREAD_PARAM_STACK_SIZE,
							    THREAD_PARAM_PRIORITY, 
							    THREAD_PARAM_TIMESLICE);
    
    if(RT_EOK == result)
    {
        result = rt_thread_startup(thread_param);
        log_d("thread param init.");
    }
    else
    {
        log_e("thread param init fail.");
    }
    
    return result;
}

