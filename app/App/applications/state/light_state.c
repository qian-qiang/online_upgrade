/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-01-17     qianqiang    the first version
 */
 
#define LOG_TAG      "state.light"
#define LOG_LVL      LOG_LVL_WARNING

#include "state.h"
#include "ulog.h"
#include "stdbool.h"
#include "state_program.h"
#include "light_state.h"

state_t state_light;
event_t event_light;

static void light_state_callback(state_t *me,event_t *event);

const char *light_state_callback_buf[] =
{
    "INJECT_STATE_ROOT", 
    "INJECT_STATE_STOP",     
    "INJECT_STATE_IDLE",     
    "INJECT_STATE_RUN",        
    "INJECT_STATE_TRIG_WAIT",
    "INJECT_STATE_PAUSE",
    "INJECT_STATE_DELAY",
    "INJECT_STATE_DONE",     
    "INJECT_STATE_BREAK",
};
    
int light_state_init(void)
{
	state_light.current_state = INJECT_STATE_ROOT;
	state_light.next_state  = INJECT_STATE_ROOT;
	state_light.previous_state  = INJECT_STATE_ROOT;

	light_event_init();
	
	return RT_EOK;
}

void light_state_process(void)
{
	event_light.sig = light_event_check();
	light_state_callback(&state_light, (event_t*)&event_light);
}

light_state_t light_state_get(void)
{
	return (light_state_t)state_light.current_state;
}

