/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG      "task"
#define LOG_LVL      LOG_LVL_DBG

#include "ulog.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "at24c08.h"
#include "seed.h"
#include "aom.h"
#include "pump.h"
#include "ShgThg.h"
#include "fpga.h"
#include "task.h"
#include "cd74xx.h"
#include "pd.h"
#include "funtimer.h"
#include "fan.h"

#define THREAD_STACK_SIZE   1024
#define THREAD_PRIORITY     20
#define THREAD_TIMESLICE    10

static rt_thread_t tid_init_task = RT_NULL;
static rt_thread_t uart_process_task = RT_NULL;
static rt_thread_t other_task = RT_NULL;
static rt_thread_t laser_task = RT_NULL;

#define scanNum                                                 20                 //检测告警位扫描次数
#define CLK_FS                                                  200000000          //FPGA 基频时钟200MHz
                                                     
rt_uint8_t rtcFlag[2] = {0,0};                                                     //保存RTC日期和时间更新标志
rt_uint8_t keyState = 1;                                                           //保存按钮状态  

rt_bool_t UserSetValueChaFlag[4] = {false,false,false,false};                      //UI某值修改标志位分别是FreCha\BurstCha\aom1delayCha\aom2delayCha
rt_bool_t laserSwitchFlag = true;                                                  //SWITCH动作标志位

//保存泵的开关状/态
typedef struct
{ 
	rt_uint8_t pump1State;					 										//泵1开关标志位,1-ON,0-OFF
	rt_uint8_t pump2State;					 										//泵2开关标志位,1-ON,0-OFF
	rt_uint8_t pump3State;					 										//泵3开关标志位,1-ON,0-OFF
	rt_uint8_t pump4State;					 										//泵4开关标志位,1-ON,0-OFF
}laserPartEnaStruct;

laserPartEnaStruct laserPart={0,0,0,0};  


