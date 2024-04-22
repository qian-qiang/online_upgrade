///*
// * Copyright (c) 2019-2020, qianqinag
// *
// * SPDX-License-Identifier: Apache-2.0
// *
// * Change Logs:
// * Date           Author       Notes
// * 2022-01-21     qianqiang    the first version
// */
// 
//#define LOG_TAG      "state.program"
//#define LOG_LVL      LOG_LVL_DBG

//#include "ulog.h"
//#include "state.h"
//#include "board.h"
#include "state_program.h"
//#include "stdbool.h"
//#include "thread_can.h"

//#define TIMER_ERR_INJECT_TEST_DELETE_TIMEOUT       (3000)

//static rt_mq_t         mq_pressure;
//static rt_mq_t         mq_program_set;
//static rt_mailbox_t    mb_test_ml_10;
//state_program_t _state_program = STATE_PROGRAM_DEFAULT;

//const char *step_easy_name_buf[] = 
//{
//	"NULL",
//    "NULL",
//    "NULL",
//	"CYLINDER",
//	"DELAY",
//	"PAUSE",
//    "TRIG",
//	"START",
//	"END",
//};

//const char *step_name_buf[] = 
//{
//	"STATE_PROGRAM_ID_NULL",
//   	"STATE_PROGRAM_ID_NULL",
//  	"STATE_PROGRAM_ID_NULL",
//	"STATE_PROGRAM_ID_CYLINDER",
//	"STATE_PROGRAM_ID_PAUSE",
//	"STATE_PROGRAM_ID_INJECT_TRIG",
//    "STATE_PROGRAM_ID_DELAY",
//	"STATE_PROGRAM_ID_START",
//	"STATE_PROGRAM_ID_END",
//};

//static rt_err_t program_inject_param_check(move_inject_param_s param);
//static rt_err_t state_program_inject_param_check(uint8_t step);
//static rt_err_t state_program_check(void);
//void state_program_lock_set(uint8_t flag);
//static void state_program_step_nums_set(uint8_t num);
//static void state_program_step_id_set(uint8_t step, state_program_id_t id);
//static void state_program_step_param_set(uint8_t step, uint32_t ml_10, uint32_t ml_s_10, uint8_t up_time);
//static void state_program_print(void);
//static rt_err_t state_program_dosage_check(void);
//static rt_err_t state_program_dsa_link_check(void);
//static void state_program_step_set(state_program_step_set_t step_set);

////state_program初始化
//void state_program_init(void)
//{
//    mq_pressure =  rt_mq_create("pressure_set", sizeof(state_program_pressure_t), 32, RT_IPC_FLAG_FIFO);
//    mq_program_set = rt_mq_create("program_set", sizeof(state_program_set_t), 64, RT_IPC_FLAG_FIFO);
//    mb_test_ml_10 = rt_mb_create("test_ml_10", 5, RT_IPC_FLAG_FIFO);
//   
//	_state_program.step_num_now = 0;
//	_state_program.is_lock = false;
//	log_i("state_program step %d.",_state_program.step_num_now);
//}

////外部输入的测试注射的ml
//void state_program_test_ml_set(uint32_t ml_10)
//{
//    RT_ASSERT(mb_test_ml_10 != RT_NULL);
//    rt_mb_send(mb_test_ml_10, ml_10);
//}
////串口输入的program数据 由state_program_read读取处理到注射阶段参数
//void state_program_set(state_program_set_t program_set)
//{
//	RT_ASSERT(mq_program_set != RT_NULL);
//    rt_mq_send(mq_program_set, &program_set, sizeof(state_program_set_t));
//}
////state_program的参数处理
//static void state_program_read(void)
//{
//	RT_ASSERT(mq_program_set != RT_NULL);
//    static state_program_step_set_t step_set;
//    static uint8_t step_num = 0;
//    uart_cmd_t uart_cmd;
//    state_program_set_t program_set;
//    rt_err_t result;
//    rt_uint32_t value;
//    
//    //由state_program_pressure_set设置压力
//    result = rt_mq_recv(mq_pressure, &_state_program.pressure, sizeof(state_program_pressure_t), 0);
//  
//    result = rt_mb_recv(mb_test_ml_10, &value, 0);
//    if(RT_EOK == result)
//    {
//        _state_program.test_ml_10 = value;
//        log_d("_state_program.test_ml_10: %d ml",_state_program.test_ml_10/10);
//    }
//    
//    result = rt_mq_recv(mq_program_set, &program_set, sizeof(state_program_set_t),0);
//    if(RT_EOK == result)
//    {
//        switch(program_set.cmd)
//        {
//            case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_LOGIN:
//                log_d("PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_LOGIN state is %d",program_set.data[2]);	
//                _state_program.login = (login_state)program_set.data[2];
//                break;
//            //dsa扫描延时和注射延时
//            case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_DSALINK_DELAY        	    : 
//                log_d("PROTOCOL_CMD_SCREEN_TO_TOPCTRL_DSALINK_DELAY");	
//                if(program_set.data[0] == 0)
//                {
//                    log_i("scan_delaytime set %d s",((program_set.data[1] << 8) + program_set.data[2]));
//                    _state_program.dsa_link.scan_delaytime = 1000 * ((program_set.data[1] << 8) + program_set.data[2]);
//                    _state_program.dsa_link.inject_delaytime = 0;
//                }
//                else
//                {
//                    log_i("inject_delaytime set %d s",((program_set.data[1] << 8) + program_set.data[2]));
//                    _state_program.dsa_link.scan_delaytime = 0;
//                    _state_program.dsa_link.inject_delaytime = 1000 * ((program_set.data[1] << 8) + program_set.data[2]);;
//                }
//                break;
//            //方案阶段个数
//            case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_PROGRAM_NUMS        	    :
//				log_d("PROTOCOL_CMD_SCREEN_TO_TOPCTRL_PROGRAM_NUMS");
//                step_num = program_set.data[1];
//                state_program_step_nums_set(program_set.data[1]);
//                break;
//            //阶段注射参数
//            case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_PARAM_STEP     	        : 
//            {
//                log_d("PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_PARAM_STEP");
//                //data1低4位：设置阶段的ID
//                step_set.id = (state_program_id_t)(program_set.data[0] & 0xf);
//                //data1高4位：4位数据，当前设置的阶段数（1-10）
//                step_set.step = (program_set.data[0] >> 4) & 0xf;
//                
//                if(step_set.id == STATE_PROGRAM_ID_CYLINDER)
//                {
//                    //当阶段ID为针筒时，
//                    //data1高4位：设置阶段数
//                    //data2-3：16位数据，注射速度*10，单位ml/s
//                    step_set.ml_s_10 = program_set.data[2] + (program_set.data[1] << 8);
//                    //data4：8位数据，注射容量，单位ml 转换成pluse
//                    step_set.ml_step = program_set.data[3];
//                    //data5：8位数据，上升时间，单位s*10
//                    step_set.speed_up_time = program_set.data[4];
//                    
//                    log_d("step_set:step %d,id %d, %d ml_s(min)(*10), %d ml,speed_up_time %d s",
//                    step_set.step,
//                    step_set.id,
//                    step_set.ml_s_10,
//                    step_set.ml_step ,
//                    step_set.speed_up_time);
//                }
//               
//                if(step_set.id == STATE_PROGRAM_ID_DELAY)
//                {
//                    //当阶段 ID为延时时，data2无效，data3 data4组合成16为数据，延时时间
//                    step_set.delay_time = ((program_set.data[3] << 8) + program_set.data[4]) * 1000;
//                    
//                    log_d("step_set:step %d,id %d,delay_time %d",step_set.step, step_set.id, step_set.delay_time);
//                }
//                
//                if(step_set.id == STATE_PROGRAM_ID_PAUSE)
//                {
//                    //当阶段 ID为暂停时，data无效
//                    log_d("step_set:step %d,id %d, pause ",step_set.step, step_set.id);
//                }
//                
//                if(step_set.id == STATE_PROGRAM_ID_INJECT_TRIG)
//                {
//                    //当阶段 ID为暂停时，data无效
//                    log_d("step_set:step %d,id %d, trig ",step_set.step, step_set.id);
//                }
//                
//                state_program_step_set(step_set);
//                
//                //主状态机进入备妥状态 发送state_program_lock_set（）进行备妥检查  检查通过就进入锁定状态
//                if(step_num == step_set.step)
//                {
//                    host_event_send(HOST_EVENT_PREPARED);
//                }
//                break;
//            }
//           case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_PUPOP                :
//               //排气确认弹窗
//               if(program_set.data[0] == ERROR_ID_POPUP_ID_CHECK_AIR)
//               {
//                    if(program_set.data[2] == 1)
//                    {
//                        log_d("ERROR_ID_POPUP_ID_CHECK_AIR CANCEL");
//                        _state_program.air_check_popup = true;
//                        host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//                        _state_program.prepared =  NULL_PREPARED;
//                    }
//                    if(program_set.data[2] == 2)
//                    {
//                        log_d("ERROR_ID_POPUP_ID_CHECK_AIR SURE");
//                        _state_program.air_check_popup = false;
//                        _state_program.is_lock = true;
//                        host_event_send(HOST_EVENT_PROGRAM_LOCK);
//                    }
//               }
//               //剂量不足弹窗
//               if(program_set.data[0] == ERROR_ID_DOSE_NOT_ENOUGH_ONE_PREPARED)
//               {
//                    if(program_set.data[2] == 1)
//                    {
//                        log_d("DOSE_NOT_ENOUGH_ONE_PREPARED CANCEL");
//                    }
//                    if(program_set.data[2] == 2)
//                    {
//                        log_d("DOSE_NOT_ENOUGH_ONE_PREPARED SURE");
//                        program_set.data[3] = display_dosage_read();
//                        
//                        screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_PAOGRAM_DOSAGE, program_set.data[3]);
//                        screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_PAOGRAM_SPEED_UP_DOWN_TIME, false);
//                        
//                        step_set.ml_step = program_set.data[3];
//                        step_set.speed_up_time = false;
//                        state_program_step_set(step_set);
//                        _state_program.prepared =  ONE_PREPARED;
//                        host_event_send(HOST_EVENT_PREPARED);
//                    }
//               }
//               //mi/min模式确认
//               if(program_set.data[0] == ERROR_ID_CONFIRM_ML_MIN_MODE)
//               {
//                    if(program_set.data[2] == 1)
//                    {
//                        host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//                        error_delete(ERROR_ID_POPUP_ID_CHECK_AIR);
//                        log_d("ERROR_ID_CONFIRM_ML_MIN_MODE CANCEL");
//                    }
//                    if(program_set.data[2] == 2)
//                    {
//                        log_d("ERROR_ID_CONFIRM_ML_MIN_MODE SURE");
//                    }
//               }
//                break;
//           case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_ONE_PREPARED:
//               	log_d("PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_ONE_PREPARED");   
//                _state_program.prepared = ONE_PREPARED;
//                break;
//          case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_MANY_PREPARED:
//               	log_d("PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_MANY_PREPARED");   
//                _state_program.prepared = MANY_PREPARED;
//                break;
//          case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_STAGE_PREPARED:
//               	log_d("PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_STAGE_PREPARED");   
//                _state_program.prepared = STAGE_PREPARED;
//                break;
//          case PROTOCOL_CMD_PROXIMAL_SCREEN_TO_CTRLBOX_DSA_LINKAGE_F:
//              _state_program.dsa_link.dsa_link_flag =  program_set.data[2];
//                break;
//          case PROTOCOL_CMD_PISTON_BACK:
//          case PROTOCOL_CMD_FIXATOR_TAKEOFF:
//                if(_state_program.air_check_popup == false)
//                {
//                    log_d("air_check_popup set true");  
//                    _state_program.air_check_popup = true;
//                }
//                break;
//          default:
//              log_e("state_program_read get error msg");
//        }
//    }
//}

////获取当前的备妥阶段
//prepared_t prepared_style_get(void)
//{
//    prepared_t prepared_style = NULL_PREPARED;
//    if(_state_program.prepared == ONE_PREPARED)
//    {
//        prepared_style = ONE_PREPARED;
//    }
//    else if(_state_program.prepared == MANY_PREPARED)
//    {
//        prepared_style = MANY_PREPARED;
//    }
//    else if(_state_program.prepared == STAGE_PREPARED)
//    {
//        prepared_style = STAGE_PREPARED;
//    } 
//    return prepared_style;
//}

//void many_prepared_inject_over_again_init(void)
//{
//    _state_program.step_num_now = 0;
//    //proximal_screen_beep_show_low();
//    rt_memcpy(&_state_program.step_now, &_state_program.step[_state_program.step_num_now], sizeof(state_program_step_t));
//}