static void light_state_callback(state_t *me,event_t *event)
{
	RT_ASSERT(me != RT_NULL);
	RT_ASSERT(event != RT_NULL);
	
    static int32_t light_state_delay_timeout = 0;
    static int32_t light_state_delay_time = 0;
   
    switch (me->current_state)
    {
    case INJECT_STATE_ROOT:
	{
        STATE_ENTRY_ACTION
            
            log_w("%s",light_state_callback_buf[me->current_state]);
            
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
                TRAN(INJECT_STATE_STOP);
                break;
            
            default:
                break;
            }

        STATE_EXIT_ACTION
        
        STATE_END
	}
	case INJECT_STATE_STOP:               
    {    
        STATE_ENTRY_ACTION

            //停止注射
			//state_program_step_light_stop();
            log_w("%s",light_state_callback_buf[me->current_state]);
            light_state_delay_time = 0;
            
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
                
                break;
            case INJECT_EVENT_PROGRAM_LOCK:
				TRAN(INJECT_STATE_IDLE);
				break;
			
            default:
                break;
            }

        STATE_EXIT_ACTION
        
            
        STATE_END
    }
    case INJECT_STATE_IDLE:               
    {    
        STATE_ENTRY_ACTION
            
            log_w("%s",light_state_callback_buf[me->current_state]);
		
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
                
                break;
            case INJECT_EVENT_PROGRAM_UNLOCK:
            case INJECT_EVENT_STOP:
				TRAN(INJECT_STATE_STOP);
				break;
			case INJECT_EVENT_START:
				TRAN(INJECT_STATE_RUN);
				break;
            default:
                break;
            }

        STATE_EXIT_ACTION
        
            
        STATE_END
    }
	case INJECT_STATE_RUN:               
    {   
        STATE_ENTRY_ACTION
            
//            log_w("%s",light_state_callback_buf[me->current_state]);
//			uint8_t step_turn_flag = state_program_step_turn_check();

//			if(step_turn_flag)
//			{
//				light_event_send(INJECT_EVENT_STEP_SWITCH);
//			}
//			else
//			{
//				state_program_step_light_resume();
//			}
            
        
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
                
                break;
			case INJECT_EVENT_STEP_SWITCH:
			{
//				state_prgram_turn_next();
//				state_program_class_t program_class = state_program_now_id_class_read();
//			
//				if(STATE_PROGRAM_CLASS_INJECT == program_class)
//				{			
//					state_program_step_light_run();
//				}
//				else
//				{
//					switch(state_program_step_now_id_read())
//					{
//					case STATE_PROGRAM_ID_INJECT_TRIG:
//						TRAN(INJECT_STATE_TRIG_WAIT);
//						break;
//					case STATE_PROGRAM_ID_PAUSE:	
//						TRAN(INJECT_STATE_PAUSE);
//						break;
//					case STATE_PROGRAM_ID_END:	
//						TRAN(INJECT_STATE_DONE);
//                    	break;
//                    case STATE_PROGRAM_ID_DELAY:
//                        light_state_delay_timeout = state_program_step_delay_time_read();
//                        light_state_delay_time = light_state_delay_timeout;
//                        TRAN(INJECT_STATE_DELAY);
//                        break;
//                    default:
//						log_e("_state_program.step_now.id err.");	
//						break;
//					}
//				}   
//				break;
			}
            case INJECT_EVENT_STOP:
                TRAN(INJECT_STATE_STOP);
                break;	
			case INJECT_EVENT_PAUSE:

                TRAN(INJECT_STATE_PAUSE);
                break;	
			case INJECT_EVENT_DONE_A:	

				break;				
            default:
                break;
            }

        STATE_EXIT_ACTION
           
        STATE_END
    }
	case INJECT_STATE_PAUSE:               
    {   
		static light_state_t light_state_pause_re = INJECT_STATE_ROOT;
		static uint32_t light_state_pause_time = 0;
		
        STATE_ENTRY_ACTION
            log_w("%s",light_state_callback_buf[me->current_state]);
			light_state_pause_re = (light_state_t)me->previous_state;
			light_state_pause_time = time(RT_NULL);
        
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
					
				if(time(RT_NULL) - light_state_pause_time > INJECT_STATE_INJECT_WAIT_TIMEOUT)
				{
					log_e("INJECT_STATE_PAUSE timeout.");
					TRAN(INJECT_STATE_BREAK);
				}
				break;
            case INJECT_EVENT_START:
				TRAN(light_state_pause_re);
				break;
			case INJECT_EVENT_STOP:
                TRAN(INJECT_STATE_STOP);
                break;	
            default:
                break;
            }

        STATE_EXIT_ACTION
    
        STATE_END
    }
	case INJECT_STATE_TRIG_WAIT:               
    {    
		static int32_t light_state_trig_wait_time = 0;
		
        STATE_ENTRY_ACTION
            
            log_w("%s",light_state_callback_buf[me->current_state]);
			light_state_trig_wait_time = time(RT_NULL);
			
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
				/*	状态计时	*/
				if(time(RT_NULL) - light_state_trig_wait_time > INJECT_STATE_TRIG_WAIT_TIMEOUT)
				{
					log_e("INJECT_STATE_TRIG_WAIT_TIMEOUT timeout.");
					TRAN(INJECT_STATE_BREAK);
				}
                break;
		
			case INJECT_EVENT_INJECT_TRIG:
                TRAN(INJECT_STATE_RUN);
				break;
            
            case INJECT_EVENT_STOP:
                TRAN(INJECT_STATE_STOP);
                break;	
            default:
                break;
            }

        STATE_EXIT_ACTION
        
			
        STATE_END
    }
    case INJECT_STATE_DELAY:               
    {    
        STATE_ENTRY_ACTION
            
            log_w("%s",light_state_callback_buf[me->current_state]);
		
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
                
                if(light_state_delay_time <= 0)
                {
                    log_i("light delay time over.");
					TRAN(INJECT_STATE_RUN);
                }
                else
                {
                    light_state_delay_time -= HOST_STATE_HEARTBEAT_TIME;
                }          
                break;
            case INJECT_EVENT_PAUSE:
                TRAN(INJECT_STATE_PAUSE);
                break;	
            case INJECT_EVENT_STOP:
                TRAN(INJECT_STATE_STOP);
                break;	
            default:
                break;
            }

        STATE_EXIT_ACTION
			
        STATE_END
    }
	case INJECT_STATE_DONE:               
    {    
        STATE_ENTRY_ACTION
            
            log_w("%s",light_state_callback_buf[me->current_state]);
			host_event_send(HOST_EVENT_INJECT_DONE);

        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_PROGRAM_UNLOCK:
                TRAN(INJECT_STATE_STOP);
                break;
            case INJECT_EVENT_PROGRAM_LOCK:
                TRAN(INJECT_STATE_IDLE);
                break;
            default:
                break;
            }

        STATE_EXIT_ACTION
        
            
        STATE_END
    }
	case INJECT_STATE_BREAK:               
    {    
        STATE_ENTRY_ACTION
            
            log_w("%s",light_state_callback_buf[me->current_state]);
            host_event_send(HOST_EVENT_INJECT_BREAK);
        
        STATE_TRANSITION_ACTION
        
            switch(event->sig)
            {
            case INJECT_EVENT_HEARTBEAT:
                TRAN(INJECT_STATE_STOP);
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

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void light_state_cmd(uint8_t argc, char **argv)
{
    rt_kprintf("state_light.previous_state   =     %s      \n", light_state_callback_buf[state_light.previous_state]);
    rt_kprintf("state_light.current_state    =     %s      \n", light_state_callback_buf[state_light.current_state]);
     rt_kprintf("state_light.next_state      =     %s      \n", light_state_callback_buf[state_light.next_state]);
}
MSH_CMD_EXPORT(light_state_cmd, light_state read);

#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */

