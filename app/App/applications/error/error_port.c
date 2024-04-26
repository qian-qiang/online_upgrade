/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-05     qianqinag        the first version
 */
 
#define LOG_TAG      "err.port"
#define LOG_LVL      LOG_LVL_ASSERT
//#define LOG_LVL      LOG_LVL_DBG  

#include <stdbool.h> 
#include <ulog.h> 
#include <sys/time.h>
#include "error.h"
#include "protocol_id.h"
#include "funtimer.h"
#include "serial_unpack.h"

#define THREAD_ERROR_PRIORITY         (12)
#define THREAD_ERROR_STACK_SIZE       (4096)
#define THREAD_ERROR_TIMESLICE        (20)

#define EVENT_ERROR_SEND               (1<<0)

static rt_thread_t tid_dislay = RT_NULL;
static rt_mutex_t  mutex_error = RT_NULL;
static rt_mailbox_t mailbox_heartbeat;
static rt_mailbox_t mailbox_dispay;
static rt_mailbox_t mailbox_delete;
static rt_event_t  event_display;

static uint8_t heart_beat_check_flag = true;

static void error_lvl_process(void);
static void error_heart_beat_process(void);
static void thread_error_entry(void *parameter);

void error_lock(void)
{
	RT_ASSERT(mutex_error != RT_NULL);
	rt_mutex_take(mutex_error, RT_WAITING_FOREVER);
}

void error_unlock(void)
{
	RT_ASSERT(mutex_error != RT_NULL);
	rt_mutex_release(mutex_error);
}

static uint8_t dec2bcd(uint8_t dec)
{
	return (dec + (dec/10)*6);
}

static void error_lvl_process(void)
{
   error_lvl_t lvl;
    static uint8_t lvl_re = 0xfe;
    
    lvl = error_lvl_get();
    
    if(lvl_re != lvl)
    {
        switch(lvl)
        {
        case ERROR_LVL_NONE:
            GD_BUF[ERR] = false;
            rt_timer_stop(timer_beep); 
            ERR_LED_OFF();
            BEEP_OFF();
            break;
        
        case ERROR_LVL_1:
//            beep_stop();
//            alarm_stop();
//            led_inject_set_error_ctrl(false);
//			host_event_send(HOST_EVENT_INJECT_ERROR_DELETE);
            break;

        case ERROR_LVL_2:
            GD_BUF[ERR] = error_lvl_2;
            break;
        
        case ERROR_LVL_3:
//            beep_show_err_high();
//            led_inject_set_error_ctrl(true);
//            alarm_show_err_high();
//			host_event_send(HOST_EVENT_INJECT_ERROR_INSERT);
            break;
        default:
            break;
        }
        log_d("error lvl is:%d",lvl);
        lvl_re = lvl;
    }
}

static void error_heart_beat_process(void)
{
	RT_ASSERT(mailbox_heartbeat != RT_NULL);
	rt_ubase_t str;
	static uint32_t heart_cool_board;
	uint8_t const heart_beat_time = 8;
    can_cmd_t can_cmd;
	
    if(false == heart_beat_check_flag)
    {
        return;
    }
    
	if(RT_EOK == rt_mb_recv(mailbox_heartbeat, &str, 0))
	{
		switch(str)
		{
		case ERROR_ID_COOL_BOARD_LOST :
        {
            time_t now;
            struct tm *tm, tm_tmp;

            now = time(NULL);
            tm = gmtime_r(&now, &tm_tmp);
            
			heart_cool_board = 0;
			error_delete(ERROR_ID_COOL_BOARD_LOST);
			break;
        }
		default:
			break;
		}
	} 
	
	if(heart_cool_board == heart_beat_time)
	{
		log_e("cool board lost.");
		error_insert(ERROR_ID_COOL_BOARD_LOST);
	}
	
	if(heart_cool_board > heart_beat_time)
	{
		
	}
	else
	{
		heart_cool_board++;
	}
}

static void error_display_process(void)
{
    error_id_t id = error_code_get();;
    static uint8_t is_err_exist = 1;

    rt_uint32_t id_send = id;
    
    uart_cmd_t uart_cmd_send;
    uart_cmd_send.wr = CMD_WRITE;
    uart_cmd_send.cmd = ERR; 
    uart_cmd_send.size = 4;
    rt_memcpy(&uart_cmd_send.data[0], &id_send, sizeof(id_send));
    //uart_pc_protocol_send(&uart_cmd_send);
    
}

static void thread_error_entry(void *parameter)
{
    static uint16_t error_time = 0;
    rt_thread_mdelay(2000);
    while(1)
    {      
        rt_thread_mdelay(100);
        
        if(error_time >= 100)
        {
            error_time = 0;  
            error_lvl_process();           
            error_heart_beat_process(); 
            error_display_process();
        }
        else
        {
            error_time += 10;
        }
    }
}

int error_init(void)
{
    int result;
    
	mutex_error = rt_mutex_create("error", RT_IPC_FLAG_FIFO);
	mailbox_heartbeat = rt_mb_create("heart", 32, RT_IPC_FLAG_FIFO);
    mailbox_dispay = rt_mb_create("err_display", 64, RT_IPC_FLAG_FIFO);   
    mailbox_delete = rt_mb_create("err_delete", 16, RT_IPC_FLAG_FIFO); 
    event_display = rt_event_create("err_display", RT_IPC_FLAG_FIFO);     
    
    rt_event_send(event_display, EVENT_ERROR_SEND);
 
    tid_dislay = rt_thread_create("error",
                                   thread_error_entry, RT_NULL,
                                   THREAD_ERROR_STACK_SIZE,
                                   THREAD_ERROR_PRIORITY, 
                                   THREAD_ERROR_TIMESLICE);
 
    if(RT_EOK == result)
    {
        result = rt_thread_startup(tid_dislay);
        log_d("thread error init.");
    }
    else
    {
        log_e("thread error init fail.");
    }    
    return result;
}

/**********************************************************API*************************************************************/

void error_heart_beat_send(error_id_t id)
{
    if(false == heart_beat_check_flag)
    {
        return;
    }
    
	RT_ASSERT(mailbox_heartbeat != RT_NULL);
	rt_mb_send(mailbox_heartbeat, id);
}

