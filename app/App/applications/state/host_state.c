/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-01     qianqiang   first version
 */
 
#define LOG_TAG      "state.host"
#define LOG_LVL      LOG_LVL_WARNING

#include "state.h"
#include "ulog.h"
#include "stdbool.h"
#include "state_program.h"
#include "host_state.h"

state_t state_host;
event_t event_host;
//extern state_program_t _state_program;

static void host_state_callback(state_t *me,event_t *event);

const char *host_state_callback_buf[] =
{
    "HOST_STATE_ROOT",    
    "HOST_STATE_IDLE",        
    "HOST_STATE_EXHUAST_PREPARED",
    "HOST_STATE_INJECT_TEST", 
    "HOST_STATE_INJECT_DELAY", 
    "HOST_STATE_INJECT", 
    "HOST_STATE_INJECT_PAUSE",       
    "HOST_STATE_INJECT_BREAK",
    "HOST_STATE_INJECT_DONE",
    "HOST_STATE_ERROR", 
    "HOST_STATE_REPAIR",
};
    
int host_state_init(void)
{
	state_host.current_state = HOST_STATE_ROOT;
	state_host.next_state  = HOST_STATE_ROOT;
	state_host.previous_state  = HOST_STATE_ROOT;

	host_event_init();
	
	return RT_EOK;
}

void host_state_process(void)
{
	event_host.sig = host_event_check();
	host_state_callback(&state_host, (event_t*)&event_host);
}

void host_state_callback(state_t *me,event_t *event)
{
	RT_ASSERT(me != RT_NULL);
	RT_ASSERT(event != RT_NULL);

    switch (me->current_state)
    {
    case HOST_STATE_ROOT:
	{
        static uint32_t state_root_time = 0;
        STATE_ENTRY_ACTION
            
            log_w("%s",host_state_callback_buf[me->current_state]);
            state_root_time = 0;
        
        STATE_TRANSITION_ACTION

            switch(event->sig)
            {
            case HOST_EVENT_HEARTBEAT:
                (state_root_time > 300) ?TRAN(HOST_STATE_IDLE):(state_root_time += HOST_STATE_HEARTBEAT_TIME);
                break;
            default:
                break;
            }

        STATE_EXIT_ACTION
       
        
        STATE_END
	}
    case HOST_STATE_IDLE:               
    {    
        STATE_ENTRY_ACTION
        
            log_w("%s",host_state_callback_buf[me->current_state]);

        STATE_TRANSITION_ACTION   
        
            switch(event->sig)
            {
            case HOST_EVENT_HEARTBEAT:
                break;
            default:
                break;
            }

        STATE_EXIT_ACTION  
            
            //key_id_write(KEY_ID_STOP);
            
        STATE_END
    }
	case HOST_STATE_EXHUAST_PREPARED:               
    {   
        STATE_ENTRY_ACTION
        
            log_w("%s",host_state_callback_buf[me->current_state]);

        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            //备妥时间超时
            case HOST_EVENT_HEARTBEAT:
//                if(time(RT_NULL) - exhuast_done_time >= HOST_STATE_EXHAUST_DONE_TIMEOUT)
//                {
//                    log_i("HOST_STATE_EXHAUST_DONE_TIMEOUT timeout.");
//                    error_insert(ERROR_ID_HOST_STATE_EXHUAST_DONE_OVERTIME);
//                    TRAN(HOST_STATE_IDLE);
//                }
                break;
			default:
                break;
            }

        STATE_EXIT_ACTION
       
            
        STATE_END
    }
	case HOST_STATE_INJECT_TEST:               
    {   
        static int32_t test_start_dosage = 0;
        
        STATE_ENTRY_ACTION
        
            log_w("%s",host_state_callback_buf[me->current_state]);
            
        STATE_TRANSITION_ACTION
            switch(event->sig)
            {
            case HOST_EVENT_HEARTBEAT:
                
                break;
            case HOST_EVENT_STOP:                                               //这里面需要解决手闸和脚闸的冲突
            case HOST_EVENT_PROXIMAL_SCREEN_STOP:           
            case HOST_EVENT_REMOTE_SCREEN_STOP:        
            case HOST_EVENT_PROXIMAL_SCREEN_HARDWARD_STOP:    
            case HOST_EVENT_REMOTE_SCREEN_HARDWARD_STOP:             
				//TRAN(HOST_STATE_INJECT_BREAK);
				break;
            default:
                break;
            }

        STATE_EXIT_ACTION
            
        STATE_END
    }
	case HOST_STATE_ERROR:               
    {   

        STATE_ENTRY_ACTION
        
            log_w("%s",host_state_callback_buf[me->current_state]);
        
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case HOST_EVENT_HEARTBEAT:

                break;
            case HOST_EVENT_INJECT_ERROR_DELETE:
				TRAN(HOST_STATE_IDLE);
				break;
//            //维修状态
//            case HOST_EVENT_REPAIR_START:
//                TRAN(HOST_STATE_REPAIR);
//                break;
            default:
                break;
            }

            
        STATE_EXIT_ACTION
        
            
        STATE_END
    }
    /*
        维修状态
        1：在线升级
        2：日志导出
        3: 
    */  
	case HOST_STATE_REPAIR:               
    {   
        
        
        STATE_ENTRY_ACTION
            
            log_w("%s",host_state_callback_buf[me->current_state]);

        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case HOST_EVENT_HEARTBEAT:
                
                break;
            case HOST_EVENT_REPAIR_DONE:
				TRAN(HOST_STATE_IDLE);
				break;
            default:
                break;
            }

        STATE_EXIT_ACTION
        
        STATE_END
    }
	default:
		log_e("state over err.");
		break;
    }

    return ;
}

/******************** API ******************************/

host_state_t host_state_get(void)
{
	return (host_state_t)state_host.current_state;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void host_state_cmd(uint8_t argc, char **argv)
{
    rt_kprintf("state_host.previous_state   =     %s      \n", host_state_callback_buf[state_host.previous_state]);
    rt_kprintf("state_host.current_state    =     %s      \n", host_state_callback_buf[state_host.current_state]);
     rt_kprintf("state_host.next_state      =     %s      \n", host_state_callback_buf[state_host.next_state]);
}
MSH_CMD_EXPORT(host_state_cmd, host_state read);

static void host_flag_cmd(uint8_t argc, char **argv)
{ 

}
MSH_CMD_EXPORT(host_flag_cmd, host flag read);

#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