///***    
//注射过程中当前阶段完成 （包括暂停阶段）
//命令	数据
//00 24	00 55
//data：16位数据
//注：在进入“按开始进入下一阶段”阶段时，按下开始键也是发送这条命令，理解为当前阶段完成，“中断”阶段和延时阶段完成也是发送这一条命令，注射过程中正常或异常完成，只能发送阶段完成或者停止中的一条，禁止两条同时发送。
// **/
//void state_prgram_turn_next(void)
//{
//    if(_state_program.step[_state_program.step_num_now].id != STATE_PROGRAM_ID_START)
//	{
//        uart_cmd_t uart_cmd;
//        uart_cmd.dir = CAN_ID_PROXIMAL_CTRLBOX_TO_SCREEN;
//        uart_cmd.cmd = PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_STEP_DONE; 
//        uart_cmd.data[0] = 0;
//        uart_cmd.data[1] = 0x55;
//        uart_cmd.size = 2;
//        uart_screen_protocol_send(&uart_cmd);
//        log_i("inject step done.");
//	}	
//    
//	_state_program.step_num_now++;
//	log_i("start inject: state_program step %d.", _state_program.step_num_now);
//	
//	rt_memcpy(&_state_program.step_now, &_state_program.step[_state_program.step_num_now], sizeof(state_program_step_t));
//	log_d("start inject: step_name is %s", step_name_buf[_state_program.step_now.id]);
//}
////读取当前的阶段id
//state_program_id_t state_program_step_now_id_read(void)
//{
//	return _state_program.step_now.id;
//}
////读取当前阶段的延时时间
//uint32_t state_program_step_delay_time_read(void)
//{
//    return _state_program.step[_state_program.step_num_now].delay_time;
//}
////读取当前阶段是注射还是功能
//state_program_class_t state_program_id_class_read(state_program_id_t id)
//{
//	RT_ASSERT(_state_program.step_now.id <= STATE_PROGRAM_ID_NUMS);
//	
//	state_program_class_t _class;
//	
//	switch(id)
//	{
//    case STATE_PROGRAM_ID_CYLINDER:
//		_class = STATE_PROGRAM_CLASS_INJECT;
//		break;
//    case STATE_PROGRAM_ID_INJECT_TRIG:
//    case STATE_PROGRAM_ID_PAUSE:
//    case STATE_PROGRAM_ID_DELAY:
//    case STATE_PROGRAM_ID_START:
//    case STATE_PROGRAM_ID_END:
//		_class = STATE_PROGRAM_CLASS_FUN;
//		break;
//	default:
//		log_i("_state_program.step_now.id err.");
//		break;
//		
//	}
//	return _class;
//}
////串口传入的注射的步数参数
//static void state_program_step_set(state_program_step_set_t step_set)
//{
//    rt_err_t result = RT_EOK;
//    
//    if(step_set.step > STATE_PROGRAM_STEP_NUMS_MAX - 2)
//    {
//        log_i("step %d set max 10",step_set.step);
//        result = -RT_ERROR;
//    }
//    
//    if((step_set.id == STATE_PROGRAM_ID_NULL) || (step_set.id > STATE_PROGRAM_ID_INJECT_TRIG))
//    {
//        log_i("step set id error.");
//        result = -RT_ERROR;
//    }
//    
//    if(result == -RT_ERROR)
//    {
//        return;
//    }
//    
//    //当前的阶段数号和id号对应设置
//    state_program_step_id_set(step_set.step, step_set.id);

//    switch(step_set.id)
//    {
//    case STATE_PROGRAM_ID_CYLINDER:
//        state_program_step_param_set(step_set.step, step_set.ml_step, step_set.ml_s_10, step_set.speed_up_time);    
//        break;
//    case STATE_PROGRAM_ID_DELAY:
//        _state_program.step[step_set.step].delay_time = step_set.delay_time;
//        break;
//    case STATE_PROGRAM_ID_INJECT_TRIG:
//    case STATE_PROGRAM_ID_PAUSE:
//        
//        break;
//     default:
//        break;
//    }
//}
////读取当前id的类型注射还是功能
//state_program_class_t state_program_now_id_class_read(void)
//{
//	return state_program_id_class_read(_state_program.step_now.id);
//}
////阶段检查为功能的时候才可进行切换
//uint8_t state_program_step_turn_check(void)
//{
//	uint8_t turn_flag = false;
//	
//	if(STATE_PROGRAM_CLASS_FUN == state_program_now_id_class_read())
//	{
//		turn_flag = true;
//	}
//	return turn_flag;
//}

////停止注射
//void state_program_step_inject_stop(void)
//{
//	move_inject_part_id_write(MOVE_INJECT_ID_STOP); 
//}
////恢复注射 
//void state_program_step_inject_resume(void)
//{
//	move_inject_part_id_t id = MOVE_INJECT_ID_NULL;
//	
//	switch(_state_program.step_now.id)
//	{
//	case STATE_PROGRAM_ID_CYLINDER:
//		id = MOVE_INJECT_ID_A;
//        //led_inject_set_a(LED_INJECT_FLASH);
//		break;
//	default:
//		break;
//	}
//	move_inject_part_id_write(id); 
//}
////运行注射阶段  传递参数到move_inject里执行
//void state_program_step_inject_run(void)
//{
//	move_inject_part_id_t id = MOVE_INJECT_ID_NULL;
//	move_inject_param_s param_send = {0};

//	switch(_state_program.step_now.id)
//	{
//	case STATE_PROGRAM_ID_CYLINDER:
//		id = MOVE_INJECT_ID_A;
//		//led_inject_set_a(LED_INJECT_FLASH);
//		break;
//	default: 
//		break;
//	}
//	
//	rt_thread_mdelay(60);
//	
//	rt_memcpy(&param_send, &_state_program.step_now.param_set, sizeof(move_inject_param_s) );
//	move_inject_param_write(&param_send);
//	move_inject_part_id_write(id); 
//}

////finsh显示接收到的注射阶段数据
//static void state_program_print(void)
//{
//	log_i("state_program_print:step nums:%2d", _state_program.step_nums-2);
//	
//	log_i("state_program_print:dsa_link: %s", _state_program.dsa_link.dsa_link_flag ? "true" : "false");
//	if(_state_program.dsa_link.dsa_link_flag)
//	{
//		log_i("state_program_print:scan_delaytime:%d ms", _state_program.dsa_link.scan_delaytime/1000);
//		log_i("state_program_print:inject_delaytime:%d ms", _state_program.dsa_link.inject_delaytime/1000);
//	}

//	log_i("state_program_print:step-num:  name       ml    ml_s  speed_up_time");
//	
//	for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//		if(STATE_PROGRAM_CLASS_INJECT == state_program_id_class_read(_state_program.step[i].id))
//		{
//			log_i("state_program_print:step-%2d: %4s   %3d   %3d.%d  %3d.%d", i,step_easy_name_buf[_state_program.step[i].id],
//			_state_program.step[i].param_set.ml,
//			_state_program.step[i].param_set.ml_s_10/10,_state_program.step[i].param_set.ml_s_10%10,
//            _state_program.step[i].param_set.speed_up_time_10/10,_state_program.step[i].param_set.speed_up_time_10%10);
//		}
//		else if(_state_program.step[i].id == STATE_PROGRAM_ID_DELAY)
//        {
//            log_i("state_program_print:step-%2d:  %4s     %d ms", i, step_easy_name_buf[_state_program.step[i].id], _state_program.step[i].delay_time);
//        }
//        else if(_state_program.step[i].id == STATE_PROGRAM_ID_PAUSE)
//        {
//            log_i("state_program_print:step-%2d:  %4s", i, step_easy_name_buf[_state_program.step[i].id]);
//        }
//        else if(_state_program.step[i].id == STATE_PROGRAM_ID_INJECT_TRIG)
//        {
//            log_i("state_program_print:step-%2d:  %4s", i, step_easy_name_buf[_state_program.step[i].id]);
//        }
//        else
//		{
//			log_i("state_program_print:step-%2d:  %4s", i, step_easy_name_buf[_state_program.step[i].id]);
//		}
//	}
//}

////锁定成功后删除锁定相关的错误
//void state_program_lock_error_delete(void)
//{

//}

////是否锁定条件的判断
//void state_program_lock_set(uint8_t flag)
//{
//	if(flag)
//	{
//		if(RT_EOK == state_program_check())
//		{
//			log_i("state program stage check success");
//            //dsa联动开启
//            if(_state_program.dsa_link.dsa_link_flag == true)
//            {
//                //dsa允许备妥
//                if(dsa_link_read_pin_dsa_i_disarm() == VALID_PIN_DSA_I_DISARM)
//                {
//                    //启动注射信号正常
//                    if(dsa_link_read_pin_dsa_i_start() != VALID_PIN_DSA_I_START)
//                    {
//                        if( _state_program.air_check_popup == true)
//                        {
//                            popup_insert(ERROR_ID_POPUP_ID_CHECK_AIR);
//                        }
//                        else
//                        {
//                            _state_program.is_lock = true;
//                            host_event_send(HOST_EVENT_PROGRAM_LOCK);
//                        } 
//                    }
//                    //启动注射信号提前开启
//                    else
//                    {
//                        log_i("state programing read start pin invalid, cnnot prepared");
//                        host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//                    }
//                }
//                //dsa不允许备妥
//                else
//                {
//                    error_insert(ERROR_ID_DSA_DISARM_SINGLE_NOT_ALLOW_PREPARED);
//                    log_i("state program read disarm pin invalid, cancel prepared");
//                    host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//                    _state_program.prepared =  NULL_PREPARED;
//                }
//            }
//            //dsa联动关闭
//            else
//            {
//                if( _state_program.air_check_popup == true)
//                {
//                    popup_insert(ERROR_ID_POPUP_ID_CHECK_AIR);
//                }
//                else
//                {
//                    _state_program.is_lock = true;
//                    host_event_send(HOST_EVENT_PROGRAM_LOCK);
//                }
//            }
//		}
//		else
//		{
//            log_i("state program stage check failed");
//            host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//            _state_program.prepared =  NULL_PREPARED;
//		}
//		state_program_print();
//	}
//	else
//	{
//		_state_program.is_lock = false;
//		host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//		log_i("state program unlock");
//        _state_program.prepared =  NULL_PREPARED;
//	}
//    _state_program.step_num_now = 0;
//}

////是否锁定条件的判断
//void state_program_many_prepared_lock_set(uint8_t flag)
//{
//	if(flag)
//	{
//        if(RT_EOK == state_program_check())
//		{
//			log_i("state program stage check success");
//            //dsa联动开启
//            if(_state_program.dsa_link.dsa_link_flag == true)
//            {
//                //dsa允许备妥
//                if(dsa_link_read_pin_dsa_i_disarm() == VALID_PIN_DSA_I_DISARM)
//                {
//                    //启动注射信号正常
//                    if(dsa_link_read_pin_dsa_i_start() != VALID_PIN_DSA_I_START)
//                    {
//                        _state_program.is_lock = true;
//                        host_event_send(HOST_EVENT_PROGRAM_LOCK);
//                    }
//                    //启动注射信号提前开启
//                    else
//                    {

//                    }
//                }
//                //dsa不允许备妥
//                else
//                {
//                    error_insert(ERROR_ID_DSA_DISARM_SINGLE_NOT_ALLOW_PREPARED);
//                    log_i("state program read disarm pin invalid, cancel prepared");
//                    host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//                    _state_program.prepared =  NULL_PREPARED;
//                }
//            }
//            //dsa联动关闭
//            else
//            {  
//                _state_program.is_lock = true;
//                host_event_send(HOST_EVENT_PROGRAM_LOCK);
//            }
//		}
//		else
//		{
//            log_i("state program stage check failed");
//            host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//            _state_program.prepared =  NULL_PREPARED;
//		}
//		state_program_print();
//	}
//	else
//	{
//         
//		_state_program.is_lock = false;
//		host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//		log_i("state program unlock");
//        _state_program.prepared =  NULL_PREPARED;
//	}
//    _state_program.step_num_now = 0;
//}

////方案阶段个数设置
//static void state_program_step_nums_set(uint8_t num)
//{
//	uint8_t value = 0;
//	
////	if(_state_program.is_lock)
////	{
////		log_i("state program lock.");
////		return;
////	}
//	
//	value = num + 2;
//	
//	if(value > STATE_PROGRAM_STEP_NUMS_MAX)
//	{
//		log_i("step num %d max %d", value, STATE_PROGRAM_STEP_NUMS_MAX);
//		return;
//	}	
//	
//	rt_memset(_state_program.step, 0, sizeof(state_program_step_t)*STATE_PROGRAM_STEP_NUMS_MAX);
//	
//	log_d("step nums :%d", value);
//	
//	_state_program.step_nums = value;
//	_state_program.step[0].id = STATE_PROGRAM_ID_START;
//	_state_program.step[value-1].id = STATE_PROGRAM_ID_END;
//	_state_program.step_now.id = STATE_PROGRAM_ID_START;
//	_state_program.step_num_now = 0;
//}