/********************************************************************************
*函数名: get_eeprom_data
*参数  : 无
*功能  : 写入或读取EEPROM数据
*返回值: 无
*********************************************************************************/
void get_eeprom_data(void)
{
	if(at24c08_check() == RT_EOK ) //EEPROM读取
	{	
        GD_BUF[SN1]                    = AT24CXX_ReadOneByte(AddSn1Byte1)|(AT24CXX_ReadOneByte(AddSn1Byte2)<<8)|(AT24CXX_ReadOneByte(AddSn1Byte3)<<16)|(AT24CXX_ReadOneByte(AddSn1Byte4)<<24);					
		GD_BUF[SN2]                    = AT24CXX_ReadOneByte(AddSn2Byte1)|(AT24CXX_ReadOneByte(AddSn2Byte2)<<8)|(AT24CXX_ReadOneByte(AddSn2Byte3)<<16)|(AT24CXX_ReadOneByte(AddSn2Byte4)<<24);			
		GD_BUF[SN3]                    = AT24CXX_ReadOneByte(AddSn3Byte1)|(AT24CXX_ReadOneByte(AddSn3Byte2)<<8)|(AT24CXX_ReadOneByte(AddSn3Byte3)<<16)|(AT24CXX_ReadOneByte(AddSn3Byte4)<<24);									
		GD_BUF[PUMP1_I_SET]	           = AT24CXX_ReadOneByte(AddPump1ISetByte1)|(AT24CXX_ReadOneByte(AddPump1ISetByte2)<<8);
		GD_BUF[PUMP2_I_SET]	           = AT24CXX_ReadOneByte(AddPump2ISetByte1)|(AT24CXX_ReadOneByte(AddPump2ISetByte2)<<8);
		GD_BUF[PUMP3_I_SET]	           = AT24CXX_ReadOneByte(AddPump3ISetByte1)|(AT24CXX_ReadOneByte(AddPump3ISetByte2)<<8);
		GD_BUF[PUMP4_I_SET]	           = AT24CXX_ReadOneByte(AddPump4ISetByte1)|(AT24CXX_ReadOneByte(AddPump4ISetByte2)<<8);
		GD_BUF[PUMP1_I_MAX]	           = AT24CXX_ReadOneByte(AddPump1IMaxByte1)|(AT24CXX_ReadOneByte(AddPump1IMaxByte2)<<8);
		GD_BUF[PUMP2_I_MAX]	           = AT24CXX_ReadOneByte(AddPump2IMaxByte1)|(AT24CXX_ReadOneByte(AddPump2IMaxByte2)<<8);
		GD_BUF[PUMP3_I_MAX]	           = AT24CXX_ReadOneByte(AddPump3IMaxByte1)|(AT24CXX_ReadOneByte(AddPump3IMaxByte2)<<8);
		GD_BUF[PUMP4_I_MAX]	           = AT24CXX_ReadOneByte(AddPump4IMaxByte1)|(AT24CXX_ReadOneByte(AddPump4IMaxByte2)<<8);
		GD_BUF[PUMP_TEMP_H_ALARM]      = AT24CXX_ReadOneByte(AddPumpTempHAlarmB1)|(AT24CXX_ReadOneByte(AddPumpTempHAlarmB2)<<8);
		GD_BUF[PUMP_TEMP_L_ALARM]      = AT24CXX_ReadOneByte(AddPumpTempLAlarmB1)|(AT24CXX_ReadOneByte(AddPumpTempLAlarmB2)<<8);
		GD_BUF[CAVITY_TEMP_H_ALARM]    = AT24CXX_ReadOneByte(AddCavityTempHAlarmB1)|(AT24CXX_ReadOneByte(AddCavityTempHAlarmB2)<<8);
		GD_BUF[CAVITY_TEMP_L_ALARM]    = AT24CXX_ReadOneByte(AddCavityTempLAlarmB1)|(AT24CXX_ReadOneByte(AddCavityTempLAlarmB2)<<8);
		
		GD_BUF[SEED_LASER_ENERGY_CTRL] = AT24CXX_ReadOneByte(AddSeedEnergy);
		GD_BUF[SEED_TIME]              = AT24CXX_ReadOneByte(AddSeedLaserTimeB1)|(AT24CXX_ReadOneByte(AddSeedLaserTimeB2)<<8)|(AT24CXX_ReadOneByte(AddSeedLaserTimeB3)<<16);
		GD_BUF[SEED_TYPE]              = AT24CXX_ReadOneByte(AddSeedType); 
		 
		GD_BUF[BURST_NUM]              = AT24CXX_ReadOneByte(AddFqBurst);
		GD_BUF[FQ_RF_SCALE]            = AT24CXX_ReadOneByte(AddFqRf);
		GD_BUF[FQ_FRE]                 = AT24CXX_ReadOneByte(AddFqFreB1)|(AT24CXX_ReadOneByte(AddFqFreB2)<<8);
		GD_BUF[FQ_DELAY]               = AT24CXX_ReadOneByte(AddFqDelay);

		GD_BUF[SQ_RF_SCALE]            = AT24CXX_ReadOneByte(AddSqRfB1)|(AT24CXX_ReadOneByte(AddSqRfB2) << 8);
		GD_BUF[SQ_ATTEN_SEL]           = AT24CXX_ReadOneByte(AddSqAttenSel);
		GD_BUF[SQ_ADD_TIME]            = AT24CXX_ReadOneByte(AddSqAddTimeB1)|(AT24CXX_ReadOneByte(AddSqAddTimeB2)<<8);
		GD_BUF[SQ_TYPE]                = AT24CXX_ReadOneByte(AddSqType);  
        
        // 开机默认关闭泵，防止开机直接出光
        GD_BUF[PUMP1_ENA]              = 0;                
        GD_BUF[PUMP2_ENA]              = 0;                        
        GD_BUF[PUMP3_ENA]              = 0;
        GD_BUF[PUMP4_ENA]              = 0;
        GD_BUF[SQ_STANDBY_ENA]         = on;

        GD_BUF[SEED_POWER_ENA] = off;
        GD_BUF[SEED_LASER_ENA] = off;
        GD_BUF[FQ_POWER_ENA] = off;
        GD_BUF[SQ_POWER_ENA] = off;
        
        GD_BUF[SWITCH] = off;
		AT24CXX_WriteOneByte(AddSwitch, GD_BUF[SWITCH]);
		laserSwitchFlag	= true;	
        
//        AT24CXX_WriteOneByte(AddPump1Ena, GD_BUF[PUMP1_ENA]);
//        AT24CXX_WriteOneByte(AddPump2Ena, GD_BUF[PUMP2_ENA]);
//        AT24CXX_WriteOneByte(AddPump3Ena, GD_BUF[PUMP3_ENA]);
//        AT24CXX_WriteOneByte(AddPump4Ena, GD_BUF[PUMP4_ENA]);
//        AT24CXX_WriteOneByte(AddSqStandby, GD_BUF[SQ_STANDBY_ENA]);

//        AT24CXX_WriteOneByte(AddSeedPower, GD_BUF[SEED_POWER_ENA]);
//        AT24CXX_WriteOneByte(AddSeedLaser, GD_BUF[SEED_LASER_ENA]);
//        AT24CXX_WriteOneByte(AddFqPower, GD_BUF[FQ_POWER_ENA]);
//        AT24CXX_WriteOneByte(AddSqPower, GD_BUF[SQ_POWER_ENA]);
        
        /*qq record*/
        GD_BUF[LASER_TYPE]             = AT24CXX_ReadOneByte(AddLaserType);
        GD_BUF[LASER_POWSEL]           = AT24CXX_ReadOneByte(AddPowerSel);

        GD_BUF[IR_HM_ALARM_ENA]        = AT24CXX_ReadOneByte(AddIrhmEna);
        GD_BUF[UV_HM_ALARM_ENA]        = AT24CXX_ReadOneByte(AddUvhmEna);
        GD_BUF[IR_HM_LIMIT_SET]        = AT24CXX_ReadOneByte(AddIrLimitSet1)|(AT24CXX_ReadOneByte(AddIrLimitSet2)<<8);
        GD_BUF[UV_HM_LIMIT_SET]        = AT24CXX_ReadOneByte(AddUvLimitSet1)|(AT24CXX_ReadOneByte(AddUvLimitSet2)<<8);
        GD_BUF[IR_HM_ALARM_CNT]        = AT24CXX_ReadOneByte(AddirhmAlarmCnt);
        GD_BUF[UV_HM_ALARM_CNT]        = AT24CXX_ReadOneByte(AdduvhmAlarmCnt);
        /*qq record*/
      
		GD_BUF[LASER_MODE]             = AT24CXX_ReadOneByte(AddLaserMode);
		GD_BUF[MODE_SW]                = AT24CXX_ReadOneByte(AddModeSw);
		GD_BUF[EXT_MODE]               = AT24CXX_ReadOneByte(AddExtMode);
		GD_BUF[MODULO_DIVIDER]         = AT24CXX_ReadOneByte(AddModuloDividerB1)|(AT24CXX_ReadOneByte(AddModuloDividerB2)<<8);
		GD_BUF[BURST_HOLDOFF]          = AT24CXX_ReadOneByte(AddBurstHoldOff);
		GD_BUF[BURST_LENGTH]           = AT24CXX_ReadOneByte(AddBurstLengthB1)|(AT24CXX_ReadOneByte(AddBurstLengthB2)<<8); 
		GD_BUF[EXT_MODE_ACTLEVEL]      = AT24CXX_ReadOneByte(AddExtModeLevel);
		GD_BUF[SWITCH]                 = AT24CXX_ReadOneByte(AddSwitch);
		
		GD_BUF[SHG_SET]                = AT24CXX_ReadOneByte(AddShgTempSetB1)|(AT24CXX_ReadOneByte(AddShgTempSetB2)<<8);
		GD_BUF[THG_SET]                = AT24CXX_ReadOneByte(AddThgTempSetB1)|(AT24CXX_ReadOneByte(AddThgTempSetB2)<<8);
		
		GD_BUF[AIRPUMP_EN]             = AT24CXX_ReadOneByte(AddAirPumpEna);   
		GD_BUF[AIRPUMP_FREQ]           = AT24CXX_ReadOneByte(AddAirPumpFre);
		GD_BUF[AIRPUMP_DUTY]           = AT24CXX_ReadOneByte(AddAipPumpDuty);

		GD_BUF[IR_TIME]                = AT24CXX_ReadOneByte(AddIrLaserTimeB1)|(AT24CXX_ReadOneByte(AddIrLaserTimeB2)<<8)|(AT24CXX_ReadOneByte(AddIrLaserTimeB3)<<16);
		GD_BUF[UV_TIME]                = AT24CXX_ReadOneByte(AddUvLaserTimeB1)|(AT24CXX_ReadOneByte(AddUvLaserTimeB2)<<8)|(AT24CXX_ReadOneByte(AddUvLaserTimeB3)<<16); 
	
		GD_BUF[IR_PD_FACTOR]           = AT24CXX_ReadOneByte(AddIrPdFactor);
		GD_BUF[UV_PD_FACTOR]           = AT24CXX_ReadOneByte(AddUvPdFactor);
        GD_BUF[ELECTRIC_PD_TRI]        = AT24CXX_ReadOneByte(AddElectricPdTriB1)|(AT24CXX_ReadOneByte(AddElectricPdTriB2)<<8);
        GD_BUF[LASER_OUT_PD_TRI]       = AT24CXX_ReadOneByte(AddLaserPdTriB1)|(AT24CXX_ReadOneByte(AddLaserPdTriB2)<<8);
		GD_BUF[SECLASER_OUT_PD_TRI]    = AT24CXX_ReadOneByte(AddSecLaserPdTriB1)|(AT24CXX_ReadOneByte(AddSecLaserPdTriB2)<<8);
        GD_BUF[IR_A_MAX]               = AT24CXX_ReadOneByte(AddIrAdcMaxB1)|(AT24CXX_ReadOneByte(AddIrAdcMaxB2)<<8);
		GD_BUF[IR_A_MIN]               = AT24CXX_ReadOneByte(AddIrAdcMinB1)|(AT24CXX_ReadOneByte(AddIrAdcMinB2)<<8);
		GD_BUF[IR_P_MAX]               = AT24CXX_ReadOneByte(AddIrPowerMaxB1)|(AT24CXX_ReadOneByte(AddIrPowerMaxB2)<<8);
		GD_BUF[IR_P_MIN]               = AT24CXX_ReadOneByte(AddIrPowerMinB1)|(AT24CXX_ReadOneByte(AddIrPowerMinB2)<<8);
		GD_BUF[UV_A_MAX]               = AT24CXX_ReadOneByte(AddUvAdcMaxB1)|(AT24CXX_ReadOneByte(AddUvAdcMaxB2)<<8);
		GD_BUF[UV_A_MIN]               = AT24CXX_ReadOneByte(AddUvAdcMinB1)|(AT24CXX_ReadOneByte(AddUvAdcMinB2)<<8);
		GD_BUF[UV_P_MAX]               = AT24CXX_ReadOneByte(AddUvPowerMaxB1)|(AT24CXX_ReadOneByte(AddUvPowerMaxB2)<<8);
		GD_BUF[UV_P_MIN]               = AT24CXX_ReadOneByte(AddUvPowerMinB1)|(AT24CXX_ReadOneByte(AddUvPowerMinB2)<<8);
	   
		GD_BUF[INIT_TIME]              = AT24CXX_ReadOneByte(AddInitTime);
		GD_BUF[INIT_ALARM_CNT]         = AT24CXX_ReadOneByte(AddInitAlarmCnt);
		GD_BUF[SEED_ALARM_CNT]         = AT24CXX_ReadOneByte(AddSeedAlarmCnt);         
		GD_BUF[SQ_ALARM_CNT]           = AT24CXX_ReadOneByte(AddSqAlarmCnt);
		GD_BUF[PUMP1_H_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump1HAlarmCnt); 
		GD_BUF[PUMP1_L_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump1LAlarmCnt);
		GD_BUF[PUMP2_H_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump2HAlarmCnt);
		GD_BUF[PUMP2_L_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump2LAlarmCnt);
		GD_BUF[PUMP3_H_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump3HAlarmCnt);
		GD_BUF[PUMP3_L_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump3LAlarmCnt);
		GD_BUF[PUMP4_H_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump4HAlarmCnt);
		GD_BUF[PUMP4_L_ALARM_CNT]      = AT24CXX_ReadOneByte(AddPump4LAlarmCnt);
		GD_BUF[CAVITY_H_ALARM_CNT]     = AT24CXX_ReadOneByte(AddCavityHAlarmCnt);
		GD_BUF[CAVITY_L_ALARM_CNT]     = AT24CXX_ReadOneByte(AddCavityLAlarmCnt);
		GD_BUF[CAVITY2_H_ALARM_CNT]    = AT24CXX_ReadOneByte(AddCavity2HAlarmCnt);
		GD_BUF[CAVITY2_L_ALARM_CNT]    = AT24CXX_ReadOneByte(AddCavity2LAlarmCnt);
        GD_BUF[ELECTRIC_PD_TRI_CNT]    = AT24CXX_ReadOneByte(AddElectricPdTriCnt);
        GD_BUF[LASER_OUT_PD_TRI_CNT]   = AT24CXX_ReadOneByte(AddLaserPdPdTriCnt);
        GD_BUF[SECLASER_OUT_PD_TRI_CNT]= AT24CXX_ReadOneByte(AddSecLaserPdTriCnt);
        
//		GD_BUF[SerialNumA1]            = AT24CXX_ReadOneByte(73)|(AT24CXX_ReadOneByte(74)<<8)|(AT24CXX_ReadOneByte(75)<<16);	
//		GD_BUF[SerialNumB1]            = AT24CXX_ReadOneByte(76)|(AT24CXX_ReadOneByte(77)<<8)|(AT24CXX_ReadOneByte(78)<<16);
//#if		USER_LOCK
//		GD_BUF[RTC_DATE_SET]           = AT24CXX_ReadOneByte(79)|(AT24CXX_ReadOneByte(80)<<8)|(AT24CXX_ReadOneByte(81)<<16);	
//		GD_BUF[RTC_TIME_SET]           = AT24CXX_ReadOneByte(82)|(AT24CXX_ReadOneByte(83)<<8)|(AT24CXX_ReadOneByte(84)<<16);
//		GD_BUF[USER_LIMITDAY]          = AT24CXX_ReadOneByte(85);
//#endif
	}
	else   //进行初始数据更新
	{	
		GD_BUF[SN1]                    = 0x00000000;                         //激光器SN号
		GD_BUF[SN2]                    = 0x00000000;                         //激光器SN号
		GD_BUF[SN3]                    = 0x00000001;                         //激光器SN号
		GD_BUF[PUMP1_ENA]              = 0;
		GD_BUF[PUMP2_ENA]              = 0;
		GD_BUF[PUMP3_ENA]              = 0;
		GD_BUF[PUMP4_ENA]              = 0;
		GD_BUF[PUMP1_I_SET]            = 0;
		GD_BUF[PUMP2_I_SET]            = 0;
		GD_BUF[PUMP3_I_SET]            = 0;
		GD_BUF[PUMP4_I_SET]            = 0;
		GD_BUF[PUMP1_I_MAX]            = 160;
		GD_BUF[PUMP2_I_MAX]            = 160;
		GD_BUF[PUMP3_I_MAX]            = 160;
		GD_BUF[PUMP4_I_MAX]            = 160;
		GD_BUF[PUMP_TEMP_H_ALARM]      = 350;
		GD_BUF[PUMP_TEMP_L_ALARM]      = 150;
		GD_BUF[CAVITY_TEMP_H_ALARM]    = 350;
		GD_BUF[CAVITY_TEMP_L_ALARM]    = 150;
		GD_BUF[SEED_POWER_ENA]         = 0;
		GD_BUF[SEED_LASER_ENA]         = 0;
		GD_BUF[SEED_LASER_ENERGY_CTRL] = 100;
		GD_BUF[SEED_TIME]              = 0; 
		GD_BUF[SEED_TYPE]              = 1;
		
		GD_BUF[FQ_POWER_ENA]           = 0;
		GD_BUF[BURST_NUM]              = 1;
		GD_BUF[FQ_RF_SCALE]            = 100;
		GD_BUF[FQ_FRE]                 = 1000;
		GD_BUF[FQ_DELAY]               = 20;
		 
		GD_BUF[SQ_POWER_ENA]           = 0;
		GD_BUF[SQ_STANDBY_ENA]         = 1; 
		GD_BUF[SQ_RF_SCALE]            = 1000;
		GD_BUF[SQ_ATTEN_SEL]           = 0;
		/*heart revise in 2022.7.28  600-->350*/
		GD_BUF[SQ_ADD_TIME]            = 350;  //unit:ns
		GD_BUF[SQ_TYPE]                = 1;
		GD_BUF[LASER_TYPE]             = 1;
		GD_BUF[LASER_POWSEL]	       = 1;		
			
		GD_BUF[LASER_MODE]             = 0;
		GD_BUF[MODE_SW]                = 0;
		GD_BUF[EXT_MODE]               = 0;
		GD_BUF[MODULO_DIVIDER]         = 1;
		GD_BUF[BURST_HOLDOFF]          = 0;
		GD_BUF[BURST_LENGTH]           = 1; 
		GD_BUF[EXT_MODE_ACTLEVEL]      = 1;
		GD_BUF[SWITCH]                 = 0;
		
		GD_BUF[SHG_SET]                = 3000;
		GD_BUF[THG_SET]                = 3000;

		GD_BUF[AIRPUMP_EN]             = 1;
		GD_BUF[AIRPUMP_FREQ] 		   = 20;                                 //气泵频率
		GD_BUF[AIRPUMP_DUTY]  		   = 66;                                 //气泵占空比		

		GD_BUF[IR_TIME]                = 0;                                  //LD使用计时
		GD_BUF[UV_TIME]                = 0;

		GD_BUF[IR_PD_FACTOR]           = 100;
		GD_BUF[UV_PD_FACTOR]           = 100;
        GD_BUF[ELECTRIC_PD_TRI]        = 3500;
        GD_BUF[LASER_OUT_PD_TRI]       = 3;
        GD_BUF[SECLASER_OUT_PD_TRI]    = 3;
		GD_BUF[IR_A_MAX]               = 0;
		GD_BUF[IR_A_MIN]               = 0;
		GD_BUF[IR_P_MAX]               = 0;
		GD_BUF[IR_P_MIN]               = 0;
		GD_BUF[UV_A_MAX]               = 0;
		GD_BUF[UV_A_MIN]               = 0;
		GD_BUF[UV_P_MAX]               = 0;
		GD_BUF[UV_P_MIN]               = 0;
	
		GD_BUF[INIT_TIME]              = 2;                                  //默认初始化时长1分钟
		GD_BUF[INIT_ALARM_CNT]         = 0;
		GD_BUF[SEED_ALARM_CNT]         = 0;         
		GD_BUF[SQ_ALARM_CNT]           = 0;
		GD_BUF[PUMP1_H_ALARM_CNT]      = 0; 
		GD_BUF[PUMP1_L_ALARM_CNT]      = 0;
		GD_BUF[PUMP2_H_ALARM_CNT]      = 0;
		GD_BUF[PUMP2_L_ALARM_CNT]      = 0;
		GD_BUF[PUMP3_H_ALARM_CNT]      = 0;
		GD_BUF[PUMP3_L_ALARM_CNT]      = 0;
		GD_BUF[PUMP4_H_ALARM_CNT]      = 0;
		GD_BUF[PUMP4_L_ALARM_CNT]      = 0;
		GD_BUF[CAVITY_H_ALARM_CNT]     = 0;
		GD_BUF[CAVITY_L_ALARM_CNT]     = 0;
		GD_BUF[CAVITY2_H_ALARM_CNT]    = 0;
		GD_BUF[CAVITY2_L_ALARM_CNT]    = 0;
		GD_BUF[ELECTRIC_PD_TRI_CNT]    = 0;
        GD_BUF[LASER_OUT_PD_TRI_CNT]   = 0;
        GD_BUF[SECLASER_OUT_PD_TRI_CNT]= 0;
//		GD_BUF[SerialNumA1]            = Product_Password();                 //生成锁机密码
//		GD_BUF[SerialNumB1]            = GD_BUF[SerialNumA1];                //解锁密码		

		AT24CXX_WriteOneByte(AddEepromCheck,(EEPROMPASS));                     //EEPROM读写标志

		AT24CXX_WriteOneByte(AddSn1Byte1, GD_BUF[SN1] & 0xff);
		AT24CXX_WriteOneByte(AddSn1Byte2, (GD_BUF[SN1] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddSn1Byte3, (GD_BUF[SN1] >> 16) & 0xff);
		AT24CXX_WriteOneByte(AddSn1Byte4, (GD_BUF[SN1] >> 24) & 0xff);	
	    
		AT24CXX_WriteOneByte(AddSn2Byte1, GD_BUF[SN2] & 0xff);
	    AT24CXX_WriteOneByte(AddSn2Byte2, (GD_BUF[SN2] >> 8) & 0xff);
	    AT24CXX_WriteOneByte(AddSn2Byte3, (GD_BUF[SN2] >> 16) & 0xff);
	    AT24CXX_WriteOneByte(AddSn2Byte4, (GD_BUF[SN2] >> 24) & 0xff);
		
		AT24CXX_WriteOneByte(AddSn3Byte1, GD_BUF[SN3] & 0xff);
	    AT24CXX_WriteOneByte(AddSn3Byte2, (GD_BUF[SN3] >> 8) & 0xff);
	    AT24CXX_WriteOneByte(AddSn3Byte3,(GD_BUF[SN3] >> 16) & 0xff);
	    AT24CXX_WriteOneByte(AddSn3Byte4,(GD_BUF[SN3] >> 24) & 0xff);
		
//		AT24CXX_WriteOneByte(AddPump1Ena, GD_BUF[PUMP1_ENA]);
//		AT24CXX_WriteOneByte(AddPump2Ena, GD_BUF[PUMP2_ENA]);
//		AT24CXX_WriteOneByte(AddPump3Ena, GD_BUF[PUMP3_ENA]);
//		AT24CXX_WriteOneByte(AddPump4Ena, GD_BUF[PUMP4_ENA]);
		
		AT24CXX_WriteOneByte(AddPump1ISetByte1, GD_BUF[PUMP1_I_SET] & 0xff);
		AT24CXX_WriteOneByte(AddPump1ISetByte2, (GD_BUF[PUMP1_I_SET] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddPump2ISetByte1, GD_BUF[PUMP2_I_SET] & 0xff);
		AT24CXX_WriteOneByte(AddPump2ISetByte2, (GD_BUF[PUMP2_I_SET]>> 8 ) & 0xff);
		AT24CXX_WriteOneByte(AddPump3ISetByte1, GD_BUF[PUMP3_I_SET] & 0xff);
		AT24CXX_WriteOneByte(AddPump3ISetByte2, (GD_BUF[PUMP3_I_SET] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddPump4ISetByte1, GD_BUF[PUMP4_I_SET] & 0xff);
		AT24CXX_WriteOneByte(AddPump4ISetByte2, (GD_BUF[PUMP4_I_SET] >> 8) & 0xff);
		
		AT24CXX_WriteOneByte(AddPump1IMaxByte1, GD_BUF[PUMP1_I_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddPump1IMaxByte2, (GD_BUF[PUMP1_I_MAX] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddPump2IMaxByte1, GD_BUF[PUMP2_I_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddPump2IMaxByte2, (GD_BUF[PUMP2_I_MAX] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddPump3IMaxByte1, GD_BUF[PUMP3_I_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddPump3IMaxByte2, (GD_BUF[PUMP3_I_MAX] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddPump4IMaxByte1, GD_BUF[PUMP4_I_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddPump4IMaxByte2, (GD_BUF[PUMP4_I_MAX] >> 8) & 0xff);
		
		AT24CXX_WriteOneByte(AddPumpTempHAlarmB1, GD_BUF[PUMP_TEMP_H_ALARM] & 0xff);
		AT24CXX_WriteOneByte(AddPumpTempHAlarmB2, (GD_BUF[PUMP_TEMP_H_ALARM] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddPumpTempLAlarmB1, GD_BUF[PUMP_TEMP_L_ALARM] & 0xff);
		AT24CXX_WriteOneByte(AddPumpTempLAlarmB2, (GD_BUF[PUMP_TEMP_L_ALARM] >> 8) & 0xff);	
		
		AT24CXX_WriteOneByte(AddCavityTempHAlarmB1, GD_BUF[CAVITY_TEMP_H_ALARM] & 0xff);
		AT24CXX_WriteOneByte(AddCavityTempHAlarmB2, (GD_BUF[CAVITY_TEMP_H_ALARM] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddCavityTempLAlarmB1, GD_BUF[CAVITY_TEMP_L_ALARM] & 0xff);
		AT24CXX_WriteOneByte(AddCavityTempLAlarmB2, (GD_BUF[CAVITY_TEMP_L_ALARM] >> 8) & 0xff);
		
//		AT24CXX_WriteOneByte(AddSeedPower, GD_BUF[SEED_POWER_ENA]);
//		AT24CXX_WriteOneByte(AddSeedLaser, GD_BUF[SEED_LASER_ENA]);
		AT24CXX_WriteOneByte(AddSeedEnergy, GD_BUF[SEED_LASER_ENERGY_CTRL]);
		AT24CXX_WriteOneByte(AddSeedLaserTimeB1, GD_BUF[SEED_TIME] & 0xff);
	    AT24CXX_WriteOneByte(AddSeedLaserTimeB2, (GD_BUF[SEED_TIME] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddSeedLaserTimeB3, (GD_BUF[SEED_TIME] >> 16) & 0xff);
		AT24CXX_WriteOneByte(AddSeedType, GD_BUF[SEED_TYPE]);
		
//	    AT24CXX_WriteOneByte(AddFqPower, GD_BUF[FQ_POWER_ENA]); 
		AT24CXX_WriteOneByte(AddFqBurst, GD_BUF[BURST_NUM]);
		AT24CXX_WriteOneByte(AddFqRf, GD_BUF[FQ_RF_SCALE]);
		AT24CXX_WriteOneByte(AddFqFreB1, GD_BUF[FQ_FRE] & 0xff);
		AT24CXX_WriteOneByte(AddFqFreB2, (GD_BUF[FQ_FRE] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddFqDelay, GD_BUF[FQ_DELAY]);
		
//		AT24CXX_WriteOneByte(AddSqPower, GD_BUF[SQ_POWER_ENA]);
//		AT24CXX_WriteOneByte(AddSqStandby, GD_BUF[SQ_STANDBY_ENA]);
		AT24CXX_WriteOneByte(AddSqRfB1, GD_BUF[SQ_RF_SCALE] & 0xff);
		AT24CXX_WriteOneByte(AddSqRfB2, (GD_BUF[SQ_RF_SCALE] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddSqAttenSel, GD_BUF[SQ_ATTEN_SEL]);
		AT24CXX_WriteOneByte(AddSqAddTimeB1, GD_BUF[SQ_ADD_TIME] & 0xff);
		AT24CXX_WriteOneByte(AddSqAddTimeB2, (GD_BUF[SQ_ADD_TIME] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddSqType, GD_BUF[SQ_TYPE]);
        
        AT24CXX_WriteOneByte(AddLaserType,GD_BUF[LASER_TYPE]);
        AT24CXX_WriteOneByte(AddPowerSel,GD_BUF[LASER_POWSEL]); 
      
		AT24CXX_WriteOneByte(AddLaserMode, GD_BUF[LASER_MODE]);
		AT24CXX_WriteOneByte(AddModeSw, GD_BUF[MODE_SW]);
	    AT24CXX_WriteOneByte(AddExtMode, GD_BUF[EXT_MODE]);
		AT24CXX_WriteOneByte(AddModuloDividerB1, GD_BUF[MODULO_DIVIDER] & 0xff);
		AT24CXX_WriteOneByte(AddModuloDividerB2, (GD_BUF[MODULO_DIVIDER] >> 8)  & 0xff);
		AT24CXX_WriteOneByte(AddBurstHoldOff, GD_BUF[BURST_HOLDOFF]);
		AT24CXX_WriteOneByte(AddBurstLengthB1, GD_BUF[BURST_LENGTH] & 0xff);
		AT24CXX_WriteOneByte(AddBurstLengthB2, (GD_BUF[BURST_LENGTH] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddExtModeLevel, GD_BUF[EXT_MODE_ACTLEVEL]);
		AT24CXX_WriteOneByte(AddSwitch, GD_BUF[SWITCH]&0xff);
		
		AT24CXX_WriteOneByte(AddShgTempSetB1, GD_BUF[SHG_SET] & 0xff);
		AT24CXX_WriteOneByte(AddShgTempSetB2, (GD_BUF[SHG_SET] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddThgTempSetB1, GD_BUF[THG_SET] & 0xff);
		AT24CXX_WriteOneByte(AddThgTempSetB2, (GD_BUF[THG_SET] >> 8) & 0xff);
		
		AT24CXX_WriteOneByte(AddAirPumpEna, GD_BUF[AIRPUMP_EN]);
		AT24CXX_WriteOneByte(AddAirPumpFre, GD_BUF[AIRPUMP_FREQ]);
		AT24CXX_WriteOneByte(AddAipPumpDuty, GD_BUF[AIRPUMP_DUTY]);
		
		AT24CXX_WriteOneByte(AddIrLaserTimeB1, GD_BUF[IR_TIME] & 0xff); //2^24 * 0.1H / 24 = 69905 天
		AT24CXX_WriteOneByte(AddIrLaserTimeB2,(GD_BUF[IR_TIME] >> 8) & 0xff);
	    AT24CXX_WriteOneByte(AddIrLaserTimeB3,(GD_BUF[IR_TIME] >> 16) & 0xff);
		AT24CXX_WriteOneByte(AddUvLaserTimeB1, GD_BUF[UV_TIME] & 0xff);
		AT24CXX_WriteOneByte(AddUvLaserTimeB2,(GD_BUF[UV_TIME] >> 8) & 0xff);
	    AT24CXX_WriteOneByte(AddUvLaserTimeB3,(GD_BUF[UV_TIME] >> 16) & 0xff);
		
		
//		AT24CXX_WriteOneByte(73, GD_BUF[SerialNumA1] & 0xff);
//		AT24CXX_WriteOneByte(74,(GD_BUF[SerialNumA1] >> 8) & 0xff);
//		AT24CXX_WriteOneByte(75,(GD_BUF[SerialNumA1] >> 16) & 0xff);
//	    AT24CXX_WriteOneByte(76, GD_BUF[SerialNumB1] & 0xff);
//		AT24CXX_WriteOneByte(77,(GD_BUF[SerialNumB1] >> 8) & 0xff);
//		AT24CXX_WriteOneByte(78,(GD_BUF[SerialNumB1] >> 16) & 0xff);
			
		AT24CXX_WriteOneByte(AddIrPdFactor, GD_BUF[IR_PD_FACTOR]);
		AT24CXX_WriteOneByte(AddUvPdFactor, GD_BUF[UV_PD_FACTOR]);

		AT24CXX_WriteOneByte(AddElectricPdTriB1, GD_BUF[ELECTRIC_PD_TRI] & 0xff);
		AT24CXX_WriteOneByte(AddElectricPdTriB2, (GD_BUF[ELECTRIC_PD_TRI] >> 8) & 0xff);
        AT24CXX_WriteOneByte(AddLaserPdTriB1, GD_BUF[LASER_OUT_PD_TRI] & 0xff);
		AT24CXX_WriteOneByte(AddLaserPdTriB2, (GD_BUF[LASER_OUT_PD_TRI] >> 8) & 0xff);
        AT24CXX_WriteOneByte(AddSecLaserPdTriB1, GD_BUF[SECLASER_OUT_PD_TRI] & 0xff);
		AT24CXX_WriteOneByte(AddSecLaserPdTriB2, (GD_BUF[SECLASER_OUT_PD_TRI] >> 8) & 0xff);

		AT24CXX_WriteOneByte(AddIrAdcMaxB1, GD_BUF[IR_A_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddIrAdcMaxB2, (GD_BUF[IR_A_MAX] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddIrAdcMinB1, GD_BUF[IR_A_MIN] & 0xff);
		AT24CXX_WriteOneByte(AddIrAdcMinB2, (GD_BUF[IR_A_MIN] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddIrPowerMaxB1, GD_BUF[IR_P_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddIrPowerMaxB2, (GD_BUF[IR_P_MAX] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddIrPowerMinB1, GD_BUF[IR_P_MIN] & 0xff);
		AT24CXX_WriteOneByte(AddIrPowerMinB2, (GD_BUF[IR_P_MIN] >>8) & 0xff);
		AT24CXX_WriteOneByte(AddUvAdcMaxB1, GD_BUF[UV_A_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddUvAdcMaxB2, (GD_BUF[UV_A_MAX] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddUvAdcMinB1, GD_BUF[UV_A_MIN] & 0xff);
		AT24CXX_WriteOneByte(AddUvAdcMinB2, (GD_BUF[UV_A_MIN] >> 8) & 0xff);
		AT24CXX_WriteOneByte(AddUvPowerMaxB1, GD_BUF[UV_P_MAX] & 0xff);
		AT24CXX_WriteOneByte(AddUvPowerMaxB2, (GD_BUF[UV_P_MAX]>>8) & 0xff); 
		AT24CXX_WriteOneByte(AddUvPowerMinB1, GD_BUF[UV_P_MIN] & 0xff);
		AT24CXX_WriteOneByte(AddUvPowerMinB2, (GD_BUF[UV_P_MIN] >>8) & 0xff);
		
     	AT24CXX_WriteOneByte(AddInitTime, GD_BUF[INIT_TIME]);
		AT24CXX_WriteOneByte(AddInitAlarmCnt, GD_BUF[INIT_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddSeedAlarmCnt, GD_BUF[SEED_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddSqAlarmCnt, GD_BUF[SQ_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump1HAlarmCnt, GD_BUF[PUMP1_H_ALARM_CNT]); //上限255次
		AT24CXX_WriteOneByte(AddPump1LAlarmCnt, GD_BUF[PUMP1_L_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump2HAlarmCnt, GD_BUF[PUMP2_H_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump2LAlarmCnt, GD_BUF[PUMP2_L_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump3HAlarmCnt, GD_BUF[PUMP3_H_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump3LAlarmCnt, GD_BUF[PUMP3_L_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump4HAlarmCnt, GD_BUF[PUMP4_H_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddPump4LAlarmCnt, GD_BUF[PUMP4_L_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddCavityHAlarmCnt, GD_BUF[CAVITY_H_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddCavityLAlarmCnt, GD_BUF[CAVITY_L_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddCavity2HAlarmCnt, GD_BUF[CAVITY2_H_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddCavity2LAlarmCnt, GD_BUF[CAVITY2_L_ALARM_CNT]);
		AT24CXX_WriteOneByte(AddCavity2LAlarmCnt, GD_BUF[CAVITY2_L_ALARM_CNT]);
        AT24CXX_WriteOneByte(AddElectricPdTriCnt, GD_BUF[ELECTRIC_PD_TRI_CNT]);
        AT24CXX_WriteOneByte(AddLaserPdPdTriCnt, GD_BUF[LASER_OUT_PD_TRI_CNT]);
        AT24CXX_WriteOneByte(AddSecLaserPdTriCnt, GD_BUF[SECLASER_OUT_PD_TRI_CNT]);
        
#if		USER_LOCK		
		GD_BUF[RTC_DATE_SET]           = 200701;                             //YYMMDD
		GD_BUF[RTC_TIME_SET]           = 131415;                             //HHMMSS
		GD_BUF[USER_LIMITDAY]          = 255;                                //试用天数	
//		AT24CXX_WriteOneByte(79,GD_BUF[RTC_DATE_SET] & 0xff);                    
//		AT24CXX_WriteOneByte(80,(GD_BUF[RTC_DATE_SET] >> 8) & 0xff);
//		AT24CXX_WriteOneByte(81,(GD_BUF[RTC_DATE_SET] >> 16) & 0xff);
//		AT24CXX_WriteOneByte(82,GD_BUF[RTC_TIME_SET] & 0xff);		
//		AT24CXX_WriteOneByte(83,(GD_BUF[RTC_TIME_SET] >> 8) & 0xff);	
//		AT24CXX_WriteOneByte(84,(GD_BUF[RTC_TIME_SET]  >> 16) & 0xff);	
//		AT24CXX_WriteOneByte(85,GD_BUF[USER_LIMITDAY]);	
#endif		
	}
    if(GD_BUF[AIRPUMP_EN] == 1)
    {
        FAN_EN_ON();
    }
    else  
    {
        FAN_EN_OFF(); 
    }
} 

void All_Init(void)
{     
    get_eeprom_data();              //获取EEPROM存储数据 
    rt_thread_mdelay(1000);                 
    Seed_Init();                    //种子源初始化
    Aom_Init();                     //AOM初始化
    PUMP_Init();                    //泵浦初始化
//    HM_Init();                      //AHT20温湿度计初始化
    STHG_Init();                    //二/三倍频初始化     
    DAC_CBM128S085_Init();
    
    ACTIVE_SEED_ENERGY_CTRL(GD_BUF[SEED_LASER_ENERGY_CTRL],Channel_E);  //设置种子源功率
    Aom1RfSet(GD_BUF[FQ_RF_SCALE],Channel_F);                           //设置Fiber Q射频功率 
    STHG_TEMP_SET(GD_BUF[SHG_SET],Channel_G);                           //设置二倍频温度
    STHG_TEMP_SET(GD_BUF[THG_SET],Channel_H);                           //设置三倍频温度
    SHG_ENA_ON();                 									    //使能二倍频
    THG_ENA_ON(); 		         									    //使能三倍频
    
    ACTIVE_WR_FPGA();           									    //向FPGA写入数据
    
    GD_BUF[USER_STATE]        = 0; 
    GD_BUF[ERR]               = 0;
    GD_BUF[PUMP1_TEMP_READ]   = 250;
    GD_BUF[PUMP2_TEMP_READ]   = 250;
    GD_BUF[PUMP3_TEMP_READ]   = 250; 
    GD_BUF[PUMP4_TEMP_READ]   = 250;
    GD_BUF[CAVITY_TEMP_READ]  = 250;
    GD_BUF[CAVITY2_TEMP_READ] = 250;      
}

static void Initail_Check_Task(void* parameter)
{
 	 rt_err_t err;

	static rt_uint32_t init_cnt = 0;  //初始化过程计时
    static rt_uint8_t checkItemCnt[8]={0,0,0,0,0,0,0,0}; //记录每项的检查次数
    bool checkItemResult[8]={false,false,false,false,false,false,false,false}; //保存初始化时检查项的结果
	
	//上电初始化检查项
	typedef enum
    {
		seedItem       = 0,
		sqItem         = 1,
		pump1Item      = 2,
		pump2Item      = 3,
		pump3Item      = 4,
		pump4Item      = 5,
		cavityItem     = 6,
		cavity2Item    = 7
	}checkItem;


    while(1)
    {
        rt_thread_mdelay(100);                                          //延时100ms
		//可硬件屏蔽初始化检查
        if(!ALARM_EN)                                                   
        {
            if( GD_BUF[ERR] == none)                                     //无告警
            {
				if(GD_BUF[SEED_TYPE] == TOPTICA)
				{
					//种子源检查
					if( SEED_ALARM != 0)      
					{
						checkItemCnt[seedItem]++;
						if(checkItemCnt[seedItem] > scanNum)                 //检查scanNum遍无异常
						{
							checkItemResult[seedItem] = true;
						}
					}
					else
					{
						checkItemResult[seedItem] = false;
						checkItemCnt[seedItem] = 0;
					}
				}
				else if(GD_BUF[SEED_TYPE] == NPI)
				{
					checkItemResult[seedItem] = true;
				} 

				//空间Q检查
				if(SQ_SWR_ALARM != 0)       
				{
					checkItemCnt[sqItem]++;
					if(checkItemCnt[sqItem] > scanNum)
					{
						checkItemResult[sqItem] = true;
					}
				}
				else
				{
					checkItemResult[sqItem] = false;
					checkItemCnt[sqItem] = 0;
				}
				
                //泵1温度检查
                if(IsInSide(GD_BUF[PUMP1_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM]+10, GD_BUF[PUMP_TEMP_H_ALARM]-10))//比设定的上下限报警范围小1℃
                {
                   	checkItemCnt[pump1Item]++;
					if(checkItemCnt[pump1Item] > scanNum)
					{
						checkItemResult[pump1Item] = true;
					}
                }
                else
                {
                   	checkItemResult[pump1Item] = false;
					checkItemCnt[pump1Item] = 0;
                }
				
				//泵2温度检查
                if(IsInSide(GD_BUF[PUMP2_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM]+10, GD_BUF[PUMP_TEMP_H_ALARM]-10))
                {
                   	checkItemCnt[pump2Item]++;
					if(checkItemCnt[pump2Item] > scanNum)
					{
						checkItemResult[pump2Item] = true;
					}
                }
                else
                {
                   	checkItemResult[pump2Item] = false;
					checkItemCnt[pump2Item] = 0;
                }
				
//				//泵3温度检查
//                if(IsInSide(GD_BUF[PUMP3_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM]+10, GD_BUF[PUMP_TEMP_H_ALARM]-10))
//                {
//                   	checkItemCnt[pump3Item]++;
//					if(checkItemCnt[pump3Item] > scanNum)
//					{
//						checkItemResult[pump3Item] = true;
//					}
//                }
//                else
//                {
//                   	checkItemResult[pump3Item] = false;
//					checkItemCnt[pump3Item] = 0;
//                }
				
				//腔体1处温度检查
                if(IsInSide(GD_BUF[CAVITY_TEMP_READ], GD_BUF[CAVITY_TEMP_L_ALARM]+10, GD_BUF[CAVITY_TEMP_H_ALARM]-10))
                {
                   	checkItemCnt[cavityItem]++;
					if(checkItemCnt[cavityItem] > scanNum)
					{
						checkItemResult[cavityItem] = true;
					}
                }
                else
                {
                   	checkItemResult[cavityItem] = false;
					checkItemCnt[cavityItem] = 0;
                }	
				
				//腔体2处温度检查
                if(IsInSide(GD_BUF[CAVITY2_TEMP_READ], GD_BUF[CAVITY_TEMP_L_ALARM]+10, GD_BUF[CAVITY_TEMP_H_ALARM]-10))
                {
                   	checkItemCnt[cavity2Item]++;
						if(checkItemCnt[cavity2Item] > scanNum)
						{
							checkItemResult[cavity2Item] = true;
						}
                }
                else
                {
                   	checkItemResult[cavity2Item] = false;
						checkItemCnt[cavity2Item] = 0;
                }	
				
                //需要初始化检测
                if(GD_BUF[INIT_TIME] != 0) 
                {
                    if(init_cnt++ > GD_BUF[INIT_TIME] * 300) //超过最大初始化时间，报警,以半分钟为步进
                    {
                        GD_BUF[ERR_DETAIL] = initailBit;   
						
                        GD_BUF[ERR] = initialWarning ;
						
                        if(GD_BUF[INIT_ALARM_CNT] < 255) 
                        { 
                            GD_BUF[INIT_ALARM_CNT]++;
                            AT24CXX_WriteOneByte(AddInitAlarmCnt, GD_BUF[INIT_ALARM_CNT]); //记录报警次数
                        }
                       return;	         //删除start_task任务自身
                    }
                    GD_BUF[INIT_TIME_CNT] = init_cnt / 10;   //1S累加1次
                }
                //无需初始化检测等待，直接进入主函数
                else
                {
                    rt_thread_mdelay(2000);             //延时2s

                    GD_BUF[STARTING] = true;

                    return;	//删除start_task任务自身
                }
            }
        }
        //检查初始化情况，可屏蔽
//		if( ( (checkItemResult[seedItem] && checkItemResult[sqItem] && checkItemResult[pump1Item] && checkItemResult[pump2Item] && checkItemResult[pump3Item]\
//			&& checkItemResult[cavityItem] && checkItemResult[cavity2Item] ) && (GD_BUF[ERR] == none) ) || ALARM_EN )
		if( ( (checkItemResult[seedItem] && checkItemResult[sqItem] && checkItemResult[pump1Item] && checkItemResult[pump2Item]\
			&& checkItemResult[cavityItem] ) && (GD_BUF[ERR] == none) ) || ALARM_EN )
        {
            GD_BUF[STARTING] = true;
			
            return;	//删除start_task任务自身
        }
    }
}

/********************************************************************************
*函数名: Monitor
*参数  : 无
*功能  : 监测LD,光学腔温度,置位告警标志
*返回值: 无
*GD_BUF[ERR]
*1-Seed error;2-Space Q error;3-Pump1 T error;4-Pump2 T error;5-Pump3 T error;6-Pump4 T error;7-Caviyt error
*********************************************************************************/
void Monitor(void)
{
	static rt_uint8_t cnt[8]={0,0,0,0,0,0,0,0};
	//监测种子源是否产生告警
	if(GD_BUF[SEED_TYPE] == TOPTICA)
	{
		if( (SEED_ALARM == 0) && (GD_BUF[ERR]== none) )  //低电平持续时间> 400ms 有效
		{	
			cnt[0]++;
			if(cnt[0] > scanNum)  //20*100ms=2s
			{
				cnt[0] = 0;
				GD_BUF[ERR] = seedWarning;
				GD_BUF[ERR_DETAIL] = seedBit;
				if(GD_BUF[SEED_ALARM_CNT] < 255)
				{
					GD_BUF[SEED_ALARM_CNT]++;
					AT24CXX_WriteOneByte(AddSeedAlarmCnt, GD_BUF[SEED_ALARM_CNT]);
				}
			}						
		}
		else
		{
			cnt[0] = 0;
		}
	}
	else if(GD_BUF[SEED_TYPE] == NPI)
	{
		if(ReadSeedLaserPinState)
		{
			if( (SEED_ALARM == 0) && (GD_BUF[ERR]== none) )  //低电平持续时间> 17s 有效
			{	
				cnt[0]++;
				if(cnt[0] > scanNum+150)  //170*100ms=17s
				{
					cnt[0] = 0;
					GD_BUF[ERR] = seedWarning;
					GD_BUF[ERR_DETAIL] = seedBit;
					if(GD_BUF[SEED_ALARM_CNT] < 255)
					{
						GD_BUF[SEED_ALARM_CNT]++;
						AT24CXX_WriteOneByte(AddSeedAlarmCnt, GD_BUF[SEED_ALARM_CNT]);
					}
				}						
			}
			else
			{
				cnt[0] = 0;
			}
		}
	}

	//监测空间Q是否产生告警
	if( (SQ_SWR_ALARM == 0) && (GD_BUF[ERR] == none) )  //只进入1次告警程序
	{	
		cnt[1]++;
		if(cnt[1] > scanNum)  //20*100ms=2s
		{
			cnt[1] = 0;
			GD_BUF[ERR] = sqWarning;
			GD_BUF[ERR_DETAIL] = sqBit;
			if(GD_BUF[SQ_ALARM_CNT] < 255)
			{
				GD_BUF[SQ_ALARM_CNT]++; 
				AT24CXX_WriteOneByte(AddSqAlarmCnt,GD_BUF[SQ_ALARM_CNT]);
			}
		}						
	}
	else
	{
		cnt[1] = 0;
	}
	//监测泵1温度是否超出范围
	if( IsOutSide( GD_BUF[PUMP1_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM], GD_BUF[PUMP_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none) )  //只进入1次告警程序
	{	
		cnt[2]++;
		if(cnt[2] > scanNum)   //20*100ms=2s
		{
			cnt[2] = 0;
			GD_BUF[ERR] = pump1Warning;	                                 //PUMP1温度报警
			if(IsInSide(GD_BUF[PUMP1_TEMP_READ], GD_BUF[PUMP_TEMP_H_ALARM], 1000 ))
            {
                GD_BUF[ERR_DETAIL] = pump1HBit;
                if(GD_BUF[PUMP1_H_ALARM_CNT] < 255)
                {
                    GD_BUF[PUMP1_H_ALARM_CNT]++;
                    AT24CXX_WriteOneByte(AddPump1HAlarmCnt, GD_BUF[PUMP1_H_ALARM_CNT]);
                }
            }
            else
            {
                GD_BUF[ERR_DETAIL] = pump1LBit;
                if(GD_BUF[PUMP1_L_ALARM_CNT] < 255)
                {
                    GD_BUF[PUMP1_L_ALARM_CNT]++;
                    AT24CXX_WriteOneByte(AddPump1LAlarmCnt, GD_BUF[PUMP1_L_ALARM_CNT]);
                }
            }
		}
	}
	else 
	{
		cnt[2] = 0;
	}
    //监测泵2温度是否超出范围

	if( IsOutSide( GD_BUF[PUMP2_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM], GD_BUF[PUMP_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none) )   //只进入1次告警程序
	{	
		cnt[3]++;
		if(cnt[3] > scanNum)   //20*100ms=2s
		{
			cnt[3] = 0;
			GD_BUF[ERR] = pump2Warning;	                                 //PUMP2温度报警
			if(IsInSide(GD_BUF[PUMP2_TEMP_READ], GD_BUF[PUMP_TEMP_H_ALARM], 1000 ))
            {
                GD_BUF[ERR_DETAIL] = pump2HBit;
                if(GD_BUF[PUMP2_H_ALARM_CNT] < 255)
                {
                    GD_BUF[PUMP2_H_ALARM_CNT]++;
                    AT24CXX_WriteOneByte(AddPump2HAlarmCnt, GD_BUF[PUMP2_H_ALARM_CNT]);
                }
            }
            else
            {
                GD_BUF[ERR_DETAIL] = pump2LBit;
                if(GD_BUF[PUMP2_L_ALARM_CNT] < 255)
                {
                    GD_BUF[PUMP2_L_ALARM_CNT]++;
                    AT24CXX_WriteOneByte(AddPump2LAlarmCnt, GD_BUF[PUMP2_L_ALARM_CNT]);
                }
            }
		}
	}
	else 
	{
		cnt[3] = 0;
	}
	
	//监测腔体温度是否超出范围
	if( IsOutSide( GD_BUF[CAVITY_TEMP_READ], GD_BUF[CAVITY_TEMP_L_ALARM], GD_BUF[CAVITY_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none)  )  //只进入1次告警程序	
	{	
		cnt[6]++;
		if(cnt[6] > scanNum)  //20*100ms=2s
		{
			cnt[6] = 0; 											
			GD_BUF[ERR] = cavityWarning;	                                 //CAVITY温度报警,关闭激光
			if(IsInSide(GD_BUF[CAVITY_TEMP_READ], GD_BUF[CAVITY_TEMP_H_ALARM], 1000 ))
            {
                GD_BUF[ERR_DETAIL] = cavityHBit;
                if(GD_BUF[CAVITY_H_ALARM_CNT] < 255)
                {
                    GD_BUF[CAVITY_H_ALARM_CNT]++;
					AT24CXX_WriteOneByte(AddCavityHAlarmCnt, GD_BUF[CAVITY_H_ALARM_CNT]);
                }
            }
            else
            {
                GD_BUF[ERR_DETAIL] = cavityLBit;
                if(GD_BUF[CAVITY_L_ALARM_CNT] < 255)
                {
                    GD_BUF[CAVITY_L_ALARM_CNT]++;
					AT24CXX_WriteOneByte(AddCavityLAlarmCnt, GD_BUF[CAVITY_L_ALARM_CNT]);
                }
            }
		}
	}
	else 
	{
		cnt[6] = 0;
	}
	
	//监测腔体2处温度是否超出范围
//	if( IsOutSide( GD_BUF[CAVITY2_TEMP_READ], GD_BUF[CAVITY_TEMP_L_ALARM], GD_BUF[CAVITY_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none)  )  //只进入1次告警程序	
//	{	
//		cnt[7]++;
//		if(cnt[7] > scanNum)  //20*100ms=2s
//		{
//			cnt[7] = 0; 											
//			GD_BUF[ERR] = cavity2Warning;	                                 //CAVITY2处温度报警,关闭激光
//		
//			if(IsInSide(GD_BUF[CAVITY2_TEMP_READ], GD_BUF[CAVITY_TEMP_H_ALARM], 1000 ))
//            {
//                GD_BUF[ERR_DETAIL] = cavity2HBit;
//                if(GD_BUF[CAVITY2_H_ALARM_CNT] < 255)
//                {
//                    GD_BUF[CAVITY2_H_ALARM_CNT]++;
//					AT24CXX_WriteOneByte(AddCavity2HAlarmCnt, GD_BUF[CAVITY2_H_ALARM_CNT]);
//                }
//            }
//            else
//            {
//                GD_BUF[ERR_DETAIL] = cavityLBit;
//                if(GD_BUF[CAVITY2_L_ALARM_CNT] < 255)
//                {
//                    GD_BUF[CAVITY2_L_ALARM_CNT]++;
//					AT24CXX_WriteOneByte(AddCavity2LAlarmCnt, GD_BUF[CAVITY2_L_ALARM_CNT]);
//                }
//            }
//		}
//	}
//	else 
//	{
//		cnt[7] = 0;
//	}
}

/********************************************************************************
*函数名: AlarmSet
*参数  : 无
*功能  : 执行告警动作
*返回值: 无
*********************************************************************************/
void AlarmSet(void)
{
	if((GD_BUF[ERR]&0xff) != none)
	{			
		GD_BUF[SEED_POWER_ENA] = off;
		GD_BUF[SEED_LASER_ENA] = off;
		GD_BUF[FQ_POWER_ENA]   = off;
		if(GD_BUF[PUMP1_I_READ] < 10)                            //泵关闭再关SQ
		{
			GD_BUF[SQ_POWER_ENA]   = off;
		}
		GD_BUF[SQ_STANDBY_ENA] = on; //待机	
		GD_BUF[PUMP1_ENA]      = off;
		GD_BUF[PUMP2_ENA]      = off;
		GD_BUF[PUMP3_ENA]      = off;
		
		ERR_LED_ON();		                                     //ERR灯亮
		rt_timer_start(timer_beep); 	                         //使能蜂鸣器控制定时器  			
	}	
}


/********************************************************************************
*函数名: Update_LaserTime_CNT
*参数  : 无
*功能  : 更新出光计时
*返回值: 无
*********************************************************************************/
void Update_LaserTime_CNT(void)
{
	if(USER_TIME_FLAG[0]) 
	{
		USER_TIME_FLAG[0] = 0;
		AT24CXX_Write(AddSeedLaserTimeB1, (uint8_t *)&GD_BUF[SEED_TIME], 3);
	}
	else if(USER_TIME_FLAG[1])                                                                                //每0.1H存储一次计数值
	{
		USER_TIME_FLAG[1] = 0;
        AT24CXX_Write(AddIrLaserTimeB1, (uint8_t *)&GD_BUF[IR_TIME], 3);
	}
	else if(USER_TIME_FLAG[2])
	{
		USER_TIME_FLAG[2] = 0;
		AT24CXX_Write(AddUvLaserTimeB1, (uint8_t *)&GD_BUF[UV_TIME], 3);
	}
}

/*fpga运算*/
void Fpga_Data_Calcultate(void)
{
		rt_uint16_t Calcultate[4]={0};//运算结果存放aom1CycleTotalCnt、aom1DutyTotalCnt、AOM1delay/5、AOM2delay/5
		rt_uint16_t Seedfretemp = 0;
		
		Seedfretemp = (FPGA_Read(SEED_FRE_R));//读取种子原频率 out_data <= seedFre / 12'd1000;
		if(Seedfretemp < 1)
		{
			Seedfretemp =1;
		}	
		GD_BUF[SEED_FRE] = Seedfretemp;
		
		/******AOM1fre*****/
		/*aom1CycleTotalCnt <= (seedFreTemp / aom1FreTemp / 12'd1000);以种子源时钟计算AOM1周期*/
		Calcultate[0] = Seedfretemp/GD_BUF[FQ_FRE];
		FPGA_Write(AOM1_CYCLE_TOTAL_CNT_W,Calcultate[0]);
		
		 /*aom1DutyTotalCnt  <= CLK_FS * aom1BurstNumTemp / seedFreTemp;以200M时钟计算aom1占空比*/
		 Calcultate[1] = CLK_FS*GD_BUF[BURST_NUM]/Seedfretemp/1000;
		 FPGA_Write(AOM1_DUTY_TOTAL_CNT_W,Calcultate[1]);
		
		 /*AOM1delay/5*/
		 Calcultate[2] = GD_BUF[FQ_DELAY]/5;
		 FPGA_Write(AOM1_DELAY_DIVIDE5_W,Calcultate[2]);	
		
		 /*AOM2delay/5*/
		 Calcultate[3] = GD_BUF[SQ_ADD_TIME]/5;
		 FPGA_Write(AOM2_DELAY_DIVIDE5_W,Calcultate[3]);
		
		if(UserSetValueChaFlag[aom1FreChaFlag])    //aom1的频率被修改
		{				
			FPGA_Write(FQ_FRE_W, GD_BUF[FQ_FRE]); 
			GD_BUF[FQ_FRE] = FPGA_Read(FQ_FRE_R);		
            log_i("GD_BUF[FQ_FRE] fpga back %d ", GD_BUF[FQ_FRE]);   
            AT24CXX_Write(AddFqFreB1, (uint8_t *)&GD_BUF[FQ_FRE], 1);
            AT24CXX_Read(AddFqFreB1, (uint8_t *)&GD_BUF[FQ_FRE], 1);
            log_i("GD_BUF[FQ_FRE] in flash is %d ", GD_BUF[FQ_FRE]); 
			UserSetValueChaFlag[aom1FreChaFlag] = false;
		}
				
		if(UserSetValueChaFlag[aom1BurstChaFlag])  //aom1的脉冲数被修改
		{
			FPGA_Write(Burst_Num_W, GD_BUF[BURST_NUM]);
			GD_BUF[BURST_NUM] = FPGA_Read(Burst_Num_R);
            log_i("GD_BUF[BURST_NUM] fpga back %d ", GD_BUF[BURST_NUM]); 
            AT24CXX_Write(AddFqBurst, (uint8_t *)&GD_BUF[BURST_NUM], 1);
            AT24CXX_Read(AddFqBurst, (uint8_t *)&GD_BUF[BURST_NUM], 1);
            log_i("GD_BUF[BURST_NUM] in flash is %d ", GD_BUF[BURST_NUM]); 
			UserSetValueChaFlag[aom1BurstChaFlag] = false;
		}					
	/***************AOM1/AOM2 delay Change*************************/

		 if(UserSetValueChaFlag[aom1DelayChaFlag])   //aom1延时有更改
		 {			
            //2024/1/30 发现该参数没有用
            //FPGA_Write(AOM1_DELAY_DIVIDE_5_W, GD_BUF[FQ_DELAY]); 
            GD_BUF[FQ_DELAY] = FPGA_Read(AOM1_DELAY_DIVIDE_5_R)*5;	
            //log_i("GD_BUF[FQ_DELAY] fpga back %d ", GD_BUF[FQ_DELAY]);              
            AT24CXX_Write(AddFqDelay, (uint8_t *)&GD_BUF[FQ_DELAY], 1);
            AT24CXX_Read(AddFqDelay, (uint8_t *)&GD_BUF[FQ_DELAY], 1);
            log_i("GD_BUF[FQ_DELAY] in flash is %d ", GD_BUF[FQ_DELAY]); 
			UserSetValueChaFlag[aom1DelayChaFlag] = false;
		 }
			 
		 if(UserSetValueChaFlag[aom2DelayChaFlag])   //aom2delay有更改
		{
			//2024/1/30 发现该参数没有用	
            //FPGA_Write(AOM2_DELAY_DIVIDE_5_W, GD_BUF[SQ_ADD_TIME]);             
			GD_BUF[SQ_ADD_TIME] = FPGA_Read(AOM2_DELAY_DIVIDE_5_R)*5;
            //log_i("GD_BUF[SQ_ADD_TIME] fpga back %d ", GD_BUF[SQ_ADD_TIME]); 
            AT24CXX_Write(AddSqAddTimeB1, (uint8_t *)&GD_BUF[SQ_ADD_TIME], 2);
            AT24CXX_Read(AddSqAddTimeB1, (uint8_t *)&GD_BUF[SQ_ADD_TIME], 2);
            log_i("GD_BUF[SQ_DELAY_TIME] in flash is %d ", GD_BUF[SQ_ADD_TIME]);
			UserSetValueChaFlag[aom2DelayChaFlag] = false;
		}
}



/********************************************************************************
*函数名: KeyScan
*参数  : 无
*功能  : 按钮状态检测
*返回值: 1-按钮弹起，0-按钮按下
*********************************************************************************/
rt_uint8_t KeyScan(void)
{
	static rt_uint8_t keyFilter[2]={0,0};
	static rt_uint8_t keyFlag;
	
	if(KEY_IN == 1)                                                                                  //钥匙关
	{
		keyFilter[0]++;
		keyFilter[1]=0;
		if(keyFilter[0] > 3)
		{   
			keyFilter[0] =0;
			keyFlag = 1;
		}
	}
	else                                                                                              //钥匙开
	{		
		keyFilter[0]=0;
		keyFilter[1]++;
		if(keyFilter[1] > 3)
		{
			keyFilter[1]=0;			                                                             
			keyFlag = 0;
		}
	}
	return keyFlag;
}


/********************************************************************************
*函数名: Key_Action
*参数  : 无
*功能  : 执行按钮状态相应的动作
*返回值: 无
*********************************************************************************/
void Key_Action(rt_uint8_t Value)
{
	if(Value == 1)  //按钮弹起
	{		
		GD_BUF[SEED_POWER_ENA] = off;
		GD_BUF[SEED_LASER_ENA] = off;
		GD_BUF[FQ_POWER_ENA]   = off;
		GD_BUF[SQ_POWER_ENA]   = off;
		GD_BUF[SQ_STANDBY_ENA] = on; //待机	
		GD_BUF[PUMP1_ENA]      = off;
		GD_BUF[PUMP2_ENA]      = off;
		GD_BUF[PUMP3_ENA]      = off;
	}
	else  //按钮按下
	{
		//进行按钮记忆，按钮按下时根据存储在EEPROME内的数据状态执行相应部件的开/关
				
		GD_BUF[SEED_POWER_ENA] = AT24CXX_ReadOneByte(AddSeedPower);
	
		GD_BUF[SEED_LASER_ENA] = AT24CXX_ReadOneByte(AddSeedLaser);
	
		GD_BUF[FQ_POWER_ENA] = AT24CXX_ReadOneByte(AddFqPower);
	
		GD_BUF[SQ_POWER_ENA] = AT24CXX_ReadOneByte(AddSqPower);
		
		GD_BUF[SQ_STANDBY_ENA] = AT24CXX_ReadOneByte(AddSqStandby);									 
		
		GD_BUF[PUMP1_ENA] = AT24CXX_ReadOneByte(AddPump1Ena);									 
		
		GD_BUF[PUMP2_ENA] = AT24CXX_ReadOneByte(AddPump2Ena);									 
		
		GD_BUF[PUMP3_ENA] = AT24CXX_ReadOneByte(AddPump3Ena);	

		//GD_BUF[PUMP4_ENA] = AT24CXX_ReadOneByte(AddPump4Ena);		      
	}			
}

static void other_entry(void* parameter)
{
	bool keyInitAct = true;
	
	while(1)
	{	
		//HmRead(&GD_BUF[HM_RH], &GD_BUF[HM_T]);                               //读取紫外腔温湿度值
		
		//Hm2Read(&GD_BUF[IR_HM_RH], &GD_BUF[IR_HM_T]);                        //读取红外腔温湿度值
		
		AlarmSet();                                                          //执行告警动作
		
		if(GD_BUF[STARTING])                                                 //初始化完成
		{
			Update_LaserTime_CNT();                                          //更新种子源，红外，紫外激光计时
		
			if(!ALARM_EN)                                                    //可硬件屏蔽报警
			{
				Monitor();                                                   //监测告警量
			}
		
			if( GD_BUF[ERR] == none)                                         //无告警
			{
				//fpga除法运算相关
				Fpga_Data_Calcultate();
				
				if( (keyState != KeyScan()) || (keyInitAct) )
				{
					keyState = KeyScan();                                        //按钮检测
					Key_Action(keyState);                                        //按钮功能
					keyInitAct = false;
				}
				
				if(!keyState)
				{
					if(laserSwitchFlag) 
					{
						if(GD_BUF[SWITCH])
						{
							GD_BUF[SEED_POWER_ENA] = on;
							GD_BUF[SEED_LASER_ENA] = on;
							GD_BUF[FQ_POWER_ENA] = on;
							GD_BUF[SQ_POWER_ENA] = on;
							GD_BUF[SQ_STANDBY_ENA] = on;
							GD_BUF[PUMP1_ENA] = on;
							GD_BUF[PUMP2_ENA] = on;
							GD_BUF[PUMP3_ENA] = on;
						}
						else
						{			
							GD_BUF[SEED_POWER_ENA] = off;
							GD_BUF[SEED_LASER_ENA] = off;
							GD_BUF[FQ_POWER_ENA] = off;
							GD_BUF[SQ_POWER_ENA] = off;
							GD_BUF[SQ_STANDBY_ENA] = on;
							GD_BUF[PUMP1_ENA] = off;
							GD_BUF[PUMP2_ENA] = off;
							GD_BUF[PUMP3_ENA] = off;
						}
						AT24CXX_WriteOneByte(AddSeedPower, GD_BUF[SEED_POWER_ENA]);
						AT24CXX_WriteOneByte(AddSeedLaser, GD_BUF[SEED_LASER_ENA]);
						AT24CXX_WriteOneByte(AddFqPower, GD_BUF[FQ_POWER_ENA]);
						AT24CXX_WriteOneByte(AddSqPower, GD_BUF[SQ_POWER_ENA]);
//						AT24CXX_WriteOneByte(AddSqStandby, GD_BUF[SQ_STANDBY_ENA]); 			
						AT24CXX_WriteOneByte(AddPump1Ena, GD_BUF[PUMP1_ENA]);
						AT24CXX_WriteOneByte(AddPump2Ena, GD_BUF[PUMP2_ENA]);
						AT24CXX_WriteOneByte(AddPump3Ena, GD_BUF[PUMP3_ENA]);
						
						laserSwitchFlag = false;		    
					}
				}
			}
		    //aom2操作
			Aom2CtrlAct(GD_BUF[SQ_POWER_ENA], GD_BUF[SQ_STANDBY_ENA], GD_BUF[SQ_ATTEN_SEL], GD_BUF[SQ_TYPE]);  //放在此处是让Aom2的控制不受泵开/关限制
		}
		rt_thread_mdelay(500);
	}
}


/********************************************************************************************
函数名：PumpSwJudge
功能  ：泵浦开/关条件判断
参数  ：无
返回值：无
*********************************************************************************************/
void PumpSwJudge(void)
{
	if(!ALARM_EN) //整机运行
	{
//		if( (GD_BUF[SEED_LASER_ENERGY_MON] >= 90 ) && (ReadFQPowerPinState) )    //种子源稳定出光 Fiber Q打开
		
		if((ReadFQPowerPinState) )	 //种子源稳定出光 Fiber Q打开
//		if( (ReadSeedLaserPinState) && (ReadFQPowerPinState) )
		{
//			if(GD_BUF[COLLIMATOR_IR_STATE] == 1)    //检测到准直器有种子光才允许执行开LD操作
			        
//			{
				if(GD_BUF[PUMP1_ENA] == on)   //开泵1
				{
					laserPart.pump1State = on;	
				}
				else if( (GD_BUF[PUMP1_ENA] == off) && (GD_BUF[PUMP2_I_READ] < 5) )  //泵2关闭后才允许关泵1
				{
					laserPart.pump1State = off;
				}
				
				if( (GD_BUF[PUMP2_ENA] == on) && (GD_BUF[PUMP1_I_READ] > 5 ) && ((GD_BUF[PUMP1_I_SET] + 3 - GD_BUF[PUMP1_I_READ]) < 6) ) //泵1打开后，才允许开泵2
				{
					laserPart.pump2State = on;	
				}
				else if( (GD_BUF[PUMP2_ENA] == off) && (GD_BUF[PUMP3_I_READ] < 5) ) //泵3关闭后才允许关泵2
				{
					laserPart.pump2State = off;
				}
				
				if( (GD_BUF[PUMP3_ENA] == on) && (GD_BUF[PUMP2_I_READ] > 5) && ((GD_BUF[PUMP2_I_SET] + 3 - GD_BUF[PUMP2_I_READ]) < 6) ) //泵2打开后，才允许开泵3
				{
					laserPart.pump3State = on;	
				}
				else if(GD_BUF[PUMP3_ENA] == off) //关泵3
				{
					laserPart.pump3State = off;
				}
		}
		else   //种子光意外中断
		{
			laserPart.pump1State = off;
			laserPart.pump2State = off;
			laserPart.pump3State = off;
		}
	}
	else     //用于单板测试
	{
		if(GD_BUF[PUMP1_ENA] == 1)
		{
			laserPart.pump1State = on;
		}
		else
		{
			laserPart.pump1State = off;
		}
		
		if(GD_BUF[PUMP2_ENA] == 1)
		{
			laserPart.pump2State = on;
		}
		else
		{
			laserPart.pump2State = off;
		}
		
		if(GD_BUF[PUMP3_ENA] == 1)
		{
			laserPart.pump3State = on;
		}
		else
		{
			laserPart.pump3State = off;
		}
		
//		if(GD_BUF[PUMP4_ENA] == 1)
//		{
//			laserPart.pump4State = on;
//		}
//		else
//		{
//			laserPart.pump4State = off;
//		}
	}
}

/********************************************************************************************
函数名：PumpSwAct
功能  ：执行泵浦开/关
参数  ：无
返回值：无
*********************************************************************************************/
void PumpSwAct(void) 
{	
	//关泵顺序3-2-1
   	//ACTIVE_PUMP_I_SET(laserPart.pump4State, GD_BUF[PUMP4_I_SET], 3);
	
	//ACTIVE_PUMP_I_SET(laserPart.pump3State, GD_BUF[PUMP3_I_SET], 2);   
	
	ACTIVE_PUMP_I_SET(laserPart.pump2State, GD_BUF[PUMP2_I_SET], 1);
	
	ACTIVE_PUMP_I_SET(laserPart.pump1State, GD_BUF[PUMP1_I_SET], 0);
}

/********************************************************************************
*函数名: PumpCtrl
*参数  : 无
*功能  : 泵的相关控制
*返回值: 无
*********************************************************************************/
void PumpCtrl(void)
{
	PumpSwJudge();
	PumpSwAct();
}

static void laser_entry(void* parameter)
{
	while(1)
	{
		if(GD_BUF[STARTING]) //初始化完成
		{
			//种子源控制操作
			SeedCtrlAct(GD_BUF[SEED_POWER_ENA], GD_BUF[SEED_LASER_ENA], GD_BUF[SEED_LASER_ENERGY_MON],GD_BUF[SEED_TYPE], GD_BUF[PUMP1_I_READ], GD_BUF[PUMP2_I_READ], GD_BUF[PUMP3_I_READ], 0 );
			
			//Aom1控制操作
			if(GD_BUF[SEED_TYPE] == TOPTICA)
			{
				Aom1CtrlAct(GD_BUF[FQ_POWER_ENA],GD_BUF[SEED_TYPE], GD_BUF[SEED_LASER_ENERGY_MON]); 
			}
			else if(GD_BUF[SEED_TYPE] == NPI)
			{
				Aom1CtrlAct(GD_BUF[FQ_POWER_ENA],GD_BUF[SEED_TYPE], ReadSeedLaserPinState);
			}
			
			//泵控制操作
			PumpCtrl();
		}
		rt_thread_mdelay(500);
	}
}

int task_init(void)
{
    All_Init();   
    
    tid_init_task = rt_thread_create("initail",
                            Initail_Check_Task, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY-1, THREAD_TIMESLICE);
    if (tid_init_task != RT_NULL)
        rt_thread_startup(tid_init_task);

    other_task = rt_thread_create("other",
                            other_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY-4,THREAD_TIMESLICE);
    if (other_task != RT_NULL)
        rt_thread_startup(other_task);
    
    laser_task = rt_thread_create("laser",
                            laser_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY-4,THREAD_TIMESLICE);
    if (laser_task != RT_NULL)
        rt_thread_startup(laser_task);
    
    return 0;
}
