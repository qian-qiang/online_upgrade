///*
// * Copyright (c) 2019-2020, qianqinag
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2020-01-21     qianqinag        the first version
// */

//#ifndef __STATE_PROGRAM_H
//#define __STATE_PROGRAM_H
//#include <rtthread.h>
//#include <drv_common.h>
//#include "protocol_id.h"

//#define STATE_PROGRAM_STEP_NUMS_MAX			                                (12)
//#define PROTOCOL_CMD_PISTON_BACK                                            (0x0100)
//#define PROTOCOL_CMD_FIXATOR_TAKEOFF                                        (0x0101)

//typedef enum
//{
//	STATE_PROGRAM_ID_NULL = 0,
//	STATE_PROGRAM_ID_CYLINDER = 3,          //针筒运动
//    STATE_PROGRAM_ID_DELAY,                 //延时
//	STATE_PROGRAM_ID_PAUSE,                 //暂停
//	STATE_PROGRAM_ID_INJECT_TRIG,           //注射触发
//	STATE_PROGRAM_ID_START,                 //开始
//	STATE_PROGRAM_ID_END,                   //结束
//	
//	STATE_PROGRAM_ID_NUMS,
//	
//}state_program_id_t;


//typedef enum
//{
//	STATE_PROGRAM_CLASS_INJECT = 0,
//	STATE_PROGRAM_CLASS_FUN,
//	
//}state_program_class_t;

//typedef enum
//{
//    STATE_PROGRAM_SYRING_NULL = 0,
//    STATE_PROGRAM_SYRING_A, 
//}state_program_syring_t;

//typedef struct
//{
//    move_inject_param_s param_set;
//	state_program_id_t id;
//    uint32_t delay_time;
//}state_program_step_t;

//typedef struct
//{
//    uint8_t step;
//    state_program_id_t id;
//    uint16_t ml_s_10;
//    uint32_t ml_step;
//    uint8_t speed_up_time;
//    uint32_t  delay_time;
//    
//}state_program_step_set_t;


//typedef struct
//{
//    uint8_t               dsa_link_flag;

//    uint32_t              scan_delaytime;
//    uint32_t              inject_delaytime;

//    
//}state_program_dsa_link_t;

//typedef struct
//{
//    uint32_t dosage_a;
//    
//}state_program_ml_10_t;


//typedef struct
//{
//    uint32_t a;
//  
//}state_program_pressure_t;

//typedef enum 
//{
//    NULL_PREPARED = 0,
//    ONE_PREPARED,
//    MANY_PREPARED ,
//    STAGE_PREPARED,
//}prepared_t;

//typedef enum 
//{
//    UNLOGIN = 0,
//    LOGIN,
//    LOGINED ,
//}login_state;

//typedef struct
//{
//    uint8_t                 is_start;
//	uint8_t                 step_nums;
//	uint8_t                 step_num_now;
//	uint8_t				    is_lock;
//    login_state			    login;
//    uint32_t                test_ml_10;
//    uint32_t                pressure_limit;
//    state_program_ml_10_t   ml_10_set;
//    state_program_pressure_t pressure;
//    state_program_dsa_link_t dsa_link;
//    state_program_syring_t  syring;
//	state_program_step_t    step_now;
//	state_program_step_t    step[STATE_PROGRAM_STEP_NUMS_MAX];
//	prepared_t              prepared;
//    uint8_t                 air_check_popup;
//}state_program_t;

//typedef struct
//{
//    uint16_t cmd;
//    
//    uint8_t data[6];
//    
//}state_program_set_t;


//#define STATE_PROGRAM_DEFAULT                                                   \
//{                                                                               \
//    .dsa_link.dsa_link_flag                     = false,                        \
//    .dsa_link.inject_delaytime                  = 0,                            \
//    .dsa_link.scan_delaytime                    = 0,                            \
//	.step_nums                                  = 8,	                        \
//    .login                                      = UNLOGIN,	                    \
//	.step_now.id                                = STATE_PROGRAM_ID_START,       \
//	.step_num_now                               = 0,                            \
//	.step[0].id                                 = STATE_PROGRAM_ID_START,       \
//	.step[1].id                                 = STATE_PROGRAM_ID_CYLINDER,    \
//	.step[1].param_set.ml = 50,                                                           \
//	.step[1].param_set.ml_s_10 = 30,                                                      \
//	.step[2].id                                 = STATE_PROGRAM_ID_INJECT_TRIG, \
//	.step[3].id                                 = STATE_PROGRAM_ID_PAUSE,       \
//    .step[4].id                                 = STATE_PROGRAM_ID_DELAY,       \
//    .step[4].delay_time                         = 10000,                        \
//	.step[5].id                                 = STATE_PROGRAM_ID_END,         \
//    .prepared                                   = NULL_PREPARED,                 \
//    .air_check_popup                            = true                          \
//}\


//void state_program_init(void);
//state_program_class_t state_program_now_id_class_read(void);
//uint8_t state_program_step_turn_check(void);
//void state_program_step_inject_resume(void);
//void state_program_step_inject_stop(void);
//void state_program_step_inject_run(void);
//void state_prgram_turn_next(void);
//state_program_id_t state_program_step_now_id_read(void);
//void state_program_set(state_program_set_t program_set);
//void state_program_arrive_ml_set(uint32_t ml_10);
//void state_program_test_ml_set(uint32_t ml_10);
//void state_program_process(void);
//state_program_t *state_program_handle(void);
//void state_program_inject_time_send(uint32_t ms);
//void sate_program_arrive_time_send(uint32_t ms);
//void state_program_scan_time_send(uint32_t ms);
//void state_program_time_send(uint32_t run_ms, uint32_t pause_ms);
//state_program_syring_t state_program_syring_read(void);
//uint32_t state_program_step_delay_time_read(void);
//rt_err_t state_program_start_check(void);
//void state_program_many_prepared_lock_set(uint8_t flag);
//rt_err_t state_program_inject_test_check(void);
//void state_program_lock_error_delete(void);
//void state_program_inject_state_send(topctrl_to_ct_inject_state_t state);
//void state_program_run_send(uint8_t flag);
//void state_program_arm_send(uint8_t flag);
//void state_program_dosage_send(uint8_t dosage);
//void state_program_is_start_set(uint8_t value);
//rt_err_t state_program_pressure_set(uint32_t a);
//login_state state_program_login_state(void);
//void state_program_lock_set(uint8_t flag);
//prepared_t prepared_style_get(void);
//void prepared_style_set(prepared_t prepared);
//void many_prepared_inject_over_again_init(void);
//uint8_t state_program_dsa_link_flag_read(void);
//int state_program_inject_state_to_screen(uint16_t inject_state);
//void state_program_mode_pop(void);
//uint32_t state_program_all_dosage_read(void);
//#endif
