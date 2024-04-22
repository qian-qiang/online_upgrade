/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-31     qq           the first version
 */

#define LOG_TAG      "unpack"
#define LOG_LVL      LOG_LVL_DBG

#include <rtthread.h>
#include <board.h>
#include "rtdbg.h"
#include "serial_unpack.h"
#include "serial_pc.h"
#include "stdbool.h"
#include "task.h"
#include "seed.h"
#include "aom.h"
#include "at24c08.h"
#include "fpga.h"
#include "ShgThg.h"
#include "fan.h"
#include <stdlib.h>
#include <sys/time.h>

static void uart_protocol_unpack(uart_cmd_t *pack, uint8_t* data, uint16_t size);
static void pc_handle_cb(uint8_t *d, uint16_t size);

static uint8_t bcd2dec(uint8_t bcd)
{
	return (bcd - (bcd >> 4) * 6);
}

static void uart_pc_process_send(uint8_t *d, uint16_t size)
{
    uart_pc_send(d, size);
}

int serial_pc_msg_unpack(uint8_t *buf, uint32_t length)
{
    pc_handle_cb(buf, length);
    return RT_EOK;
}

void run_cmd_set(rt_uint8_t cmd, uart_cmd_t *pack)
{
	switch(cmd)
	{ 
		case SN1:		           	
            AT24CXX_Write(AddSn1Byte1, &pack->data[0], sizeof(GD_BUF[SN1]));       
            AT24CXX_Read(AddSn1Byte1, (uint8_t *)&GD_BUF[SN1], sizeof(GD_BUF[SN1]));
            log_i("GD_BUF[SN1] set %lx",GD_BUF[SN1]);   
            break;
		case SN2:
            AT24CXX_Write(AddSn2Byte1, &pack->data[0], sizeof(GD_BUF[SN2]));       
            AT24CXX_Read(AddSn2Byte1, (uint8_t *)&GD_BUF[SN2], sizeof(GD_BUF[SN2])); 
            log_i("GD_BUF[SN2] set %lx",GD_BUF[SN2]); 
			break;	
		case SN3:
            AT24CXX_Write(AddSn3Byte1, &pack->data[0], sizeof(GD_BUF[SN3]));
            AT24CXX_Read(AddSn3Byte1, (uint8_t *)&GD_BUF[SN3], sizeof(GD_BUF[SN3])); 
            log_i("GD_BUF[SN3] set %lx",GD_BUF[SN3]);         
		    break;	
        case IR_TIME:
            AT24CXX_Write(AddIrLaserTimeB1, &pack->data[0], 3);
            AT24CXX_Read(AddIrLaserTimeB1, (uint8_t *)&GD_BUF[IR_TIME], 3);
            log_i("GD_BUF[IR_TIME] set %d", &GD_BUF[IR_TIME]);   
			USER_TIME_FLAG[1]= 0;  //更新出光计时
			break; 
		case UV_TIME:
            AT24CXX_Write(AddUvLaserTimeB1, &pack->data[0], 3);
            AT24CXX_Read(AddUvLaserTimeB1, (uint8_t *)&GD_BUF[UV_TIME], 3);
            log_i("GD_BUF[UV_TIME] set %d", &GD_BUF[UV_TIME]);   
            USER_TIME_FLAG[2] = 0;  //更新出光计时
            break;        
        case SEED_TIME:   
            AT24CXX_Write(AddSeedLaserTimeB1, &pack->data[0], 3);
            AT24CXX_Read(AddSeedLaserTimeB1, (uint8_t *)&GD_BUF[SEED_TIME], 3);
            log_i("GD_BUF[SEED_TIME] set %d", &GD_BUF[SEED_TIME]);   
            USER_TIME_FLAG[0]=0;  //更新出光计时
            break;	
		case PUMP1_ENA: 							
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING])  )
            {
                AT24CXX_Write(AddPump1Ena, &pack->data[0], 1);
                AT24CXX_Read(AddPump1Ena, (uint8_t *)&GD_BUF[PUMP1_ENA], 1);
                log_i("GD_BUF[PUMP1_ENA] set %s", GD_BUF[PUMP1_ENA]?"on":"off");   
            }
            break;  				
		case PUMP2_ENA: 
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]) )
            {
                AT24CXX_Write(AddPump2Ena, &pack->data[0], 1);
                AT24CXX_Read(AddPump2Ena, (uint8_t *)&GD_BUF[PUMP2_ENA], 1);
                log_i("GD_BUF[PUMP2_ENA] set %s", GD_BUF[PUMP2_ENA]?"on":"off");   
            }
            break; 		
		case PUMP1_I_SET:
            rt_memcpy(&GD_BUF[PUMP1_I_SET], &pack->data[0], sizeof(GD_BUF[PUMP1_I_SET]));	     
            if(GD_BUF[PUMP1_I_SET] > GD_BUF[PUMP1_I_MAX])
            {
                GD_BUF[PUMP1_I_SET] = GD_BUF[PUMP1_I_MAX];
            }
            if(GD_BUF[PUMP1_I_SET] < 1)
            {
                GD_BUF[PUMP1_I_SET] = 0;
            }
            AT24CXX_Write(AddPump1ISetByte1, &pack->data[0], 2);
            AT24CXX_Read(AddPump1ISetByte1, (uint8_t *)&GD_BUF[PUMP1_I_SET], 2);
            log_i("GD_BUF[PUMP1_I_SET] set %d.%d A", GD_BUF[PUMP1_I_SET]/10,GD_BUF[PUMP1_I_SET]%10);
            break; 	
		case PUMP2_I_SET:
            rt_memcpy(&GD_BUF[PUMP2_I_SET], &pack->data[0], sizeof(GD_BUF[PUMP2_I_SET]));	 
            if(GD_BUF[PUMP2_I_SET] > GD_BUF[PUMP2_I_MAX])
            {
                GD_BUF[PUMP2_I_SET] = GD_BUF[PUMP2_I_MAX];
            }
            if(GD_BUF[PUMP2_I_SET] < 1)
            {
                GD_BUF[PUMP2_I_SET] = 0;
            } 
            AT24CXX_Write(AddPump2ISetByte1, &pack->data[0], 2);
            AT24CXX_Read(AddPump2ISetByte1, (uint8_t *)&GD_BUF[PUMP2_I_SET], 2);
            log_i("GD_BUF[PUMP2_I_SET] set %d.%d A", GD_BUF[PUMP2_I_SET]/10,GD_BUF[PUMP2_I_SET]%10);
            break;
		case PUMP1_I_MAX:
            AT24CXX_Write(AddPump1IMaxByte1, &pack->data[0], 2);
            AT24CXX_Read(AddPump1IMaxByte1, (uint8_t *)&GD_BUF[PUMP1_I_MAX], 2);
            log_i("GD_BUF[PUMP1_I_MAX] set %d.%d A", GD_BUF[PUMP1_I_MAX]/10,GD_BUF[PUMP1_I_MAX]%10);
            break;
		case PUMP2_I_MAX:
            AT24CXX_Write(AddPump2IMaxByte1, &pack->data[0], 2);
            AT24CXX_Read(AddPump2IMaxByte1, (uint8_t *)&GD_BUF[PUMP2_I_MAX], 2);
            log_i("GD_BUF[PUMP2_I_MAX] set %d.%d A", GD_BUF[PUMP2_I_MAX]/10,GD_BUF[PUMP2_I_MAX]%10);
            break;
		case PUMP_TEMP_H_ALARM:
            AT24CXX_Write(AddPumpTempHAlarmB1, &pack->data[0], 2);
            AT24CXX_Read(AddPumpTempHAlarmB1, (uint8_t *)&GD_BUF[PUMP_TEMP_H_ALARM], 2);
            log_i("GD_BUF[PUMP_TEMP_H_ALARM] set %d.%d `C", GD_BUF[PUMP_TEMP_H_ALARM]/10,GD_BUF[PUMP_TEMP_H_ALARM]%10);
            break;
		case PUMP_TEMP_L_ALARM:
            AT24CXX_Write(AddPumpTempLAlarmB1, &pack->data[0], 2);
            AT24CXX_Read(AddPumpTempLAlarmB1, (uint8_t *)&GD_BUF[PUMP_TEMP_L_ALARM], 2);
            log_i("GD_BUF[PUMP_TEMP_L_ALARM] set %d.%d `C", GD_BUF[PUMP_TEMP_L_ALARM]/10,GD_BUF[PUMP_TEMP_L_ALARM]%10);
            break;
		case CAVITY_TEMP_H_ALARM:
            AT24CXX_Write(AddCavityTempHAlarmB1, &pack->data[0], 2);
            AT24CXX_Read(AddCavityTempHAlarmB1, (uint8_t *)&GD_BUF[CAVITY_TEMP_H_ALARM], 2);
            log_i("GD_BUF[CAVITY_TEMP_H_ALARM] set %d.%d `C", GD_BUF[CAVITY_TEMP_H_ALARM]/10,GD_BUF[CAVITY_TEMP_H_ALARM]%10);
            break;
		case CAVITY_TEMP_L_ALARM:
            AT24CXX_Write(AddCavityTempLAlarmB1, &pack->data[0], 2);
            AT24CXX_Read(AddCavityTempLAlarmB1, (uint8_t *)&GD_BUF[CAVITY_TEMP_L_ALARM], 2);
            log_i("GD_BUF[CAVITY_TEMP_L_ALARM] set %d.%d `C", GD_BUF[CAVITY_TEMP_L_ALARM]/10,GD_BUF[CAVITY_TEMP_L_ALARM]%10); 
        break;			   
		case SEED_POWER_ENA:
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]) )
            {
                rt_memcpy(&GD_BUF[SEED_POWER_ENA], &pack->data[0], sizeof(GD_BUF[SEED_POWER_ENA]));
                if((GD_BUF[SEED_POWER_ENA] == 0) && (GD_BUF[SEED_LASER_ENERGY_MON] < 10))
                { 
                    GD_BUF[SEED_POWER_ENA] = 0;
                }
                else
                {
                    if(GD_BUF[SEED_POWER_ENA] == 0)
                    {
                        log_e("GD_BUF[SEED_POWER_ENA] is %d, more than 10",GD_BUF[SEED_LASER_ENERGY_MON]); 
                    }
                    GD_BUF[SEED_POWER_ENA] = 1;
                }
                AT24CXX_Write(AddSeedPower, &pack->data[0], 1);
                AT24CXX_Read(AddSeedPower, (uint8_t *)&GD_BUF[SEED_POWER_ENA], 1);
                log_i("GD_BUF[SEED_POWER_ENA] set %s", GD_BUF[SEED_POWER_ENA]?"on":"off");  
            }			 		
            break;
		case SEED_LASER_ENA:
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]))
            {
                rt_memcpy(&GD_BUF[SEED_LASER_ENA], &pack->data[0], sizeof(GD_BUF[SEED_LASER_ENA]));
                
                if((GD_BUF[SEED_LASER_ENA] == 0) && (GD_BUF[PUMP1_I_READ] <5) && (GD_BUF[PUMP2_I_READ] <5))
                {
                    GD_BUF[SEED_LASER_ENA] = 0;
                }
                else
                {
                    if(GD_BUF[SEED_LASER_ENA] == 0)
                    {
                        log_e("GD_BUF[SEED_LASER_ENA] is %d, GD_BUF[PUMP2_I_READ] is %d, more than 5",GD_BUF[PUMP1_I_READ],GD_BUF[PUMP2_I_READ]);
                    }
                    GD_BUF[SEED_LASER_ENA] = 1;
                }
                AT24CXX_Write(AddSeedLaser, &pack->data[0], 1);
                AT24CXX_Read(AddSeedLaser, (uint8_t *)&GD_BUF[SEED_LASER_ENA], 1);
                log_i("GD_BUF[SEED_LASER_ENA] set %s", GD_BUF[SEED_LASER_ENA]?"on":"off");  
            }	   					 		
            break;		   
		case SEED_LASER_ENERGY_CTRL:
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[PUMP1_I_READ] <5) && (GD_BUF[PUMP2_I_READ] <5))
            {
                rt_memcpy(&GD_BUF[SEED_LASER_ENERGY_CTRL], &pack->data[0], sizeof(GD_BUF[SEED_LASER_ENERGY_CTRL]));

                if(GD_BUF[SEED_LASER_ENERGY_CTRL] > 100)
                {
                    GD_BUF[SEED_LASER_ENERGY_CTRL] = 100;
                }
                else if(GD_BUF[SEED_LASER_ENERGY_CTRL] < 90)
                {
                    GD_BUF[SEED_LASER_ENERGY_CTRL] = 90;
                }
                AT24CXX_Write(AddSeedEnergy, (uint8_t *)&GD_BUF[SEED_LASER_ENERGY_CTRL], 1);
                AT24CXX_Read(AddSeedEnergy, (uint8_t *)&GD_BUF[SEED_LASER_ENERGY_CTRL], 1);
                log_i("GD_BUF[SEED_LASER_ENERGY_CTRL] set %d", GD_BUF[SEED_LASER_ENERGY_CTRL]);  
                ACTIVE_SEED_ENERGY_CTRL(GD_BUF[SEED_LASER_ENERGY_CTRL],Channel_E);  //设置种子源功率
            } 
            break;
		case SEED_TYPE:
            AT24CXX_Write(AddSeedType, &pack->data[0], 1);
            AT24CXX_Read(AddSeedType, (uint8_t *)&GD_BUF[SEED_TYPE], 1);
            log_i("GD_BUF[SEED_TYPE] set %s", GD_BUF[SEED_TYPE] == 2?"NPI":"TOPTICA");          
            break;
		case FQ_POWER_ENA:
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]) )			
            {   
                rt_memcpy(&GD_BUF[FQ_POWER_ENA], &pack->data[0], sizeof(GD_BUF[FQ_POWER_ENA]));
                if((GD_BUF[FQ_POWER_ENA] == 0) && (GD_BUF[SEED_LASER_ENERGY_MON] < 10))
                {
                    GD_BUF[FQ_POWER_ENA] = 0;
                }
                else
                {
                    if(GD_BUF[FQ_POWER_ENA] ==0)
                    {
                        log_e("GD_BUF[SEED_LASER_ENERGY_MON] is %d, more than 10",GD_BUF[SEED_LASER_ENERGY_MON]);
                    }
                    GD_BUF[FQ_POWER_ENA] = 1;
                }	
                AT24CXX_Write(AddFqPower, (uint8_t *)&GD_BUF[FQ_POWER_ENA], 1);
                AT24CXX_Read(AddFqPower, (uint8_t *)&GD_BUF[FQ_POWER_ENA], 1);
                log_i("GD_BUF[FQ_POWER_ENA] set %s", GD_BUF[FQ_POWER_ENA]?"on":"off");   
            }	
            break;
        //LD1-LD4电流不为0时，不执行Burst设置		
		case BURST_NUM:
            rt_memcpy(&GD_BUF[BURST_NUM], &pack->data[0], sizeof(GD_BUF[BURST_NUM]));
            if(GD_BUF[BURST_NUM] > 10)
            {
                GD_BUF[BURST_NUM] = 10;
            }
            else if(GD_BUF[BURST_NUM] < 1)
            {
                GD_BUF[BURST_NUM] = 1;
            }
            log_i("GD_BUF[BURST_NUM] set %d to fpga", GD_BUF[BURST_NUM]); 
            UserSetValueChaFlag[aom1BurstChaFlag] = true;						
            break;
		case FQ_FRE: 
            rt_memcpy(&GD_BUF[FQ_FRE], &pack->data[0], sizeof(GD_BUF[FQ_FRE]));
            if(GD_BUF[FQ_FRE] > 2000) //上限1000K，高了SQ响应不了
            {
                GD_BUF[FQ_FRE] = 2000;
            }
            else if(GD_BUF[FQ_FRE]<100)
            {
                GD_BUF[FQ_FRE] = 100;
            }
            
            // 要求AOM1的功率和burst的乘积大于400
            if((GD_BUF[BURST_NUM] * GD_BUF[FQ_FRE]) <= 400)
            {
                GD_BUF[BURST_NUM] = 400/GD_BUF[FQ_FRE];
                
                if(400%GD_BUF[FQ_FRE] > 0)
                    GD_BUF[BURST_NUM]++;
                log_i("Because GD_BUF[BURST_NUM] * GD_BUF[FQ_FRE] < 400, GD_BUF[BURST_NUM] set %d to fpga", GD_BUF[BURST_NUM]);
                UserSetValueChaFlag[aom1BurstChaFlag] = true;
            }
            log_i("GD_BUF[FQ_FRE] set %d to fpga", GD_BUF[FQ_FRE]); 
            UserSetValueChaFlag[aom1FreChaFlag] = true; 
            break;					
	    case FQ_RF_SCALE:
             if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[PUMP1_I_READ] <5) && (GD_BUF[PUMP2_I_READ] <5))
             {
                rt_memcpy(&GD_BUF[FQ_RF_SCALE], &pack->data[0], sizeof(GD_BUF[FQ_RF_SCALE]));
                if(GD_BUF[FQ_RF_SCALE] > 100)
                {
                    GD_BUF[FQ_RF_SCALE] = 100;
                }
                else if(GD_BUF[FQ_RF_SCALE] < 1)
                {
                    GD_BUF[FQ_RF_SCALE] = 0;
                }
                Aom1RfSet(GD_BUF[FQ_RF_SCALE], Channel_F);
                AT24CXX_Write(AddFqRf, (uint8_t *)&GD_BUF[FQ_RF_SCALE], 1);
                AT24CXX_Read(AddFqRf, (uint8_t *)&GD_BUF[FQ_RF_SCALE], 1);
                log_i("GD_BUF[FQ_RF_SCALE] set %d", GD_BUF[FQ_RF_SCALE]);   
             }
             break;			
		case FQ_DELAY:
            rt_memcpy(&GD_BUF[FQ_DELAY], &pack->data[0], sizeof(GD_BUF[FQ_DELAY]));
            if(GD_BUF[FQ_DELAY] > 250)
            {
                GD_BUF[FQ_DELAY] = 250;
            }
            log_i("GD_BUF[FQ_DELAY] set %d to fpga", GD_BUF[FQ_DELAY]); 
            UserSetValueChaFlag[aom1DelayChaFlag] = true;
            break;
		case SQ_POWER_ENA:
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]) )		
            {  
                AT24CXX_Write(AddSqPower, &pack->data[0], 1);
                AT24CXX_Read(AddSqPower, (uint8_t *)&GD_BUF[SQ_POWER_ENA], 1);
                log_i("GD_BUF[SQ_POWER_ENA] set %s", GD_BUF[SQ_POWER_ENA]?"on":"off");
            } 		
            break;	 
        case SQ_STANDBY_ENA:					    
            if((keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]) )			
            { 
                AT24CXX_Write(AddSqStandby, &pack->data[0], 1);
                AT24CXX_Read(AddSqStandby, (uint8_t *)&GD_BUF[SQ_STANDBY_ENA], 1);
                log_i("GD_BUF[SQ_STANDBY_ENA] set %s", GD_BUF[SQ_STANDBY_ENA]?"on":"off");
            }				 		
            break;	       
		case SQ_ATTEN_SEL:
            AT24CXX_Write(AddSqAttenSel, &pack->data[0], 1);
            AT24CXX_Read(AddSqAttenSel, (uint8_t *)&GD_BUF[SQ_ATTEN_SEL], 1);
            log_i("GD_BUF[SQ_ATTEN_SEL] set %s", GD_BUF[SQ_ATTEN_SEL]?"out_contrl":"int_contrl");		 		
            break;	 
        case MODULO_DIVIDER:
            rt_memcpy(&GD_BUF[MODULO_DIVIDER], &pack->data[0], sizeof(GD_BUF[MODULO_DIVIDER]));
            if(GD_BUF[MODULO_DIVIDER] > 65000)
            {
                GD_BUF[MODULO_DIVIDER] = 65000;
            }
            else if(GD_BUF[MODULO_DIVIDER] < 1)
            {
                GD_BUF[MODULO_DIVIDER] = 1;
            }
            log_i("GD_BUF[SQ_DIVIDER] set %d to fpga", GD_BUF[MODULO_DIVIDER]); 
            FPGA_Write(MODULO_DIVIDER_W, GD_BUF[MODULO_DIVIDER]); 
            GD_BUF[MODULO_DIVIDER] = FPGA_Read(MODULO_DIVIDER_R);
            log_i("GD_BUF[SQ_DIVIDER] fpga back %d", GD_BUF[MODULO_DIVIDER]); 
            AT24CXX_Write(AddModuloDividerB1, (uint8_t *)&GD_BUF[MODULO_DIVIDER], 1);
            AT24CXX_Read(AddModuloDividerB1, (uint8_t *)&GD_BUF[MODULO_DIVIDER], 1);
            log_i("GD_BUF[SQ_DIVIDER] set is %d ", GD_BUF[MODULO_DIVIDER]);
            break;
		case SQ_RF_SCALE:
            rt_memcpy(&GD_BUF[SQ_RF_SCALE], &pack->data[0], sizeof(GD_BUF[SQ_RF_SCALE]));
            if(GD_BUF[SQ_RF_SCALE] > 1000)
            {
                GD_BUF[SQ_RF_SCALE] = 1000;
            }
            else if(GD_BUF[SQ_RF_SCALE] < 1)
            {
                GD_BUF[SQ_RF_SCALE] = 0;
            }
            log_i("GD_BUF[SQ_RF_SCALE] set %d to fpga", GD_BUF[SQ_RF_SCALE]); 
            FPGA_Write(SQ_RF_SCALE_W, aom2RfSet(GD_BUF[SQ_RF_SCALE]) ); 
            //GD_BUF[SQ_RF_SCALE] = FPGA_Read(SQ_RF_SCALE_R);
            //log_i("GD_BUF[SQ_RF_SCALE] fpga back %d", GD_BUF[SQ_RF_SCALE]); 
            AT24CXX_Write(AddSqRfB1, (uint8_t *)&GD_BUF[SQ_RF_SCALE], 2);
            AT24CXX_Read(AddSqRfB1, (uint8_t *)&GD_BUF[SQ_RF_SCALE], 2);
            log_i("GD_BUF[SQ_RF_SCALE] set %d", GD_BUF[SQ_RF_SCALE]);
            break;
		case SQ_ADD_TIME:
            rt_memcpy(&GD_BUF[SQ_ADD_TIME], &pack->data[0], sizeof(GD_BUF[SQ_ADD_TIME]));
            if(GD_BUF[SQ_ADD_TIME] > 1000)
            {
                GD_BUF[SQ_ADD_TIME] = 1000;
            }
            else if(GD_BUF[SQ_ADD_TIME] < 1)
            {
                GD_BUF[SQ_ADD_TIME] = 0;
            }
            log_i("GD_BUF[SQ_DELAY_TIME] set %d to fpga", GD_BUF[SQ_ADD_TIME]);
            UserSetValueChaFlag[aom2DelayChaFlag] = true;						
            break;	
        case SHG_SET: 
            rt_memcpy(&GD_BUF[SHG_SET], &pack->data[0], sizeof(GD_BUF[SHG_SET]));
            if(GD_BUF[SHG_SET] == 0)
            {
                GD_BUF[SHG_SET] = 0;
            }
            else if(GD_BUF[SHG_SET] < 1600)
            {
                GD_BUF[SHG_SET] = 1600;
            }
            else if(GD_BUF[SHG_SET] > 3400)
            {
                GD_BUF[SHG_SET] = 3400;
            }
            AT24CXX_Write(AddShgTempSetB1, (uint8_t *)&GD_BUF[SHG_SET], 2);
            AT24CXX_Read(AddShgTempSetB1, (uint8_t *)&GD_BUF[SHG_SET], 2);
            STHG_TEMP_SET(GD_BUF[SHG_SET], Channel_G);
            log_i("GD_BUF[SHG_SET] set %d.%d `C", GD_BUF[SHG_SET]/100,GD_BUF[SHG_SET]%100);
            break; 
		case THG_SET: 
            rt_memcpy(&GD_BUF[THG_SET], &pack->data[0], sizeof(GD_BUF[THG_SET]));
            if(GD_BUF[THG_SET] == 0)
            {
                GD_BUF[THG_SET] = 0;
            }
            else if(GD_BUF[THG_SET] < 1600)
            {
                GD_BUF[THG_SET] = 1600;
            }
            else if(GD_BUF[THG_SET] > 3400)
            {
                GD_BUF[THG_SET] = 3400;
            }
            AT24CXX_Write(AddThgTempSetB1, (uint8_t *)&GD_BUF[THG_SET], 2);
            AT24CXX_Read(AddThgTempSetB1, (uint8_t *)&GD_BUF[THG_SET], 2);
            STHG_TEMP_SET(GD_BUF[THG_SET], Channel_H);
            log_i("GD_BUF[THG_SET] set %d.%d `C", GD_BUF[THG_SET]/100,GD_BUF[THG_SET]%100);
            break;
        //模式开关  LD1-LD4电流不为0时，不执行模式切换	
		case MODE_SW:
            if(((GD_BUF[PUMP1_I_READ] < 5) && (GD_BUF[PUMP2_I_READ] < 5)) || (ALARM_EN))
            {
                rt_memcpy(&GD_BUF[MODE_SW], &pack->data[0], sizeof(GD_BUF[MODE_SW]));
                log_i("GD_BUF[MODE_SW] set %s to fpga", GD_BUF[MODE_SW]?"on":"off"); 
                FPGA_Write(Mode_SW_W, GD_BUF[MODE_SW]); 
                GD_BUF[MODE_SW] = FPGA_Read(Mode_SW_R);
                log_i("GD_BUF[MODE_SW] fpga back %s",  GD_BUF[MODE_SW]?"on":"off");
                AT24CXX_Write(AddModeSw, &pack->data[0], 1);
                AT24CXX_Read(AddModeSw, (uint8_t *)&GD_BUF[MODE_SW], 1);
                log_i("GD_BUF[MODE_SW] set is %s ",  GD_BUF[MODE_SW]?"on":"off");
            }
            else
            {
                log_e("GD_BUF[PUMP1_I_READ] is %d, GD_BUF[PUMP2_I_READ] is %d, more than 5, Cannot change mode_sw",GD_BUF[PUMP1_I_READ],GD_BUF[PUMP2_I_READ]);
            }
            break;
        //出光模式
		case LASER_MODE: 
            rt_memcpy(&GD_BUF[LASER_MODE], &pack->data[0], sizeof(GD_BUF[LASER_MODE]));
            log_i("GD_BUF[LASER_MODE] set %d to fpga", GD_BUF[LASER_MODE]); 
            FPGA_Write(Mode_W, GD_BUF[LASER_MODE]);
            GD_BUF[LASER_MODE] = FPGA_Read(Mode_R);
            log_i("GD_BUF[LASER_MODE] fpga back %d", GD_BUF[LASER_MODE]); 
            AT24CXX_Write(AddLaserMode, &pack->data[0], 1);
            AT24CXX_Read(AddLaserMode, (uint8_t *)&GD_BUF[LASER_MODE], 1);
            log_i("GD_BUF[LASER_MODE] set is %d ", GD_BUF[LASER_MODE]);
            break;
        //外控模式
		case EXT_MODE:
            rt_memcpy(&GD_BUF[EXT_MODE], &pack->data[0], sizeof(GD_BUF[EXT_MODE]));
            log_i("GD_BUF[EXT_MODE] set %d to fpga", GD_BUF[EXT_MODE]); 
            FPGA_Write(EXT_MODE_W, GD_BUF[EXT_MODE]);
            GD_BUF[EXT_MODE] = FPGA_Read(EXT_MODE_R);
            log_i("GD_BUF[EXT_MODE] fpga back %d", GD_BUF[EXT_MODE]); 
            AT24CXX_Write(AddExtMode, &pack->data[0], 1);
            AT24CXX_Read(AddExtMode, (uint8_t *)&GD_BUF[EXT_MODE], 1);
            log_i("GD_BUF[EXT_MODE] set is %d ", GD_BUF[EXT_MODE]);
            break;
        //有效电平
        case EXT_MODE_ACTLEVEL:
            rt_memcpy(&GD_BUF[EXT_MODE_ACTLEVEL], &pack->data[0], sizeof(GD_BUF[EXT_MODE_ACTLEVEL]));
            log_i("GD_BUF[EXT_MODE_ACTLEVEL] set %d to fpga", GD_BUF[EXT_MODE_ACTLEVEL]); 
            FPGA_Write(EXT_MODE_ACTLEVEL_W, GD_BUF[EXT_MODE_ACTLEVEL]);
            GD_BUF[EXT_MODE_ACTLEVEL] = FPGA_Read(EXT_MODE_ACTLEVEL_R);
            log_i("GD_BUF[EXT_MODE_ACTLEVEL] fpga back %d", GD_BUF[EXT_MODE_ACTLEVEL]); 
            AT24CXX_Write(AddExtModeLevel, &pack->data[0], 1);
            AT24CXX_Read(AddExtModeLevel, (uint8_t *)&GD_BUF[EXT_MODE_ACTLEVEL], 1);
            log_i("GD_BUF[EXT_MODE_ACTLEVEL] set is %d ", GD_BUF[EXT_MODE_ACTLEVEL]);
            break;		
		//BURST延迟个数					
		case BURST_HOLDOFF:
            rt_memcpy(&GD_BUF[BURST_HOLDOFF], &pack->data[0], sizeof(GD_BUF[BURST_HOLDOFF]));

            if(GD_BUF[BURST_HOLDOFF] > 250)
            {
                GD_BUF[BURST_HOLDOFF] = 250;
            }
            else if(GD_BUF[BURST_HOLDOFF] < 1)
            {
                GD_BUF[BURST_HOLDOFF] = 0;
            }
            log_i("GD_BUF[BURST_HOLDOFF] set %d to fpga", GD_BUF[BURST_HOLDOFF]); 
            FPGA_Write(BURST_HOLDOFF_W, GD_BUF[BURST_HOLDOFF]); 
            GD_BUF[BURST_HOLDOFF] = FPGA_Read(BURST_HOLDOFF_R);
            log_i("GD_BUF[BURST_HOLDOFF] fpga back %d", GD_BUF[BURST_HOLDOFF]); 
            AT24CXX_Write(AddBurstHoldOff, &pack->data[0], 1);
            AT24CXX_Read(AddBurstHoldOff, (uint8_t *)&GD_BUF[BURST_HOLDOFF], 1);
            log_i("GD_BUF[BURST_HOLDOFF] set is %d ", GD_BUF[BURST_HOLDOFF]);
            break;	
        //BURST出光个数
		case BURST_LENGTH:
            rt_memcpy(&GD_BUF[BURST_LENGTH], &pack->data[0], sizeof(GD_BUF[BURST_LENGTH]));
            if(GD_BUF[BURST_LENGTH] > 65000)
            {
                GD_BUF[BURST_LENGTH] = 65000;
            }
            else if(GD_BUF[BURST_LENGTH] < 1)
            {
                GD_BUF[BURST_LENGTH] = 1;
            }
            log_i("GD_BUF[BURST_LENGTH] set %d to fpga", GD_BUF[BURST_LENGTH]); 
            FPGA_Write(BURST_LENGTH_W, GD_BUF[BURST_LENGTH]); 
            GD_BUF[BURST_LENGTH] = FPGA_Read(BURST_LENGTH_R);
            log_i("GD_BUF[BURST_LENGTH] fpga back %d", GD_BUF[BURST_LENGTH]); 
            AT24CXX_Write(AddBurstLengthB1, (uint8_t *)&GD_BUF[BURST_LENGTH], 2);
            AT24CXX_Read(AddBurstLengthB1, (uint8_t *)&GD_BUF[BURST_LENGTH], 2);
            log_i("GD_BUF[BURST_LENGTH] set %d", GD_BUF[BURST_LENGTH]);
            break;		
		case SQ_TYPE:
            AT24CXX_Write(AddSqType, &pack->data[0], 1);
            AT24CXX_Read(AddSqType, (uint8_t *)&GD_BUF[SQ_TYPE], 1);
            log_i("GD_BUF[SEED_TYPE] set %s", GD_BUF[SQ_TYPE] == 2?"SPY":"GH");          
            break;
		case LASER_TYPE:
            AT24CXX_Write(AddLaserType, &pack->data[0], 1);
            AT24CXX_Read(AddLaserType, (uint8_t *)&GD_BUF[LASER_TYPE], 1);
            log_i("GD_BUF[LASER_TYPE] set %s", GD_BUF[LASER_TYPE] == 1?"IR":GD_BUF[LASER_TYPE] == 2?"UV":"GR");      
            break;
		case LASER_POWSEL:
            AT24CXX_Write(AddPowerSel, &pack->data[0], 1);
            AT24CXX_Read(AddPowerSel, (uint8_t *)&GD_BUF[LASER_POWSEL], 1);
            log_i("GD_BUF[LASER_POWSEL] set %d",GD_BUF[LASER_POWSEL]);      
            break;   
        //激光器一键开关
	    case SWITCH:
            if( (keyState == 0) && (GD_BUF[ERR] == none) && (GD_BUF[STARTING]) )
            {
                AT24CXX_Write(AddSwitch, &pack->data[0], 1);
                AT24CXX_Read(AddSwitch, (uint8_t *)&GD_BUF[SWITCH], 1);
                log_i("GD_BUF[SWITCH] set %s",GD_BUF[SWITCH]?"on":"off");      
                laserSwitchFlag	= true;											
            }
            break;	
		case AIRPUMP_EN:
            AT24CXX_Write(AddAirPumpEna, &pack->data[0], 1);
            AT24CXX_Read(AddAirPumpEna, (uint8_t *)&GD_BUF[AIRPUMP_EN], 1);
            log_i("GD_BUF[AIRPUMP_EN] set %s",GD_BUF[AIRPUMP_EN]?"on":"off");  
            if(GD_BUF[AIRPUMP_EN] == 1)
            {
                FAN_EN_ON();
            }
            else  
            {
                FAN_EN_OFF(); 
            }
            break;										
		//没有使用	
		case AIRPUMP_FREQ: 
            rt_memcpy(&GD_BUF[AIRPUMP_FREQ], &pack->data[0], sizeof(GD_BUF[AIRPUMP_FREQ]));
            //Set_AirPumpFreq_pro();   
            break;  
		//没有使用	
		case AIRPUMP_DUTY: 
            rt_memcpy(&GD_BUF[AIRPUMP_DUTY], &pack->data[0], sizeof(GD_BUF[AIRPUMP_DUTY]));
            //Set_AirPumpDuty_pro();
            break;		
        //没搞懂是干嘛的
		case IR_PD_FACTOR:
            AT24CXX_Write(AddIrPdFactor, &pack->data[0], 1);
            AT24CXX_Read(AddIrPdFactor, (uint8_t *)&GD_BUF[IR_PD_FACTOR], 1);
            log_i("GD_BUF[IR_PD_FACTOR] set %d",GD_BUF[IR_PD_FACTOR]);      
            break;	
		case UV_PD_FACTOR:
            AT24CXX_Write(AddUvPdFactor, &pack->data[0], 1);
            AT24CXX_Read(AddUvPdFactor, (uint8_t *)&GD_BUF[UV_PD_FACTOR], 1);
            log_i("GD_BUF[UV_PD_FACTOR] set %d",GD_BUF[UV_PD_FACTOR]);   
            break;	
		case IR_A_MAX:
            rt_memcpy(&GD_BUF[IR_A_MAX], &pack->data[0], sizeof(GD_BUF[IR_A_MAX]));
            if(GD_BUF[IR_A_MAX] > 4095)
            {
                GD_BUF[IR_A_MAX] = 4095;
            }
            AT24CXX_Write(AddIrAdcMaxB1, (uint8_t *)&GD_BUF[IR_A_MAX], 2);
            AT24CXX_Read(AddIrAdcMaxB1, (uint8_t *)&GD_BUF[IR_A_MAX], 2);
            log_i("GD_BUF[OUTLASER_A_MAX] set %d",GD_BUF[IR_A_MAX]);   
            break;
		case IR_A_MIN:
            rt_memcpy(&GD_BUF[IR_A_MIN], &pack->data[0], sizeof(GD_BUF[IR_A_MIN]));
            if(GD_BUF[IR_A_MIN] > 4095)
            {
                GD_BUF[IR_A_MIN] = 4095;
            }
            AT24CXX_Write(AddIrAdcMinB1, (uint8_t *)&GD_BUF[IR_A_MIN], 2);
            AT24CXX_Read(AddIrAdcMinB1, (uint8_t *)&GD_BUF[IR_A_MIN], 2);
            log_i("GD_BUF[OUTLASER_A_MIN] set %d",GD_BUF[IR_A_MIN]);   
            break;
		case IR_P_MAX: 
            rt_memcpy(&GD_BUF[IR_P_MAX], &pack->data[0], sizeof(GD_BUF[IR_P_MAX]));
            if(GD_BUF[IR_P_MAX] > 65000)
            {
                GD_BUF[IR_P_MAX] = 65000;
            }
            AT24CXX_Write(AddIrPowerMaxB1, (uint8_t *)&GD_BUF[IR_P_MAX], 2);
            AT24CXX_Read(AddIrPowerMaxB1, (uint8_t *)&GD_BUF[IR_P_MAX], 2);
            log_i("GD_BUF[OUTLASER_P_MAX] set %d",GD_BUF[IR_P_MAX]);  
            break;
		case IR_P_MIN:
            rt_memcpy(&GD_BUF[IR_P_MIN], &pack->data[0], sizeof(GD_BUF[IR_P_MIN]));
            if(GD_BUF[IR_P_MIN] > 65000)
            {
                GD_BUF[IR_P_MIN] = 65000;
            }
            AT24CXX_Write(AddIrPowerMinB1, (uint8_t *)&GD_BUF[IR_P_MIN], 2);
            AT24CXX_Read(AddIrPowerMinB1, (uint8_t *)&GD_BUF[IR_P_MIN], 2);
            log_i("GD_BUF[OUTLASER_P_MIN] set %d",GD_BUF[IR_P_MIN]);  
            break;				
		case INIT_TIME:
            AT24CXX_Write(AddInitTime, &pack->data[0], 1);
            AT24CXX_Read(AddInitTime, (uint8_t *)&GD_BUF[INIT_TIME], 1);
            log_i("GD_BUF[INIT_TIME] set %d",GD_BUF[INIT_TIME]);   
            break;
		case ERASE_ERR_RECORD:
        {
            uint8_t zero = 0;
            rt_memcpy(&GD_BUF[ERASE_ERR_RECORD], &pack->data[0], sizeof(GD_BUF[ERASE_ERR_RECORD]));
            if(GD_BUF[ERASE_ERR_RECORD] == 0)
            {
                AT24CXX_Write(AddInitAlarmCnt, &zero, 1);
                AT24CXX_Write(AddSeedAlarmCnt, &zero, 1);
                AT24CXX_Write(AddSqAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump1HAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump1LAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump2HAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump2LAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump3HAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump3LAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump4HAlarmCnt, &zero, 1);
                AT24CXX_Write(AddPump4LAlarmCnt, &zero, 1);
                AT24CXX_Write(AddCavityHAlarmCnt, &zero, 1);
                AT24CXX_Write(AddCavityLAlarmCnt, &zero, 1);
                AT24CXX_Write(AddCavity2HAlarmCnt, &zero, 1);
                AT24CXX_Write(AddCavity2LAlarmCnt, &zero, 1);

                AT24CXX_Read(AddInitAlarmCnt, (uint8_t *)&GD_BUF[INIT_ALARM_CNT], 1);
                AT24CXX_Read(AddSeedAlarmCnt, (uint8_t *)&GD_BUF[SEED_ALARM_CNT], 1);
                AT24CXX_Read(AddSqAlarmCnt, (uint8_t *)&GD_BUF[SQ_ALARM_CNT], 1);
                AT24CXX_Read(AddPump1HAlarmCnt, (uint8_t *)&GD_BUF[PUMP1_H_ALARM_CNT], 1);
                AT24CXX_Read(AddPump1LAlarmCnt, (uint8_t *)&GD_BUF[PUMP1_L_ALARM_CNT], 1);
                AT24CXX_Read(AddPump2HAlarmCnt, (uint8_t *)&GD_BUF[PUMP2_H_ALARM_CNT], 1);
                AT24CXX_Read(AddPump2LAlarmCnt, (uint8_t *)&GD_BUF[PUMP2_L_ALARM_CNT], 1);
                AT24CXX_Read(AddPump3HAlarmCnt, (uint8_t *)&GD_BUF[PUMP3_H_ALARM_CNT], 1);
                AT24CXX_Read(AddPump3LAlarmCnt, (uint8_t *)&GD_BUF[PUMP3_L_ALARM_CNT], 1);
                AT24CXX_Read(AddPump4HAlarmCnt, (uint8_t *)&GD_BUF[PUMP4_H_ALARM_CNT], 1);
                AT24CXX_Read(AddPump4LAlarmCnt, (uint8_t *)&GD_BUF[PUMP4_L_ALARM_CNT], 1);
                AT24CXX_Read(AddCavityHAlarmCnt, (uint8_t *)&GD_BUF[CAVITY_H_ALARM_CNT], 1);
                AT24CXX_Read(AddCavityLAlarmCnt, (uint8_t *)&GD_BUF[CAVITY_L_ALARM_CNT], 1);
                AT24CXX_Read(AddCavity2HAlarmCnt, (uint8_t *)&GD_BUF[CAVITY2_H_ALARM_CNT], 1);
                AT24CXX_Read(AddCavity2LAlarmCnt, (uint8_t *)&GD_BUF[CAVITY2_L_ALARM_CNT], 1);
                log_i("GD_BUF[ERASE_ERR_RECORD] set zero"); 
            }
            break;
        }
							
		default:break;	
	}		
}