////阶段号里面的id号赋值
//static void state_program_step_id_set(uint8_t step, state_program_id_t id)
//{
////	if(_state_program.is_lock)
////	{
////		log_i("state_program_step_id_set : state program lock.");
////		return;
////	}
//	
//	if((step == 0) || (step >= (_state_program.step_nums -1)))
//	{
//		log_i("step id set error with step.");
//		return;
//	}
//	
//	_state_program.step[step].id = (state_program_id_t)(id);
//	
//	//log_d("step-%2d mode:%s",step,step_easy_name_buf[_state_program.step[step].id]);
//}
////注射阶段的针筒的参数设置 
//static void state_program_step_param_set(uint8_t step, uint32_t ml_step, uint32_t ml_s_10, uint8_t up_time)
//{
////	if(_state_program.is_lock)
////	{
////		log_i("state_program_step_param_set : state program lock.");
////		return;
////	}
//	
//	if((step == 0) || (step >= (_state_program.step_nums -1)))
//	{
//		log_i("step param ch set error with step.");
//		return;
//	}
//	
//	_state_program.step[step].param_set.ml = ml_step;
//	_state_program.step[step].param_set.ml_s_10 = ml_s_10;
//	_state_program.step[step].param_set.speed_up_time_10 = up_time;
//    
//	//log_d("step-%2d: %3d ml %3d.%d ml_s %3d.%d up_time", step,ml_step/PARAM_ML_TO_PLUSE,ml_s_10/10,ml_s_10%10,up_time/10,up_time%10);
//}
////注射阶段的参数检查
//static rt_err_t program_inject_param_check(move_inject_param_s param)
//{
//	if((param.ml == 0) || (param.ml_s_10 == 0))
//	{
//		return -RT_ERROR;
//	}
//	
//	return RT_EOK;
//}

////总阶段的参数检查 
//static rt_err_t state_program_inject_param_check(uint8_t step)
//{
//	//A筒注射阶段 A筒参数不能为零
//	rt_err_t result = RT_EOK;
//    
//	if(STATE_PROGRAM_CLASS_FUN == state_program_id_class_read(_state_program.step[step].id))
//	{
//		return RT_EOK;
//	}
//	else
//	{
//		switch(_state_program.step[step].id)
//		{
//		case STATE_PROGRAM_ID_CYLINDER:
//			result = program_inject_param_check(_state_program.step[step].param_set);
//			break;
//		default:
//			break;
//		}
//	}
//	return result;
//}

////剂量检查
//static rt_err_t state_program_dosage_check(void)
//{
//	rt_err_t result = RT_EOK;
//	float dosage_less = (float)display_dosage_read();
//	float dosage_set = 0;
//    _state_program.ml_10_set.dosage_a = 0;
//    
//    //计算阶段的剂量
//	for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//		switch(_state_program.step[i].id)
//		{
//		case STATE_PROGRAM_ID_CYLINDER:
//			dosage_set += _state_program.step[i].param_set.ml;
//            dosage_set += (_state_program.step[i].param_set.ml_s_10 * _state_program.step[i].param_set.speed_up_time_10)/100;
//			break;
//		default:
//			break;
//		}
//	}

//    _state_program.ml_10_set.dosage_a = (uint32_t)dosage_set;

//    if((_state_program.prepared == ONE_PREPARED) || (_state_program.prepared == MANY_PREPARED))
//    {
//        if((dosage_set != 0) && (dosage_set > dosage_less))
//        {
//            error_insert(ERROR_ID_DOSE_NOT_ENOUGH_ONE_PREPARED);
//            log_i("state_program_dosage_check : one/many prepared syring set %d ml is more than %d ml",(int32_t)dosage_set, (int32_t)dosage_less);
//            result = -RT_ERROR;
//            return result;
//        }
//    }
//    
//    //多阶段参数异常-药量过多-单个注射参数为零
//    if(_state_program.prepared == STAGE_PREPARED)
//    {
//        //药量过多
//        if((dosage_set != 0) && (dosage_set > dosage_less))
//        {
//            error_insert(ERROR_ID_PARAM_FAULT_STAGE_PREPARED);
//            log_i("state_program_dosage_check : many prepared syring set %d ml is more than %d ml",dosage_set, (uint32_t)dosage_less);
//            result = -RT_ERROR;
//            return result;
//        }
//        //单个注射参数为零
//        for(uint8_t i = 0; i < _state_program.step_nums; i++)
//        {
//            switch(_state_program.step[i].id)
//            {
//            case STATE_PROGRAM_ID_CYLINDER:
//                if((_state_program.step[i].param_set.ml == 0) || (_state_program.step[i].param_set.ml_s_10 == 0))
//                {
//                    error_insert(ERROR_ID_PARAM_FAULT_STAGE_PREPARED);
//                    log_i("state_program_dosage_check : many prepared syring ml or ml/s is zero");
//                    result = -RT_ERROR;
//                    return result;
//                }
//                break;
//            default:
//                break;
//            }
//        }
//    }
//    
//	return result;
//}

////dsa连接检查
//static rt_err_t state_program_dsa_link_check(void)
//{
//	rt_err_t result = RT_EOK;
//	
//	for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//		switch(_state_program.step[i].id)
//		{
//		case STATE_PROGRAM_ID_INJECT_TRIG:
//			if(false == _state_program.dsa_link.dsa_link_flag)
//			{
//				result = -RT_ERROR;
//			}
//			break;
//		default:
//			break;
//		}
//	}
//	
//	if(-RT_ERROR == result)
//	{
//		log_i("program can't set STEP_INJECT_TRIG without ct_link.");
//	}
//	
//	return result;
//}

////检查针筒安装情况
//static rt_err_t state_program_syring_check(void)
//{
//    rt_err_t result = RT_EOK;
//    uint8_t step_a_num;
//    
//    step_a_num = 0;

//    
//    if(_state_program.dsa_link.dsa_link_flag)
//    {
//        step_a_num++;
//    }
//    
//	for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//		switch(_state_program.step[i].id)
//		{
//        case STATE_PROGRAM_ID_CYLINDER:
//            step_a_num++;
//            break;
//		default:
//			break;
//		}
//	}
//    
//    if(step_a_num > 0) 
//    {
//        _state_program.syring = STATE_PROGRAM_SYRING_A;
//    }

