/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-16     qianqinag        the first version
 */

#ifndef __INJECT_STATE_H
#define __INJECT_STATE_H
#include <rtthread.h>
#include <drv_common.h>
#include "fsm.h"

#define INJECT_STATE_HEARTBEAT_TIME                (100)
#define INJECT_STATE_INJECT_WAIT_TIMEOUT	       (30)  //9999
#define INJECT_STATE_TRIG_WAIT_TIMEOUT			   (30)   //9999

typedef enum
{
    INJECT_EVENT_NULL = 0,                         /*!< NULL                                   */
    INJECT_EVENT_HEARTBEAT,                        /*!< 心跳事件                               */       
    INJECT_EVENT_PROGRAM_LOCK,                     /*!< 方案锁定                               */
	INJECT_EVENT_PROGRAM_UNLOCK,                   /*!< 方案解锁                               */
    INJECT_EVENT_START,                            /*!< 开始事件                               */
    INJECT_EVENT_STOP,                             /*!< 停止事件                               */
    INJECT_EVENT_PAUSE,                            /*!< 暂停事件                               */
	INJECT_EVENT_STEP_SWITCH,                	   /*!< 阶段切换事件                           */
    INJECT_EVENT_INJECT_TRIG,                      /*!< 注射触发事件                           */
	INJECT_EVENT_DONE_A,	                       /*!< A筒注射完成事件                        */

}light_event_t;


typedef enum
{
    INJECT_STATE_ROOT = 0,             			 /*!< ROOT状态                               */    
    INJECT_STATE_STOP,                 			 /*!< 停止状态                               */    
    INJECT_STATE_IDLE,             		 		 /*!< 空闲状态  		     			     */
    INJECT_STATE_RUN,            			 	 /*!< 运行状态			     			     */			
    INJECT_STATE_TRIG_WAIT,                	 	 /*!< 等待状态			     			     */
    INJECT_STATE_PAUSE,            				 /*!< 暂停触发状态			     	         */
    INJECT_STATE_DELAY,                          /*!< 延时状态		     	     	         */
    INJECT_STATE_DONE,                 	 		 /*!< 完成状态			     			     */    
    INJECT_STATE_BREAK,                 	     /*!< 中止状态			     			     */   
			                                                                                 
}light_state_t;


/*  light_state.c */
int light_state_init(void);
void light_state_process(void);
light_state_t light_state_get(void);

/*	 light_event.c	*/
void light_event_init(void);
void light_event_send(light_event_t event);
light_event_t light_event_check(void);

#endif