static void pc_handle_cb(uint8_t *d, uint16_t size)
{
    uart_cmd_t cmd_unpack;
	uart_protocol_unpack(&cmd_unpack, d, size);
    uart_cmd_t uart_cmd_send;

    uart_cmd_send.wr = cmd_unpack.wr;
    uart_cmd_send.cmd = cmd_unpack.cmd; 
    uart_cmd_send.data[0] = false;
    uart_cmd_send.data[1] = false;
    uart_cmd_send.data[2] = false;
    uart_cmd_send.data[3] = false;
    uart_cmd_send.size = 4;

    switch(cmd_unpack.cmd)
    {
        case TIME_SYNC:
        {     
            static uint8_t time_set = 0;
            extern rt_err_t set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second);
            extern rt_err_t set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t day);
            
            time_t now;
            struct tm *tm, tm_tmp;
            now = time(NULL);
            tm = gmtime_r(&now, &tm_tmp);
            
            struct tm tm_new;
            
            tm_new.tm_year = 2024;
            tm_new.tm_mon = cmd_unpack.wr;
            tm_new.tm_mday =cmd_unpack.data[0];
            tm_new.tm_hour = cmd_unpack.data[1];
            tm_new.tm_min =cmd_unpack.data[2];
            tm_new.tm_sec = cmd_unpack.data[3];
            
            if(abs(tm->tm_min - tm_new.tm_min) >= 5)
            {
                set_date(tm_new.tm_year, tm_new.tm_mon, tm_new.tm_mday);
                set_time(tm_new.tm_hour, tm_new.tm_min, tm_new.tm_sec);
                
                log_i("time set %02d-%02d-%02d %02d:%02d:%02d ",
                tm_new.tm_year, tm_new.tm_mon, tm_new.tm_mday,
                tm_new.tm_hour, tm_new.tm_min, tm_new.tm_sec);    
            }
        }
            break;
    	case MCU_VERS:
            GD_BUF[MCU_VERS]   = MCU_VERSION;
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[MCU_VERS], sizeof(GD_BUF[MCU_VERS]));
            uart_pc_protocol_send(&uart_cmd_send);
		    break;	
		case FPGA_VERS: 
			GD_BUF[FPGA_VERS]  = FPGA_Read(FPAG_VERS_R);
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[FPGA_VERS], sizeof(GD_BUF[FPGA_VERS]));
            uart_cmd_send.data[0] = GD_BUF[FPGA_VERS];
            uart_pc_protocol_send(&uart_cmd_send);
		    break;	 
		case SN1: 
			if(cmd_unpack.wr == CMD_WRITE)
			{
				run_cmd_set(SN1, &cmd_unpack);
			}
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SN1], sizeof(GD_BUF[SN1]));
            uart_pc_protocol_send(&uart_cmd_send);
			break;				
		case SN2: 
			if(cmd_unpack.wr == CMD_WRITE)
			{
			    run_cmd_set(SN2, &cmd_unpack);
			}
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SN2], sizeof(GD_BUF[SN2]));
            uart_pc_protocol_send(&uart_cmd_send);
			break;
		case SN3: 
			if(cmd_unpack.wr == CMD_WRITE)
			{
				run_cmd_set(SN3, &cmd_unpack);
			}
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SN3], sizeof(GD_BUF[SN3]));
            uart_pc_protocol_send(&uart_cmd_send);
			break;								
		case ERR:       
            //rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[ERR], sizeof(GD_BUF[ERR]));
            //uart_pc_protocol_send(&uart_cmd_send);
		    break;				
		case IR_TIME: 
			if(cmd_unpack.wr == CMD_WRITE)
			{
				run_cmd_set(IR_TIME, &cmd_unpack);
			}
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_TIME], sizeof(GD_BUF[IR_TIME]));
            uart_pc_protocol_send(&uart_cmd_send);
		    break; 
		case PUMP1_ENA: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP1_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_ENA], sizeof(GD_BUF[PUMP1_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
							
		case PUMP2_ENA: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP2_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_ENA], sizeof(GD_BUF[PUMP2_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;									
		case PUMP1_I_SET:     
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP1_I_SET, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_I_SET], sizeof(GD_BUF[PUMP1_I_SET]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;			
		case PUMP2_I_SET:     
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP2_I_SET, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_I_SET], sizeof(GD_BUF[PUMP1_I_SET]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
	    case PUMP1_I_MAX: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP1_I_MAX, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_I_MAX], sizeof(GD_BUF[PUMP1_I_MAX]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;					
		case PUMP2_I_MAX: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP2_I_MAX, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_I_MAX], sizeof(GD_BUF[PUMP2_I_MAX]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;			
		case PUMP1_I_READ: 
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_I_READ], sizeof(GD_BUF[PUMP1_I_READ]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		case PUMP2_I_READ: 
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_I_READ], sizeof(GD_BUF[PUMP2_I_READ]));
            uart_pc_protocol_send(&uart_cmd_send);
		    break;	
		case PUMP1_TEMP_READ: 
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_TEMP_READ], sizeof(GD_BUF[PUMP1_TEMP_READ]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;	
		case PUMP2_TEMP_READ: 
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_TEMP_READ], sizeof(GD_BUF[PUMP2_TEMP_READ]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;	
		case PUMP_TEMP_H_ALARM:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP_TEMP_H_ALARM, &cmd_unpack);
            }	
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP_TEMP_H_ALARM], sizeof(GD_BUF[PUMP_TEMP_H_ALARM]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		case PUMP_TEMP_L_ALARM:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(PUMP_TEMP_L_ALARM, &cmd_unpack);
            }	
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP_TEMP_L_ALARM], sizeof(GD_BUF[PUMP_TEMP_L_ALARM]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;									
		case CAVITY_TEMP_READ: 
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY_TEMP_READ], sizeof(GD_BUF[CAVITY_TEMP_READ]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;				
		case CAVITY_TEMP_H_ALARM:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(CAVITY_TEMP_H_ALARM, &cmd_unpack);
            }	
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY_TEMP_H_ALARM], sizeof(GD_BUF[CAVITY_TEMP_H_ALARM]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		case CAVITY_TEMP_L_ALARM:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(CAVITY_TEMP_L_ALARM, &cmd_unpack);
            }	
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY_TEMP_L_ALARM], sizeof(GD_BUF[CAVITY_TEMP_L_ALARM]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;							
		case SEED_POWER_ENA: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SEED_POWER_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_POWER_ENA], sizeof(GD_BUF[SEED_POWER_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		case SEED_LASER_ENA: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SEED_LASER_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_LASER_ENA], sizeof(GD_BUF[SEED_LASER_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;	
							
		case SEED_LASER_ENERGY_CTRL:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SEED_LASER_ENERGY_CTRL, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_LASER_ENERGY_CTRL], sizeof(GD_BUF[SEED_LASER_ENERGY_CTRL]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;	
		
		case SEED_LASER_ENERGY_MON:
            if(GD_BUF[SEED_LASER_ENA] == 1)
            {
                uart_cmd_send.data[0] = 100;//GD_BUF[SEED_LASER_ENERGY_MON];  // 0 - 100
            }
            else
            {
                rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_LASER_ENERGY_MON], sizeof(GD_BUF[SEED_LASER_ENERGY_MON]));
            }
            uart_pc_protocol_send(&uart_cmd_send);
            break;	
		case SEED_FRE:
            if(GD_BUF[SEED_LASER_ENA] == 1)
            {
                rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_FRE], sizeof(GD_BUF[SEED_FRE]));
            }
            else
            {
                uart_cmd_send.data[0] = 0;
            }
            uart_pc_protocol_send(&uart_cmd_send);
			break;
		case SEED_TYPE:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SEED_TYPE, &cmd_unpack);
            }														
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_TYPE], sizeof(GD_BUF[SEED_TYPE]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		case FQ_POWER_ENA: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(FQ_POWER_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[FQ_POWER_ENA], sizeof(GD_BUF[FQ_POWER_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;			
		case FQ_RF_SCALE:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(FQ_RF_SCALE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[FQ_RF_SCALE], sizeof(GD_BUF[FQ_RF_SCALE]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;		
		case FQ_DELAY:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(FQ_DELAY, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[FQ_DELAY], sizeof(GD_BUF[FQ_DELAY]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
							
		case FQ_FRE:  
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(FQ_FRE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[FQ_FRE], sizeof(GD_BUF[FQ_FRE]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;							
		case BURST_NUM:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(BURST_NUM, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[BURST_NUM], sizeof(GD_BUF[BURST_NUM]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		case SQ_POWER_ENA: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SQ_POWER_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_POWER_ENA], sizeof(GD_BUF[SQ_POWER_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;			
		case SQ_ADD_TIME: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SQ_ADD_TIME, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_ADD_TIME], sizeof(GD_BUF[SQ_ADD_TIME]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
							
		case SQ_RF_SCALE:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SQ_RF_SCALE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_RF_SCALE], sizeof(GD_BUF[SQ_RF_SCALE]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
							
	    case SQ_STANDBY_ENA:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SQ_STANDBY_ENA, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_STANDBY_ENA], sizeof(GD_BUF[SQ_STANDBY_ENA]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
							
		case SQ_ATTEN_SEL:	
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SQ_ATTEN_SEL, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_ATTEN_SEL], sizeof(GD_BUF[SQ_ATTEN_SEL]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;			
		case SQ_TYPE:	
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SQ_TYPE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_TYPE], sizeof(GD_BUF[SQ_TYPE])); 
            uart_pc_protocol_send(&uart_cmd_send);
            break;		                  
        case LASER_TYPE:	
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(LASER_TYPE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[LASER_TYPE], sizeof(GD_BUF[LASER_TYPE])); 
            uart_pc_protocol_send(&uart_cmd_send);
            break;
                     
		case LASER_POWSEL:	
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(LASER_POWSEL, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[LASER_POWSEL], sizeof(GD_BUF[LASER_POWSEL])); 
            uart_pc_protocol_send(&uart_cmd_send);
            break;       				            
		case LASER_MODE: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(LASER_MODE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[LASER_MODE], sizeof(GD_BUF[LASER_MODE]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;				
		case MODE_SW:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(MODE_SW, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[MODE_SW], sizeof(GD_BUF[MODE_SW]));
            uart_pc_protocol_send(&uart_cmd_send);
			break;			
	    case EXT_MODE:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(EXT_MODE, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[EXT_MODE], sizeof(GD_BUF[EXT_MODE]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;													
		case MODULO_DIVIDER:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(MODULO_DIVIDER, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[MODULO_DIVIDER], sizeof(GD_BUF[MODULO_DIVIDER]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
        case BURST_HOLDOFF:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(BURST_HOLDOFF, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[BURST_HOLDOFF], sizeof(GD_BUF[BURST_HOLDOFF]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;			
		case BURST_LENGTH:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(BURST_LENGTH, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[BURST_LENGTH], sizeof(GD_BUF[BURST_LENGTH]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;
		
		case EXT_MODE_ACTLEVEL :
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(EXT_MODE_ACTLEVEL, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[EXT_MODE_ACTLEVEL], sizeof(GD_BUF[EXT_MODE_ACTLEVEL]));
            uart_pc_protocol_send(&uart_cmd_send);
            break;					
		case SHG_SET:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SHG_SET, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SHG_SET], sizeof(GD_BUF[SHG_SET]));
            uart_pc_protocol_send(&uart_cmd_send);                            
            break;					
        case SHG_REAL:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SHG_REAL], sizeof(GD_BUF[SHG_REAL]));   
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	
		case THG_SET:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(THG_SET, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[THG_SET], sizeof(GD_BUF[THG_SET])); 
            uart_pc_protocol_send(&uart_cmd_send);  
            break;				
		case THG_REAL:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[THG_REAL], sizeof(GD_BUF[THG_REAL]));
            uart_pc_protocol_send(&uart_cmd_send);          
            break;		
		case IR_POWER:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_POWER], sizeof(GD_BUF[IR_POWER]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	
		case UV_POWER:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[UV_POWER], sizeof(GD_BUF[UV_POWER])); 
            uart_pc_protocol_send(&uart_cmd_send);  					
            break;
		case SEED_TIME: 
            if(cmd_unpack.wr==CMD_WRITE)
            {
                run_cmd_set(SEED_TIME, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_TIME], sizeof(GD_BUF[SEED_TIME]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;
							
		case UV_TIME: 
            if(cmd_unpack.wr==CMD_WRITE)
            {
                run_cmd_set(UV_TIME, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[UV_TIME], sizeof(GD_BUF[UV_TIME]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;
		
		case SerialNumA1: 	
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SerialNumA1, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SerialNumA1], sizeof(GD_BUF[SerialNumA1]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	
							
		case SerialNumB1:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SerialNumB1, &cmd_unpack);
            }	
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SerialNumB1], sizeof(GD_BUF[SerialNumB1]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	
							
		case SWITCH:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(SWITCH, &cmd_unpack);
            }	
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SWITCH], sizeof(GD_BUF[SWITCH]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	
							
		case AIRPUMP_EN:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(AIRPUMP_EN, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[AIRPUMP_EN], sizeof(GD_BUF[AIRPUMP_EN]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;					
							
		case AIRPUMP_FREQ: 
            if(cmd_unpack.wr == CMD_WRITE)   //write
            {
                run_cmd_set(AIRPUMP_FREQ, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[AIRPUMP_FREQ], sizeof(GD_BUF[AIRPUMP_FREQ]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;
		
		case AIRPUMP_DUTY: 
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(AIRPUMP_DUTY, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[AIRPUMP_DUTY], sizeof(GD_BUF[AIRPUMP_DUTY]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	
		
		case IR_PD_FACTOR:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(IR_PD_FACTOR, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_PD_FACTOR], sizeof(GD_BUF[IR_PD_FACTOR]));
            uart_pc_protocol_send(&uart_cmd_send);                              
            break;
							
		case UV_PD_FACTOR:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(UV_PD_FACTOR, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[UV_PD_FACTOR], sizeof(GD_BUF[UV_PD_FACTOR]));
            uart_pc_protocol_send(&uart_cmd_send);  
            break;	 
		case HM_RH:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[HM_RH], sizeof(GD_BUF[HM_RH]));
            uart_pc_protocol_send(&uart_cmd_send);  
			break;		 
		
		case HM_T:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[HM_T], sizeof(GD_BUF[HM_T]));
            uart_pc_protocol_send(&uart_cmd_send);          
			break;
		case IR_A:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_A], sizeof(GD_BUF[IR_A]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		
		case IR_A_MAX:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(IR_A_MAX, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_A_MAX], sizeof(GD_BUF[IR_A_MAX]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
							
		case IR_A_MIN:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(IR_A_MIN, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_A_MIN], sizeof(GD_BUF[IR_A_MIN]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
							
		case IR_P_MAX:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(IR_P_MAX, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_P_MAX], sizeof(GD_BUF[IR_P_MAX]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
							
		case IR_P_MIN:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(IR_P_MIN, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_P_MIN], sizeof(GD_BUF[IR_P_MIN]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
            
		case COLLIMATOR_IR:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[COLLIMATOR_IR], sizeof(GD_BUF[COLLIMATOR_IR]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
							
		case COLLIMATOR_IR_STATE:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[COLLIMATOR_IR_STATE], sizeof(GD_BUF[COLLIMATOR_IR_STATE]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		
		case STARTING:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[STARTING], sizeof(GD_BUF[STARTING])); 
            uart_pc_protocol_send(&uart_cmd_send);         
			break;
		
		case INIT_TIME:
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(INIT_TIME, &cmd_unpack);
            }
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[INIT_TIME], sizeof(GD_BUF[INIT_TIME]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
							
		case INIT_TIME_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[INIT_TIME_CNT], sizeof(GD_BUF[INIT_TIME_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		
		case INIT_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[INIT_ALARM_CNT], sizeof(GD_BUF[INIT_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;

		case ERR_DETAIL:     //获取详细报警信息
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[ERR_DETAIL], sizeof(GD_BUF[ERR_DETAIL]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		
		case ERASE_ERR_RECORD:     //擦除报警次数，数据进行清0
            if(cmd_unpack.wr == CMD_WRITE)
            {
                run_cmd_set(ERASE_ERR_RECORD, &cmd_unpack);
            }
            uart_cmd_send.data[0] = 1;
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
							
		 case SEED_ALARM_CNT: 
             rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SEED_ALARM_CNT], sizeof(GD_BUF[SEED_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
		 
		 case SQ_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[SQ_ALARM_CNT], sizeof(GD_BUF[SQ_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;

		 case PUMP1_H_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_H_ALARM_CNT], sizeof(GD_BUF[PUMP1_H_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP1_L_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP1_L_ALARM_CNT], sizeof(GD_BUF[PUMP1_L_ALARM_CNT]));
		    uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP2_H_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_H_ALARM_CNT], sizeof(GD_BUF[PUMP2_H_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP2_L_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP2_L_ALARM_CNT], sizeof(GD_BUF[PUMP2_L_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP3_H_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP3_H_ALARM_CNT], sizeof(GD_BUF[PUMP3_H_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP3_L_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP3_L_ALARM_CNT], sizeof(GD_BUF[PUMP3_L_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP4_H_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP4_H_ALARM_CNT], sizeof(GD_BUF[PUMP4_H_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case PUMP4_L_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[PUMP4_L_ALARM_CNT], sizeof(GD_BUF[PUMP4_L_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case CAVITY_H_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY_H_ALARM_CNT], sizeof(GD_BUF[CAVITY_H_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case CAVITY_L_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY_L_ALARM_CNT], sizeof(GD_BUF[CAVITY_L_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case CAVITY2_H_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY2_H_ALARM_CNT], sizeof(GD_BUF[CAVITY2_H_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case CAVITY2_L_ALARM_CNT:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY2_L_ALARM_CNT], sizeof(GD_BUF[CAVITY2_L_ALARM_CNT]));
            uart_pc_protocol_send(&uart_cmd_send); 
            break;
		 
		 case CAVITY2_TEMP_READ:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[CAVITY2_TEMP_READ], sizeof(GD_BUF[CAVITY2_TEMP_READ]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case IR_HM_RH:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_HM_RH], sizeof(GD_BUF[IR_HM_RH]));
            uart_pc_protocol_send(&uart_cmd_send); 
			break;
		 
		 case IR_HM_T:
            rt_memcpy(&uart_cmd_send.data[0], &GD_BUF[IR_HM_T], sizeof(GD_BUF[IR_HM_T])); 
            uart_pc_protocol_send(&uart_cmd_send); 
			break;	
		default:    
            break;            				
	}
}

static void uart_protocol_unpack(uart_cmd_t *pack, uint8_t* data, uint16_t size)
{
	pack->wr = data[1];
	pack->cmd = data[2];
	
	rt_memcpy(pack->data, &data[3], (size - 3));
}

void uart_pc_protocol_send(uart_cmd_t *pack)
{
    pack->send_buf[0] = 0xff;
	pack->send_buf[1] = pack->wr;
    pack->send_buf[2] = pack->cmd;

	rt_memcpy(&(pack->send_buf[3]), pack->data, pack->size);
	
	//upacker_pack(&msg_screen_packer, pack->send_buf, pack->size + 2);
}