//    else
//    {
//        _state_program.syring = STATE_PROGRAM_SYRING_NULL;
//    }
//    
//    switch(_state_program.syring)
//    {                         
//    case STATE_PROGRAM_SYRING_A       :
//        if(false == move_condition_syringe_install_state_get(true))
//        {
//            log_i("syring no install ok,program have step.");
//            result = -RT_ERROR;
//        }
//        break;      
//    case STATE_PROGRAM_SYRING_NULL:
//        break;
//    default:
//        log_i("program param error.");
//        result = -RT_ERROR;
//        break;
//    }
//    
//    return result;
//}
////检查注射步骤是否有问题
//static rt_err_t state_program_step_a_b_null_check(void)
//{
//    rt_err_t result = RT_EOK;
//    uint8_t step_a_num;
//    
//    step_a_num = 0;
//    
//	for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//		switch(_state_program.step[i].id)
//		{
//        case STATE_PROGRAM_ID_CYLINDER:
//            step_a_num++;
//            break;
//		default:
//			break;
//		}
//	}
//    
//    if(step_a_num == 0)
//    {
//        log_i("no step a.");
//        result = -RT_ERROR;
//    }

//    for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//        if(_state_program.step[i].id == STATE_PROGRAM_ID_NULL)
//        {
//            log_i("step %d is NULL.", i);
//            result = -RT_ERROR;
//        }
//    }
//    
//    return result;
//}
////检查注射步骤延时参数
//static rt_err_t state_program_step_delay_check(void)
//{
//    rt_err_t result = RT_EOK;
//    
//    for(uint8_t i = 0; i < _state_program.step_nums; i++)
//	{
//		if((_state_program.step[i].id == STATE_PROGRAM_ID_DELAY) && (_state_program.step[i].delay_time == 0))
//        {
//            log_i("step %d delay can't be zero.",i);
//            result = -RT_ERROR;
//        }
//	}
//    return result;
//}

////检查角度允许条件
//static rt_err_t state_program_angle_check(void)
//{
//    rt_err_t result = RT_EOK;
//    if(move_condition_perpared_position_get() == false)
//    {
//        error_insert(ERROR_ID_PROGRAM_ANGLE_FAULT);
//        result = -RT_ERROR;
//    }
//    return result;
//}

////rfid次数允许条件
//static rt_err_t state_program_rfid_check(void)
//{
//    rt_err_t result = RT_EOK;
//    if(rfid_enable_read() != true)
//    {
//        return result = -RT_ERROR;
//    }
//    return result;
//}

////注射模式检查
//static rt_err_t state_program_mode_check(void)
//{
//    rt_err_t result = RT_EOK;
//    //ml_s is true      ml/s可用任何模式
//    //ml_min is false   ml/min只能用于多次和单次模式
//    if(move_condition_move_unit_get() == true)
//    {
//        return result;
//    }
//    else{
//        if(_state_program.prepared == STAGE_PREPARED)
//        {
//            return result = -RT_ERROR;
//        }
//        else{
//            //error_insert(ERROR_ID_CONFIRM_ML_MIN_MODE);
//            return result;
//        }
//    }
//}

////病人登录检查
//static rt_err_t state_program_login_check(void)
//{
//    rt_err_t result = RT_EOK;

//    if(_state_program.login != UNLOGIN)
//    {
//        return result;
//    }
//    else{  
//        return result = -RT_ERROR;
//    }
//}

////检查注射步骤
//static rt_err_t state_program_check(void)
//{
//	rt_err_t result = RT_EOK;
//    
//    /* 病人登录检查 */
//    if(-RT_ERROR == state_program_login_check())
//    {
//        //error_insert();
//        log_i("state_program_login_check unlogin");
//        return -RT_ERROR;
//    }

//	/* 注射模式检查 */
//    if(-RT_ERROR == state_program_mode_check())
//    {
//        log_i("ml/min sport mode cannot use stage mode");
//        return -RT_ERROR;
//    }
//	/* 开始阶段和结束阶段确认 */
//	if(_state_program.step[0].id != STATE_PROGRAM_ID_START)
//	{
//		log_i("program start step id error");
//		result = -RT_ERROR;
//	}
//	
//	if(_state_program.step[_state_program.step_nums - 1].id != STATE_PROGRAM_ID_END)
//	{
//		log_i("program end step id error");
//		result = -RT_ERROR;
//	}
//	
//	/* 开始阶段和结束阶段之间不能为空阶段 也不能为开始和结束阶段 */
//    rt_bool_t step_no_inject_flag = false;
//	for(uint8_t step = 1; step < (_state_program.step_nums - 1); step++)
//	{
//		if((_state_program.step[step].id == STATE_PROGRAM_ID_NULL)
//		|| (_state_program.step[step].id >= STATE_PROGRAM_ID_START))
//		{
//			log_i("program step %d id error", step);
//			return result = -RT_ERROR;
//		}
//        if(_state_program.step[step].id == STATE_PROGRAM_ID_CYLINDER)
//        {
//            step_no_inject_flag = true;
//        } 
//	}
//    if(step_no_inject_flag == false)
//    {
//        log_i("program step no inject step");
//        error_insert(ERROR_ID_PARAM_FAULT_STAGE_PREPARED);
//        return result = -RT_ERROR;
//    }
//	
//	/* 注射阶段参数检测 */
//	for(uint8_t step = 1; step < (_state_program.step_nums - 1); step++)
//	{
//		if(-RT_ERROR == state_program_inject_param_check(step))
//		{
//			log_i("program step %d param error", step);
//            error_insert(ERROR_ID_PARAM_FAULT_STAGE_PREPARED);
//			return -RT_ERROR;
//		}
//	}
//    
//	/* 药量检测 */
//	if(-RT_ERROR == state_program_dosage_check())
//	{
//        host_event_send(HOST_EVENT_PROGRAM_UNLOCK);
//        log_i("state program dosage check failed");
//		return -RT_ERROR;
//	}
//	
//    /*  延时阶段延时时间不能为0  */
//    if(-RT_ERROR == state_program_step_delay_check())
//    {
//        log_i("state program step delay check failed");
//        error_insert(ERROR_ID_PARAM_DELAY_STAGE_PREPARED);
//        return -RT_ERROR;
//    }
//    
//	/* 联动检测是否有中断阶段 */
////	if(-RT_ERROR == state_program_dsa_link_check())
////	{
////        log_i("state program dsa linked but step have trig check failed");
////        error_insert(ERROR_ID_PROGRAM_TRIG_FAULT);
////		return -RT_ERROR;
////	}
//    
//    /* 针筒安装检测 */
//    if(-RT_ERROR == state_program_syring_check())
//    {
//        log_i("state program syring check failed");
//        error_insert(ERROR_ID_PROGRAM_SYRINGE_INSTALL_FAULT);
//        return -RT_ERROR;
//    }
//    /* 角度检测 */
//    if(-RT_ERROR == state_program_angle_check())
//    {
//        log_i("angle is %d ,not allow prepared!", move_condition_angle_get());
//        error_insert(ERROR_ID_PROGRAM_ANGLE_FAULT);
//        return -RT_ERROR;
//    }
//    
//    /* rfid次数检测 */
//    if(-RT_ERROR == state_program_rfid_check())
//    {
//        log_i("rfid num is zero ,not allow inject!");
//        error_insert(ERROR_ID_INJECT_COUNT_0);
//        return -RT_ERROR;
//    }
//	return result;
//}
////返回是否有针筒注射
//state_program_syring_t state_program_syring_read(void)
//{
//    return _state_program.syring;
//}
////注射开始前检查
//rt_err_t state_program_start_check(void)
//{
//    rt_err_t result = RT_EOK;
//    
//    result = state_program_syring_check();
//    
//    if(-RT_ERROR == result)
//    {
//        error_insert(ERROR_ID_PROGRAM_SYRINGE_INSTALL_FAULT);
//    }
//    
//    return result;
//}
////设置注射开始标志位
//void state_program_is_start_set(uint8_t value)
//{
//    _state_program.is_start = value;
//}

