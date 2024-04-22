/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-16     qianqinag        the first version
 */

#ifndef __HOST_STATE_H
#define __HOST_STATE_H
#include <rtthread.h>
#include <drv_common.h>
#include "fsm.h"

#define EXHAUST_PREPARED_SUCCESS (0x0055<<8)
#define EXHAUST_PREPARED_CANCEL (0x00aa<<8)

#define INJECT_START (0x0055)
#define INJECT_PAUSE (0x0066)
#define INJECT_FINISH (0x00aa)
#define INJECT_BREAK (0x0088)

#define HOST_STATE_HEARTBEAT_TIME			(100)
#define HOSE_STATE_RESUME_PAUSE_TIME        (200)
#define HOST_STATE_EXHAUST_DONE_TIMEOUT		(1800)
#define HOST_STATE_INJECT_PASUE_TIMEOUT		(30)

typedef enum
{
    START_INJECT_TYPE_NONE = 0,
    START_INJECT_TYPE_PROXIMAL_SCREEN_START = 6,       
    START_INJECT_TYPE_PROXIMAL_HAND_START = 10,       
    START_INJECT_TYPE_REMOTE_HAND_START,              
    START_INJECT_TYPE_FOOT_START,                
    START_INJECT_TYPE_DSA_START,                
}host_start_inject_type;

typedef enum
{
    HOST_EVENT_NULL = 0,                         /*!< NULL                                   */
    HOST_EVENT_HEARTBEAT,                        /*!< 心跳事件                               */    
    HOST_EVENT_EXHAUST_CONFIRM,                  /*!< 按键排气确认                           */ 
	HOST_EVENT_PREPARED,			             /*!< 屏幕备妥                               */ 
    HOST_EVENT_PROGRAM_LOCK,                     /*!< 方案锁定                               */
	HOST_EVENT_PROGRAM_UNLOCK,                   /*!< 方案解除锁定                           */
    HOST_EVENT_PROXIMAL_SCREEN_START,            /*!< 近端屏幕开始事件                       */
    HOST_EVENT_REMOTE_SCREEN_START,              /*!< 远端屏幕开始事件                       */
    HOST_EVENT_PROXIMAL_SCREEN_HARDWARD_START,   /*!< 近端屏幕硬件开始事件                   */
    HOST_EVENT_REMOTE_SCREEN_HARDWARD_START,     /*!< 远端屏幕硬件开始事件                   */
    HOST_EVENT_PROXIMAL_HAND_START,              /*!< 硬件开始事件                           */
    HOST_EVENT_REMOTE_HAND_START,                /*!< 硬件开始事件                           */
    HOST_EVENT_FOOT_START,                       /*!< 硬件开始事件                           */
    HOST_EVENT_DSA_START,                        /*!< DSA开始事件                            */
    HOST_EVENT_STOP,                             /*!< 停止事件                               */
    HOST_EVENT_PROXIMAL_SCREEN_STOP,             /*!< 近端屏幕停止事件                       */
    HOST_EVENT_REMOTE_SCREEN_STOP,               /*!< 远端屏幕停止事件                       */
    HOST_EVENT_PROXIMAL_SCREEN_HARDWARD_STOP,    /*!< 近端屏幕停止事件                       */
    HOST_EVENT_REMOTE_SCREEN_HARDWARD_STOP,      /*!< 远端屏幕停止事件                       */
    HOST_EVENT_PROXIMAL_HAND_STOP,               /*!< 硬件停止事件                           */
    HOST_EVENT_REMOTE_HAND_STOP,                 /*!< 硬件停止事件                           */
    HOST_EVENT_FOOT_STOP,                        /*!< 硬件停止事件                           */
    HOST_EVENT_ANGLE_NO_IN_PREPARED_POSITION,    /*!< 角度不在备妥位                         */
    HOST_EVENT_DSA_START_STOP,                   /*!< DSA停止事件                            */
    HOST_EVENT_DSA_ARM_STOP,                     /*!< DSA备妥取消事件                        */
    HOST_EVENT_INJECT_TEST,                      /*!< 测试注射事件                           */
	HOST_EVENT_INJECT_TEST_DONE,                 /*!< 测试注射完成事件                       */
    HOST_EVENT_INJECT_TRIG,                      /*!< 注射触发事件                           */
	HOST_EVENT_INJECT_DONE,             		 /*!< 注射完成事件                           */
    HOST_EVENT_INJECT_BREAK,                     /*!< 注射中止事件                           */
	HOST_EVENT_INJECT_ERROR_INSERT,              /*!< 错误触发事件                           */
	HOST_EVENT_INJECT_ERROR_DELETE,				 /*!< 错误消除事件                           */
    HOST_EVENT_SYRINGE_TAKEOFF,                  /*!< A筒取下                                */
    HOST_EVENT_PRESSURE_LIMIT,                   /*!< 压力超限                               */
    HOST_EVENT_DOSAGE_LIQUID_STOP,               /*!< 剂量停止流动                           */     
    HOST_EVENT_PRESSURE_SEND_OPEN,               /*!< 压力发送打开                           */   
    HOST_EVENT_PRESSURE_SEND_CLOSE,              /*!< 压力发送关闭                           */   
    HOST_EVENT_IMAGE_ARRIVE,                     /*!< 影像到位                               */ 
	HOST_EVENT_KEY_NORMAL,                       /*!< 普通模式按键                           */
    HOST_EVENT_REPAIR_START,                     /*!< 开始修理                               */    
    HOST_EVENT_REPAIR_DONE,                      /*!< 修理结束                               */ 
    HOST_EVENT_PRESSURE_LIMIT_UPDATE,            /*!< 压力限制值更新                         */
    
}host_event_t;


typedef enum
{
    HOST_STATE_ROOT = 0,             			 /*!< ROOT状态                               */    
    HOST_STATE_IDLE,                 			 /*!< 空闲状态                               */    
    HOST_STATE_EXHUAST_PREPARED,             	 /*!< 排气完成			     			     */
    HOST_STATE_INJECT_TEST,            			 /*!< 测试注射			     			     */
    HOST_STATE_INJECT_DELAY,            		 /*!< 注射延时			     				 */	
    HOST_STATE_INJECT,            				 /*!< 注射			     				     */			
    HOST_STATE_INJECT_PAUSE,                	 /*!< 注射暂停			     			     */
    HOST_STATE_INJECT_BREAK,                 	 /*!< 注射中止			     			     */    
    HOST_STATE_INJECT_DONE,                		 /*!< 注射完成			     			     */ 
    HOST_STATE_ERROR,					 		 /*!< 错误状态			     			     */ 
    HOST_STATE_REPAIR,                           /*!< 修理状态			     			     */  
			                                                                                 
}host_state_t;

/*	host_state.c	*/
int  host_state_init(void);
void host_state_process(void);
host_state_t host_state_get(void);
/*	host_event.c	*/
void host_event_init(void);
void host_event_send(host_event_t event);
host_event_t host_event_check(void);
rt_bool_t host_state_lock_flag(void);
#endif
