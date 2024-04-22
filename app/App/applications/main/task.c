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

#define scanNum                                                 20                 //���澯λɨ�����
#define CLK_FS                                                  200000000          //FPGA ��Ƶʱ��200MHz
                                                     
rt_uint8_t rtcFlag[2] = {0,0};                                                     //����RTC���ں�ʱ����±�־
rt_uint8_t keyState = 1;                                                           //���水ť״̬  

rt_bool_t UserSetValueChaFlag[4] = {false,false,false,false};                      //UIĳֵ�޸ı�־λ�ֱ���FreCha\BurstCha\aom1delayCha\aom2delayCha
rt_bool_t laserSwitchFlag = true;                                                  //SWITCH������־λ

//����õĿ���״/̬
typedef struct
{ 
	rt_uint8_t pump1State;					 										//��1���ر�־λ,1-ON,0-OFF
	rt_uint8_t pump2State;					 										//��2���ر�־λ,1-ON,0-OFF
	rt_uint8_t pump3State;					 										//��3���ر�־λ,1-ON,0-OFF
	rt_uint8_t pump4State;					 										//��4���ر�־λ,1-ON,0-OFF
}laserPartEnaStruct;

laserPartEnaStruct laserPart={0,0,0,0};  


/********************************************************************************
*������: get_eeprom_data
*����  : ��
*����  : д����ȡEEPROM����
*����ֵ: ��
*********************************************************************************/
void get_eeprom_data(void)
{
	if(at24c08_check() == RT_EOK ) //EEPROM��ȡ
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
        
        // ����Ĭ�Ϲرձã���ֹ����ֱ�ӳ���
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
	else   //���г�ʼ���ݸ���
	{	
		GD_BUF[SN1]                    = 0x00000000;                         //������SN��
		GD_BUF[SN2]                    = 0x00000000;                         //������SN��
		GD_BUF[SN3]                    = 0x00000001;                         //������SN��
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
		GD_BUF[AIRPUMP_FREQ] 		   = 20;                                 //����Ƶ��
		GD_BUF[AIRPUMP_DUTY]  		   = 66;                                 //����ռ�ձ�		

		GD_BUF[IR_TIME]                = 0;                                  //LDʹ�ü�ʱ
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
	
		GD_BUF[INIT_TIME]              = 2;                                  //Ĭ�ϳ�ʼ��ʱ��1����
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
//		GD_BUF[SerialNumA1]            = Product_Password();                 //������������
//		GD_BUF[SerialNumB1]            = GD_BUF[SerialNumA1];                //��������		

		AT24CXX_WriteOneByte(AddEepromCheck,(EEPROMPASS));                     //EEPROM��д��־

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
		
		AT24CXX_WriteOneByte(AddIrLaserTimeB1, GD_BUF[IR_TIME] & 0xff); //2^24 * 0.1H / 24 = 69905 ��
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
		AT24CXX_WriteOneByte(AddPump1HAlarmCnt, GD_BUF[PUMP1_H_ALARM_CNT]); //����255��
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
		GD_BUF[USER_LIMITDAY]          = 255;                                //��������	
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
    get_eeprom_data();              //��ȡEEPROM�洢���� 
    rt_thread_mdelay(1000);                 
    Seed_Init();                    //����Դ��ʼ��
    Aom_Init();                     //AOM��ʼ��
    PUMP_Init();                    //���ֳ�ʼ��
//    HM_Init();                      //AHT20��ʪ�ȼƳ�ʼ��
    STHG_Init();                    //��/����Ƶ��ʼ��     
    DAC_CBM128S085_Init();
    
    ACTIVE_SEED_ENERGY_CTRL(GD_BUF[SEED_LASER_ENERGY_CTRL],Channel_E);  //��������Դ����
    Aom1RfSet(GD_BUF[FQ_RF_SCALE],Channel_F);                           //����Fiber Q��Ƶ���� 
    STHG_TEMP_SET(GD_BUF[SHG_SET],Channel_G);                           //���ö���Ƶ�¶�
    STHG_TEMP_SET(GD_BUF[THG_SET],Channel_H);                           //��������Ƶ�¶�
    SHG_ENA_ON();                 									    //ʹ�ܶ���Ƶ
    THG_ENA_ON(); 		         									    //ʹ������Ƶ
    
    ACTIVE_WR_FPGA();           									    //��FPGAд������
    
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

	static rt_uint32_t init_cnt = 0;  //��ʼ�����̼�ʱ
    static rt_uint8_t checkItemCnt[8]={0,0,0,0,0,0,0,0}; //��¼ÿ��ļ�����
    bool checkItemResult[8]={false,false,false,false,false,false,false,false}; //�����ʼ��ʱ�����Ľ��
	
	//�ϵ��ʼ�������
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
        rt_thread_mdelay(100);                                          //��ʱ100ms
		//��Ӳ�����γ�ʼ�����
        if(!ALARM_EN)                                                   
        {
            if( GD_BUF[ERR] == none)                                     //�޸澯
            {
				if(GD_BUF[SEED_TYPE] == TOPTICA)
				{
					//����Դ���
					if( SEED_ALARM != 0)      
					{
						checkItemCnt[seedItem]++;
						if(checkItemCnt[seedItem] > scanNum)                 //���scanNum�����쳣
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

				//�ռ�Q���
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
				
                //��1�¶ȼ��
                if(IsInSide(GD_BUF[PUMP1_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM]+10, GD_BUF[PUMP_TEMP_H_ALARM]-10))//���趨�������ޱ�����ΧС1��
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
				
				//��2�¶ȼ��
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
				
//				//��3�¶ȼ��
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
				
				//ǻ��1���¶ȼ��
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
				
				//ǻ��2���¶ȼ��
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
				
                //��Ҫ��ʼ�����
                if(GD_BUF[INIT_TIME] != 0) 
                {
                    if(init_cnt++ > GD_BUF[INIT_TIME] * 300) //��������ʼ��ʱ�䣬����,�԰����Ϊ����
                    {
                        GD_BUF[ERR_DETAIL] = initailBit;   
						
                        GD_BUF[ERR] = initialWarning ;
						
                        if(GD_BUF[INIT_ALARM_CNT] < 255) 
                        { 
                            GD_BUF[INIT_ALARM_CNT]++;
                            AT24CXX_WriteOneByte(AddInitAlarmCnt, GD_BUF[INIT_ALARM_CNT]); //��¼��������
                        }
                       return;	         //ɾ��start_task��������
                    }
                    GD_BUF[INIT_TIME_CNT] = init_cnt / 10;   //1S�ۼ�1��
                }
                //�����ʼ�����ȴ���ֱ�ӽ���������
                else
                {
                    rt_thread_mdelay(2000);             //��ʱ2s

                    GD_BUF[STARTING] = true;

                    return;	//ɾ��start_task��������
                }
            }
        }
        //����ʼ�������������
//		if( ( (checkItemResult[seedItem] && checkItemResult[sqItem] && checkItemResult[pump1Item] && checkItemResult[pump2Item] && checkItemResult[pump3Item]\
//			&& checkItemResult[cavityItem] && checkItemResult[cavity2Item] ) && (GD_BUF[ERR] == none) ) || ALARM_EN )
		if( ( (checkItemResult[seedItem] && checkItemResult[sqItem] && checkItemResult[pump1Item] && checkItemResult[pump2Item]\
			&& checkItemResult[cavityItem] ) && (GD_BUF[ERR] == none) ) || ALARM_EN )
        {
            GD_BUF[STARTING] = true;
			
            return;	//ɾ��start_task��������
        }
    }
}

/********************************************************************************
*������: Monitor
*����  : ��
*����  : ���LD,��ѧǻ�¶�,��λ�澯��־
*����ֵ: ��
*GD_BUF[ERR]
*1-Seed error;2-Space Q error;3-Pump1 T error;4-Pump2 T error;5-Pump3 T error;6-Pump4 T error;7-Caviyt error
*********************************************************************************/
void Monitor(void)
{
	static rt_uint8_t cnt[8]={0,0,0,0,0,0,0,0};
	//�������Դ�Ƿ�����澯
	if(GD_BUF[SEED_TYPE] == TOPTICA)
	{
		if( (SEED_ALARM == 0) && (GD_BUF[ERR]== none) )  //�͵�ƽ����ʱ��> 400ms ��Ч
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
			if( (SEED_ALARM == 0) && (GD_BUF[ERR]== none) )  //�͵�ƽ����ʱ��> 17s ��Ч
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

	//���ռ�Q�Ƿ�����澯
	if( (SQ_SWR_ALARM == 0) && (GD_BUF[ERR] == none) )  //ֻ����1�θ澯����
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
	//����1�¶��Ƿ񳬳���Χ
	if( IsOutSide( GD_BUF[PUMP1_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM], GD_BUF[PUMP_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none) )  //ֻ����1�θ澯����
	{	
		cnt[2]++;
		if(cnt[2] > scanNum)   //20*100ms=2s
		{
			cnt[2] = 0;
			GD_BUF[ERR] = pump1Warning;	                                 //PUMP1�¶ȱ���
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
    //����2�¶��Ƿ񳬳���Χ

	if( IsOutSide( GD_BUF[PUMP2_TEMP_READ], GD_BUF[PUMP_TEMP_L_ALARM], GD_BUF[PUMP_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none) )   //ֻ����1�θ澯����
	{	
		cnt[3]++;
		if(cnt[3] > scanNum)   //20*100ms=2s
		{
			cnt[3] = 0;
			GD_BUF[ERR] = pump2Warning;	                                 //PUMP2�¶ȱ���
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
	
	//���ǻ���¶��Ƿ񳬳���Χ
	if( IsOutSide( GD_BUF[CAVITY_TEMP_READ], GD_BUF[CAVITY_TEMP_L_ALARM], GD_BUF[CAVITY_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none)  )  //ֻ����1�θ澯����	
	{	
		cnt[6]++;
		if(cnt[6] > scanNum)  //20*100ms=2s
		{
			cnt[6] = 0; 											
			GD_BUF[ERR] = cavityWarning;	                                 //CAVITY�¶ȱ���,�رռ���
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
	
	//���ǻ��2���¶��Ƿ񳬳���Χ
//	if( IsOutSide( GD_BUF[CAVITY2_TEMP_READ], GD_BUF[CAVITY_TEMP_L_ALARM], GD_BUF[CAVITY_TEMP_H_ALARM] ) && (GD_BUF[ERR] == none)  )  //ֻ����1�θ澯����	
//	{	
//		cnt[7]++;
//		if(cnt[7] > scanNum)  //20*100ms=2s
//		{
//			cnt[7] = 0; 											
//			GD_BUF[ERR] = cavity2Warning;	                                 //CAVITY2���¶ȱ���,�رռ���
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
*������: AlarmSet
*����  : ��
*����  : ִ�и澯����
*����ֵ: ��
*********************************************************************************/
void AlarmSet(void)
{
	if((GD_BUF[ERR]&0xff) != none)
	{			
		GD_BUF[SEED_POWER_ENA] = off;
		GD_BUF[SEED_LASER_ENA] = off;
		GD_BUF[FQ_POWER_ENA]   = off;
		if(GD_BUF[PUMP1_I_READ] < 10)                            //�ùر��ٹ�SQ
		{
			GD_BUF[SQ_POWER_ENA]   = off;
		}
		GD_BUF[SQ_STANDBY_ENA] = on; //����	
		GD_BUF[PUMP1_ENA]      = off;
		GD_BUF[PUMP2_ENA]      = off;
		GD_BUF[PUMP3_ENA]      = off;
		
		ERR_LED_ON();		                                     //ERR����
		rt_timer_start(timer_beep); 	                         //ʹ�ܷ��������ƶ�ʱ��  			
	}	
}


/********************************************************************************
*������: Update_LaserTime_CNT
*����  : ��
*����  : ���³����ʱ
*����ֵ: ��
*********************************************************************************/
void Update_LaserTime_CNT(void)
{
	if(USER_TIME_FLAG[0]) 
	{
		USER_TIME_FLAG[0] = 0;
		AT24CXX_Write(AddSeedLaserTimeB1, (uint8_t *)&GD_BUF[SEED_TIME], 3);
	}
	else if(USER_TIME_FLAG[1])                                                                                //ÿ0.1H�洢һ�μ���ֵ
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

/*fpga����*/
void Fpga_Data_Calcultate(void)
{
		rt_uint16_t Calcultate[4]={0};//���������aom1CycleTotalCnt��aom1DutyTotalCnt��AOM1delay/5��AOM2delay/5
		rt_uint16_t Seedfretemp = 0;
		
		Seedfretemp = (FPGA_Read(SEED_FRE_R));//��ȡ����ԭƵ�� out_data <= seedFre / 12'd1000;
		if(Seedfretemp < 1)
		{
			Seedfretemp =1;
		}	
		GD_BUF[SEED_FRE] = Seedfretemp;
		
		/******AOM1fre*****/
		/*aom1CycleTotalCnt <= (seedFreTemp / aom1FreTemp / 12'd1000);������Դʱ�Ӽ���AOM1����*/
		Calcultate[0] = Seedfretemp/GD_BUF[FQ_FRE];
		FPGA_Write(AOM1_CYCLE_TOTAL_CNT_W,Calcultate[0]);
		
		 /*aom1DutyTotalCnt  <= CLK_FS * aom1BurstNumTemp / seedFreTemp;��200Mʱ�Ӽ���aom1ռ�ձ�*/
		 Calcultate[1] = CLK_FS*GD_BUF[BURST_NUM]/Seedfretemp/1000;
		 FPGA_Write(AOM1_DUTY_TOTAL_CNT_W,Calcultate[1]);
		
		 /*AOM1delay/5*/
		 Calcultate[2] = GD_BUF[FQ_DELAY]/5;
		 FPGA_Write(AOM1_DELAY_DIVIDE5_W,Calcultate[2]);	
		
		 /*AOM2delay/5*/
		 Calcultate[3] = GD_BUF[SQ_ADD_TIME]/5;
		 FPGA_Write(AOM2_DELAY_DIVIDE5_W,Calcultate[3]);
		
		if(UserSetValueChaFlag[aom1FreChaFlag])    //aom1��Ƶ�ʱ��޸�
		{				
			FPGA_Write(FQ_FRE_W, GD_BUF[FQ_FRE]); 
			GD_BUF[FQ_FRE] = FPGA_Read(FQ_FRE_R);		
            log_i("GD_BUF[FQ_FRE] fpga back %d ", GD_BUF[FQ_FRE]);   
            AT24CXX_Write(AddFqFreB1, (uint8_t *)&GD_BUF[FQ_FRE], 1);
            AT24CXX_Read(AddFqFreB1, (uint8_t *)&GD_BUF[FQ_FRE], 1);
            log_i("GD_BUF[FQ_FRE] in flash is %d ", GD_BUF[FQ_FRE]); 
			UserSetValueChaFlag[aom1FreChaFlag] = false;
		}
				
		if(UserSetValueChaFlag[aom1BurstChaFlag])  //aom1�����������޸�
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

		 if(UserSetValueChaFlag[aom1DelayChaFlag])   //aom1��ʱ�и���
		 {			
            //2024/1/30 ���ָò���û����
            //FPGA_Write(AOM1_DELAY_DIVIDE_5_W, GD_BUF[FQ_DELAY]); 
            GD_BUF[FQ_DELAY] = FPGA_Read(AOM1_DELAY_DIVIDE_5_R)*5;	
            //log_i("GD_BUF[FQ_DELAY] fpga back %d ", GD_BUF[FQ_DELAY]);              
            AT24CXX_Write(AddFqDelay, (uint8_t *)&GD_BUF[FQ_DELAY], 1);
            AT24CXX_Read(AddFqDelay, (uint8_t *)&GD_BUF[FQ_DELAY], 1);
            log_i("GD_BUF[FQ_DELAY] in flash is %d ", GD_BUF[FQ_DELAY]); 
			UserSetValueChaFlag[aom1DelayChaFlag] = false;
		 }
			 
		 if(UserSetValueChaFlag[aom2DelayChaFlag])   //aom2delay�и���
		{
			//2024/1/30 ���ָò���û����	
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
*������: KeyScan
*����  : ��
*����  : ��ť״̬���
*����ֵ: 1-��ť����0-��ť����
*********************************************************************************/
rt_uint8_t KeyScan(void)
{
	static rt_uint8_t keyFilter[2]={0,0};
	static rt_uint8_t keyFlag;
	
	if(KEY_IN == 1)                                                                                  //Կ�׹�
	{
		keyFilter[0]++;
		keyFilter[1]=0;
		if(keyFilter[0] > 3)
		{   
			keyFilter[0] =0;
			keyFlag = 1;
		}
	}
	else                                                                                              //Կ�׿�
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
*������: Key_Action
*����  : ��
*����  : ִ�а�ť״̬��Ӧ�Ķ���
*����ֵ: ��
*********************************************************************************/
void Key_Action(rt_uint8_t Value)
{
	if(Value == 1)  //��ť����
	{		
		GD_BUF[SEED_POWER_ENA] = off;
		GD_BUF[SEED_LASER_ENA] = off;
		GD_BUF[FQ_POWER_ENA]   = off;
		GD_BUF[SQ_POWER_ENA]   = off;
		GD_BUF[SQ_STANDBY_ENA] = on; //����	
		GD_BUF[PUMP1_ENA]      = off;
		GD_BUF[PUMP2_ENA]      = off;
		GD_BUF[PUMP3_ENA]      = off;
	}
	else  //��ť����
	{
		//���а�ť���䣬��ť����ʱ���ݴ洢��EEPROME�ڵ�����״ִ̬����Ӧ�����Ŀ�/��
				
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
		//HmRead(&GD_BUF[HM_RH], &GD_BUF[HM_T]);                               //��ȡ����ǻ��ʪ��ֵ
		
		//Hm2Read(&GD_BUF[IR_HM_RH], &GD_BUF[IR_HM_T]);                        //��ȡ����ǻ��ʪ��ֵ
		
		AlarmSet();                                                          //ִ�и澯����
		
		if(GD_BUF[STARTING])                                                 //��ʼ�����
		{
			Update_LaserTime_CNT();                                          //��������Դ�����⣬���⼤���ʱ
		
			if(!ALARM_EN)                                                    //��Ӳ�����α���
			{
				Monitor();                                                   //���澯��
			}
		
			if( GD_BUF[ERR] == none)                                         //�޸澯
			{
				//fpga�����������
				Fpga_Data_Calcultate();
				
				if( (keyState != KeyScan()) || (keyInitAct) )
				{
					keyState = KeyScan();                                        //��ť���
					Key_Action(keyState);                                        //��ť����
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
		    //aom2����
			Aom2CtrlAct(GD_BUF[SQ_POWER_ENA], GD_BUF[SQ_STANDBY_ENA], GD_BUF[SQ_ATTEN_SEL], GD_BUF[SQ_TYPE]);  //���ڴ˴�����Aom2�Ŀ��Ʋ��ܱÿ�/������
		}
		rt_thread_mdelay(500);
	}
}


/********************************************************************************************
��������PumpSwJudge
����  �����ֿ�/�������ж�
����  ����
����ֵ����
*********************************************************************************************/
void PumpSwJudge(void)
{
	if(!ALARM_EN) //��������
	{
//		if( (GD_BUF[SEED_LASER_ENERGY_MON] >= 90 ) && (ReadFQPowerPinState) )    //����Դ�ȶ����� Fiber Q��
		
		if((ReadFQPowerPinState) )	 //����Դ�ȶ����� Fiber Q��
//		if( (ReadSeedLaserPinState) && (ReadFQPowerPinState) )
		{
//			if(GD_BUF[COLLIMATOR_IR_STATE] == 1)    //��⵽׼ֱ�������ӹ������ִ�п�LD����
			        
//			{
				if(GD_BUF[PUMP1_ENA] == on)   //����1
				{
					laserPart.pump1State = on;	
				}
				else if( (GD_BUF[PUMP1_ENA] == off) && (GD_BUF[PUMP2_I_READ] < 5) )  //��2�رպ������ر�1
				{
					laserPart.pump1State = off;
				}
				
				if( (GD_BUF[PUMP2_ENA] == on) && (GD_BUF[PUMP1_I_READ] > 5 ) && ((GD_BUF[PUMP1_I_SET] + 3 - GD_BUF[PUMP1_I_READ]) < 6) ) //��1�򿪺󣬲�������2
				{
					laserPart.pump2State = on;	
				}
				else if( (GD_BUF[PUMP2_ENA] == off) && (GD_BUF[PUMP3_I_READ] < 5) ) //��3�رպ������ر�2
				{
					laserPart.pump2State = off;
				}
				
				if( (GD_BUF[PUMP3_ENA] == on) && (GD_BUF[PUMP2_I_READ] > 5) && ((GD_BUF[PUMP2_I_SET] + 3 - GD_BUF[PUMP2_I_READ]) < 6) ) //��2�򿪺󣬲�������3
				{
					laserPart.pump3State = on;	
				}
				else if(GD_BUF[PUMP3_ENA] == off) //�ر�3
				{
					laserPart.pump3State = off;
				}
		}
		else   //���ӹ������ж�
		{
			laserPart.pump1State = off;
			laserPart.pump2State = off;
			laserPart.pump3State = off;
		}
	}
	else     //���ڵ������
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
��������PumpSwAct
����  ��ִ�б��ֿ�/��
����  ����
����ֵ����
*********************************************************************************************/
void PumpSwAct(void) 
{	
	//�ر�˳��3-2-1
   	//ACTIVE_PUMP_I_SET(laserPart.pump4State, GD_BUF[PUMP4_I_SET], 3);
	
	//ACTIVE_PUMP_I_SET(laserPart.pump3State, GD_BUF[PUMP3_I_SET], 2);   
	
	ACTIVE_PUMP_I_SET(laserPart.pump2State, GD_BUF[PUMP2_I_SET], 1);
	
	ACTIVE_PUMP_I_SET(laserPart.pump1State, GD_BUF[PUMP1_I_SET], 0);
}

/********************************************************************************
*������: PumpCtrl
*����  : ��
*����  : �õ���ؿ���
*����ֵ: ��
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
		if(GD_BUF[STARTING]) //��ʼ�����
		{
			//����Դ���Ʋ���
			SeedCtrlAct(GD_BUF[SEED_POWER_ENA], GD_BUF[SEED_LASER_ENA], GD_BUF[SEED_LASER_ENERGY_MON],GD_BUF[SEED_TYPE], GD_BUF[PUMP1_I_READ], GD_BUF[PUMP2_I_READ], GD_BUF[PUMP3_I_READ], 0 );
			
			//Aom1���Ʋ���
			if(GD_BUF[SEED_TYPE] == TOPTICA)
			{
				Aom1CtrlAct(GD_BUF[FQ_POWER_ENA],GD_BUF[SEED_TYPE], GD_BUF[SEED_LASER_ENERGY_MON]); 
			}
			else if(GD_BUF[SEED_TYPE] == NPI)
			{
				Aom1CtrlAct(GD_BUF[FQ_POWER_ENA],GD_BUF[SEED_TYPE], ReadSeedLaserPinState);
			}
			
			//�ÿ��Ʋ���
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