////测试注射检查
//rt_err_t state_program_inject_test_check(void)
//{
//    rt_err_t result = RT_EOK;
//    
//    if(false == _state_program.is_lock)
//    {
//        
//    }
//    else
//    {
//        uint32_t dosage_set = _state_program.ml_10_set.dosage_a + _state_program.test_ml_10;
//        uint32_t dosage_last = display_dosage_read();
//        
//        if(dosage_set > dosage_last)
//        {
//            log_i("dosage test %d max dosage %d.",dosage_set, dosage_last);
//            error_insert(ERROR_ID_DOSE_TEST_INJECT_DOSAGE_NOT_ENOUGH);
//            result = -RT_ERROR;
//        }
//    }
//    
//    return result;
//}

////外部设置program的压力
//rt_err_t state_program_pressure_set(uint32_t a)
//{
//	if(mq_pressure != RT_NULL)
//    {
//        state_program_pressure_t display;
//        
//        display.a = a;
//        
//        return rt_mq_send(mq_pressure, &display, sizeof(state_program_pressure_t));
//    }
//    return -RT_ERROR;
//}
////发送注射时间到屏幕
//void state_program_inject_time_send(uint32_t ms)
//{
//	uint16_t time;
//	uart_cmd_t uart_cmd;
//	
//	time = ms/1000;

//    uart_cmd.dir = CAN_ID_PROXIMAL_CTRLBOX_TO_SCREEN;
//    uart_cmd.cmd = PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_INJECT_TIME; 
//    uart_cmd.data[0] = time >> 8;
//    uart_cmd.data[1] = time & 0xff;
//    uart_cmd.size = 2;
//    uart_screen_protocol_send(&uart_cmd);
//    
//    //倒计时的rgb和beep效果
//    proximal_screen_rgb_set(Misaka_RGB_Bling_Color_Green,Misaka_RGB_Flash_Mode);
//    topkey_rgb_set(Misaka_RGB_Bling_Color_Green,Misaka_RGB_Flash_Mode);
//}

////发送扫描时间到屏幕
//void state_program_scan_time_send(uint32_t ms)
//{
//	uart_cmd_t uart_cmd;
//	uint16_t time;
//	
//	time = ms/1000;
//	
//    uart_cmd.dir = CAN_ID_PROXIMAL_CTRLBOX_TO_SCREEN;
//    uart_cmd.cmd = PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_SCAN_TIME; 
//    uart_cmd.data[0] = time >> 8;
//    uart_cmd.data[1] = time & 0xff;
//    uart_cmd.size = 2;
//    uart_screen_protocol_send(&uart_cmd);
//}

////发送注射状态到屏幕
//int state_program_inject_state_to_screen(uint16_t inject_state)
//{
//    if(inject_state == INJECT_START)
//    {
//        proximal_screen_beep_set(ScreenBoard_Beep_Short_Single_Tone);
//    }
//    else
//    {
//        
//    }
//    return screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_INJECT_START_DONE_STOP, inject_state); 
//}

////注射模式弹窗
//void state_program_mode_pop(void)
//{
//    //ml_s is true      ml/s可用任何模式
//    //ml_min is false   ml/min只能用于多次和单次模式
//    if(move_condition_move_unit_get() == true)
//    {
//        return;
//    }
//    else{
//        if(_state_program.prepared == STAGE_PREPARED)
//        {
//            return;
//        }
//        else{
//            error_insert(ERROR_ID_CONFIRM_ML_MIN_MODE);
//            return;
//        }
//    }
//}

//void sate_program_arrive_time_send(uint32_t ms)
//{

//}

//void state_program_time_send(uint32_t run_ms, uint32_t pause_ms)
//{
//    
//}

//void state_program_arm_send(uint8_t flag)
//{
////    can_cmd_t can_cmd;
////    
////    uint8_t data;
////    
////    if(flag)
////    {
////        data = 1;
////    }
////    else
////    {
////        data = 0;
////    }
////    
////    if(_state_program.ct_link.ct_link_flag || (!flag))
////    {
////        can_cmd.cmd = PROTOCOL_CMD_TOPCTRL_TO_CTRLBOX_LINK_ARM;
////        can_cmd.dir = CAN_ID_TOPCTRL_TO_CTRLBOX;
////        can_cmd.data[0] = data;
////        can_cmd.data[1] = 0;
////        can_cmd.data[2] = 0;
////        can_cmd.data[3] = 0;
////        can_cmd.size = 4;
////        can_protocol_send(&can_cmd);
////    }
////    
////    can_cmd.cmd = PROTOCOL_CMD_TOPCTRL_TO_CTRLBOX_LED_ARM;
////    can_cmd.dir = CAN_ID_TOPCTRL_TO_CTRLBOX;
////    can_cmd.data[0] = data;
////    can_cmd.data[1] = 0;
////    can_cmd.data[2] = 0;
////    can_cmd.data[3] = 0;
////    can_cmd.size = 4;
////    can_protocol_send(&can_cmd);
////    
////    can_cmd.cmd = PROTOCOL_CMD_TOPCTRL_TO_CT_ARM;
////    can_cmd.dir = CAN_ID_TOPCTRL_TO_CT;
////    can_cmd.data[0] = data;
////    can_cmd.data[1] = 0;
////    can_cmd.data[2] = 0;
////    can_cmd.data[3] = 0;
////    can_cmd.data[4] = 0;
////    can_cmd.data[5] = 0;
////    can_cmd.size = 6;
////    can_protocol_send(&can_cmd);
//}

//void state_program_run_send(uint8_t flag)
//{
////    can_cmd_t can_cmd;
////    
////    uint8_t data;
////    
////    if(flag)
////    {
////        data = 1;
////    }
////    else
////    {
////        data = 0;
////    }
////    
////    if(_state_program.ct_link.ct_link_flag || (!flag))
////    {
////        can_cmd.cmd = PROTOCOL_CMD_TOPCTRL_TO_CTRLBOX_LINK_RUN;
////        can_cmd.dir = CAN_ID_TOPCTRL_TO_CTRLBOX;
////        can_cmd.data[0] = data;
////        can_cmd.data[1] = 0;
////        can_cmd.data[2] = 0;
////        can_cmd.data[3] = 0;
////        can_cmd.size = 4;
////        can_protocol_send(&can_cmd);
////        
////        can_cmd.cmd = PROTOCOL_CMD_TOPCTRL_TO_CT_RUN;
////        can_cmd.dir = CAN_ID_TOPCTRL_TO_CT;
////        can_cmd.data[0] = data;
////        can_cmd.data[1] = 0;
////        can_cmd.data[2] = 0;
////        can_cmd.data[3] = 0;
////        can_cmd.data[4] = 0;
////        can_cmd.data[5] = 0;
////        can_cmd.size = 6;
////        can_protocol_send(&can_cmd);
////    }
//}

//void state_program_dosage_send(uint8_t dosage)
//{
////    uart_cmd_t uart_cmd;
////    uart_cmd.dir = CAN_ID_PROXIMAL_CTRLBOX_TO_SCREEN;
////    uart_cmd.cmd = PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_DOSAGE; 
////    uart_cmd.data[0] = (dosage) >> 8;
////    uart_cmd.data[1] = (dosage) & 0xff;
////    uart_cmd.size = 2;
////    uart_screen_protocol_send(&uart_cmd);
//}

//void state_program_inject_state_send(topctrl_to_ct_inject_state_t state)
//{
////    RT_ASSERT(state <= TOPCTRL_TO_CT_INJECT_STATE_INJECT_PAUSE);
////    
////    can_cmd_t can_cmd;
////    
////    can_cmd.cmd = PROTOCOL_CMD_TOPCTRL_TO_CT_INJECT_STATE;
////    can_cmd.dir = CAN_ID_TOPCTRL_TO_CT;
////    can_cmd.data[0] = state;
////    can_cmd.data[1] = 0;
////    can_cmd.data[2] = 0;
////    can_cmd.data[3] = 0;
////    can_cmd.data[4] = 0;
////    can_cmd.data[5] = 0;
////    can_cmd.size = 6;
////    can_protocol_send(&can_cmd);
//}

///******************** API ************************/
//login_state state_program_login_state(void)
//{
//   return _state_program.login;
//}

//void prepared_style_set(prepared_t prepared)
//{
//    _state_program.prepared = prepared;
//}

//state_program_t *state_program_handle(void)
//{
//    return &_state_program;
//}

//void state_program_process(void)
//{
//    state_program_read();
//}

//uint32_t state_program_all_dosage_read(void)
//{
//    return _state_program.ml_10_set.dosage_a;
//}

//uint8_t state_program_dsa_link_flag_read(void)
//{
//    return _state_program.dsa_link.dsa_link_flag;
//}

//#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
//#include <finsh.h>
//static void program(uint8_t argc, char **argv)
//{
//#define CMD_LOCK_INDEX                 0
//#define CMD_TRIG_INDEX                 1
//#define CMD_ARRIVE_INDEX               2
//#define CMD_IMAGE_ARRIVE_INDEX         3
//	
//	uint8_t i;
//	uint8_t flag;
//	
//    const char* help_info[] =
//    {
//        [CMD_LOCK_INDEX]            = "program lock [on|off]",
//		[CMD_TRIG_INDEX]            = "program trig ",
//        [CMD_ARRIVE_INDEX]          = "program arrive [on|off]",
//        [CMD_IMAGE_ARRIVE_INDEX]    = "program image_arrive",
//    };

//    if (argc < 2)
//    {
//        rt_kprintf("Usage:\n");
//        for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
//        {
//            rt_kprintf("%s\n", help_info[i]);
//        }
//        rt_kprintf("\n");
//    }
//    else
//    {
//		const char *operator = argv[1];
//		
//		if (!strcmp(operator, "lock"))
//		{
//			const char *operator = argv[2];
//			if (!strcmp(operator, "on"))
//			{
//				flag = true;
//				state_program_lock_set(flag);
//			}
//			else if (!strcmp(operator, "off"))
//			{
//				flag = false;
//				state_program_lock_set(flag);
//			}
//			else
//			{
//				rt_kprintf("Usage: %s.\n", help_info[CMD_LOCK_INDEX]);
//				return;
//			}
//		}
//		else if (!strcmp(operator, "trig"))
//		{
//			host_event_send(HOST_EVENT_INJECT_TRIG);
//		}
//        else if (!strcmp(operator, "image_arrive"))
//		{
//			host_event_send(HOST_EVENT_IMAGE_ARRIVE);
//		}
//		else
//		{
//			rt_kprintf("Usage:\n");
//			for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
//			{
//				rt_kprintf("%s\n", help_info[i]);
//			}
//			rt_kprintf("\n");
//			return;
//		}
//    }
//}
//MSH_CMD_EXPORT(program, program set);

//static void state_program_date_read(uint8_t argc, char **argv)
//{
//    rt_kprintf(" _state_program.air_check_popup:             %s \n ", _state_program.air_check_popup?"have":"no");  
//    rt_kprintf("_state_program.dsa_link:                    %s \n ", _state_program.dsa_link.dsa_link_flag?"yes":"no");  
//    rt_kprintf("_state_program.is_lock:                     %s \n ", _state_program.is_lock?"yes":"no");  
//    rt_kprintf("_state_program.is_start:                    %s \n ", _state_program.is_start?"yes":"no");
//    rt_kprintf("_state_program.login:                       %s \n ", _state_program.login?"logined":"unlogin"); 
//}
//MSH_CMD_EXPORT(state_program_date_read, state_program_date_read);
//#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */

