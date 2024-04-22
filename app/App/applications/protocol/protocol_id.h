/*
 * Copyright (c) 2019-2020, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-29     qq           the first version
 */

#ifndef __PROTOCOL_ID_H
#define __PROTOCOL_ID_H

#include <stdint.h>

#define GD_LEN				    154
extern uint32_t GD_BUF[GD_LEN];

#define UART_DATA_LEN   32
#define CAN_DATA_LEN    8

typedef struct
{
	uint8_t  wr;
	uint8_t cmd;
    uint8_t  data[UART_DATA_LEN]; 
	uint16_t size;
	uint8_t  send_buf[UART_DATA_LEN+3];
    
}uart_cmd_t;

typedef struct
{
	uint16_t dir;
	uint16_t cmd;
    uint16_t size;
    uint8_t  data[CAN_DATA_LEN+2];
    
}can_cmd_t;

// can id
typedef enum
{
    CAN_ID_MIANCTRL_TO_COOLCTRL = 0x10,
    CAN_ID_COOLCTRL_TO_MIANCTRL,
    
}can_id_t;


typedef enum
{
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_HEART = 0x0000,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_PUMP_TEMP,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_TEC_TEMP,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_BOX_TEMP,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_TEMP_HUM,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_PD,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_SHG_TEMP,
    PROTOCOL_CMD_MAINCTRL_TO_COOLCTRL_THG_TEMP,
}mainctrl_to_coolctrl_cmd; 


typedef enum
{
    PROTOCOL_CMD_COOLCTRL_TO_MAINCTRL_HEART = 0x0000,
    PROTOCOL_CMD_COOLCTRL_TO_MAINCTRL_PUMP_TEMP,
    PROTOCOL_CMD_COOLCTRL_TO_MAINCTRL_TEC_TEMP,
    PROTOCOL_CMD_COOLCTRL_TO_MAINCTRL_BOX_TEMP,
    PROTOCOL_CMD_COOLCTRL_TO_MAINCTRL_TEMP_HUM,
    PROTOCOL_CMD_COOLCTRL_TO_MAINCTRL_PD,
}coolctrl_to_mainctrl_cmd; 

#define CMD_READ											0x01        //与PC交互数据的读写位   
#define CMD_WRITE											0x02           

#define MCU_VERSION	            10606

#define aom1FreChaFlag    0                                             //aom1频率修改标志位
#define aom1BurstChaFlag  1                                             //aom1脉冲数修改标志位
#define aom1DelayChaFlag  2 											//aom1延时修改标志位
#define aom2DelayChaFlag  3												//aom2延时修改标志位

#define CMD_NUM											14              //与TEC板交互数据的数组长度

#define MCU_VERS									    0               //MCU软件版本，再#define 声中设置
#define FPGA_VERS									    1               //FPGA软件版本，再FPGA程序设置
#define SN1                       				     	2               //SN号1，可存4个字符
#define SN2                       					    3               //SN号2
#define SN3                      					    4               //SN号3
#define ERR                       				     	5               //告警标志
#define IR_TIME                  				     	6               //保存出激光时长
#define PUMP1_ENA                                       7               //泵浦1使能，1-ON,0-OFF
#define PUMP2_ENA                                       8 				//泵浦2使能，1-ON,0-OFF
#define PUMP3_ENA                                       9				//泵浦3使能，1-ON,0-OFF
#define PUMP4_ENA                                       10				//泵浦4使能，1-ON,0-OFF
#define PUMP1_I_SET                                     11              //泵浦1电流设置
#define PUMP2_I_SET                                     12				//泵浦2电流设置
#define PUMP3_I_SET                                     13				//泵浦3电流设置
#define PUMP4_I_SET                                     14				//泵浦4电流设置
#define PUMP1_I_MAX                                     15				//泵浦1电流上限
#define PUMP2_I_MAX                                     16				//泵浦2电流上限
#define PUMP3_I_MAX                                     17				//泵浦3电流上限
#define PUMP4_I_MAX                                     18				//泵浦4电流上限
#define PUMP1_I_READ                                    19				//泵浦1电流读取
#define PUMP2_I_READ                                    20				//泵浦2电流读取
#define PUMP3_I_READ                                    21				//泵浦3电流读取
#define PUMP4_I_READ                                    22				//泵浦4电流读取
#define PUMP1_TEMP_READ                                 23              //泵浦1温度读取
#define PUMP2_TEMP_READ                                 24				//泵浦2温度读取
#define PUMP3_TEMP_READ                                 25				//泵浦3温度读取
#define PUMP4_TEMP_READ                                 26				//泵浦4温度读取
#define PUMP_TEMP_H_ALARM                               27              //泵浦高温报警阈值
#define CAVITY_TEMP_READ                                28              //机壳温度值
#define CAVITY_TEMP_H_ALARM                             29              //机壳高温报警阈值
#define PUMP1_VOL_READ                                  30              //泵浦1电压读取-未使用
#define PUMP2_VOL_READ                                  31              //泵浦2电压读取-未使用
#define PUMP3_VOL_READ                                  32              //泵浦3电压读取-未使用
#define PUMP4_VOL_READ                                  33              //泵浦4电压读取-未使用
#define SEED_POWER_ENA                                  34              //种子源上电使能，1-ON,0-OFF
#define SEED_LASER_ENA                                  35              //种子源开关使能，1-ON,0-OFF
#define SEED_LASER_ENERGY_CTRL                          36              //种子源功率设置
#define SEED_LASER_ENERGY_MON                           37              //种子源功率读取
#define SEED_ERROR_READ                                 38              //种子源告警
#define SEED_FRE                                        39              //种子源频率
#define SQ_POWER_ENA                                    40              //空间Q上电使能，1-ON,0-OFF
#define SQ_STANDBY_ENA                                  41              //空间Q待机使能，1-待机,0-不待机
#define SQ_ADD_TIME	      	                            42              //AOM2偏置出光时间
#define SQ_RF_SCALE                                     43              //空间Q射频功率
#define FQ_POWER_ENA                                    44              //光纤Q上电使能，1-ON,0-OFF
#define FQ_FRE		                                    45              //光线Q频率
#define FQ_RF_SCALE                                     46              //光纤Q射频功率
#define FQ_DELAY                                        64              //AOM1延迟
#define BURST_NUM                                       47              //激光脉冲个数
#define LASER_MODE                                      48              //打标模式 0-内控，1-外控
#define MODE_SW                                         49              //模式使能 1-开，0-关
/********************************************未应用**************************************************/
#define RTC_DATE_SET                                    50              //RTC校准日期
#define RTC_DATE_REAL                                   51              //RTC实时日期
#define RTC_TIME_SET							        52              //RTC校准时间
#define RTC_TIME_REAL                                   53              //RTC实时时间
#define USER_LIMITDAY                                   54              //试用期
#define USER_LIMITDATE                                  55              //截止日期  
#define REMAIN_DAY                                      56              //剩余试用天数
#define USER_STATE                                      57              //使用状态
#define SerialNumA1                                     58              //原生ID1
#define SerialNumB1                                     59              //校验ID1
#define RTC_STATE                                       60              //RTC异常标志位
#define FACTORY_I                                       61              //出厂电流
#define INIT_FACTORY_SET                                62              //恢复出厂设置
#define SAVE_FACTORY_SET                                63              //保存出厂参数
/*****************************************************************************************************/
#define EXT_MODE                                        65              //0-GATE,1-BURST
#define MODULO_DIVIDER                                  66              //分频值
#define BURST_HOLDOFF                                   67              //BURST延迟个数
#define BURST_LENGTH                                    68              //BURST出光个数
#define EXT_MODE_ACTLEVEL                               69              //外控模式有效电平,0-低电平(下降沿)有效，1-高电平(上升沿)有效
#define SQ_ATTEN_SEL                                    70              //SQ射频控制,0-内控，1-外控
#define SHG_SET                                         71              //二倍频温度设置值
#define SHG_REAL                                        72              //二倍频温度实时值
#define THG_SET                                         73              //三倍频温度设置值
#define THG_REAL                                        74              //三倍频温度实时值
#define IR_POWER                                        75              //激光红外功率
#define UV_POWER                                        76              //激光紫外功率
#define SWITCH                                          77              //一键开/关机，1-开机，0-关机
#define SEED_TIME                                       78              //种子源出光计时
#define UV_TIME                                         79              //紫外激光计时
#define AIRPUMP_FREQ                                    80              //气泵频率
#define AIRPUMP_DUTY                                    81              //气泵占空比
#define IR_PD_FACTOR                                    82              //红外PD功率校正系数
#define UV_PD_FACTOR                                    83              //紫外PD功率校正系数
#define HM_RH                                           84              //紫外腔湿度
#define HM_T                                            85              //紫外腔温度
#define IR_A                                            86              //红外PD的ADC值             
#define IR_A_MAX                                        87              //红外功率90%时的ADC值        
#define IR_A_MIN                                        88              //红外功率10%时的ADC值
#define IR_P_MAX                                        89              //实测红外功率90%值
#define IR_P_MIN                                        90              //实测红外功率10%值
#define UV_A                                            91              //紫外PD的ADC值
#define UV_A_MAX                                        92              //紫外功率90%时的ADC值
#define UV_A_MIN                                        93              //紫外功率10%时的ADC值
#define UV_P_MAX                                        94              //实测紫外功率90%值
#define UV_P_MIN                                        95              //实测紫外功率10%值
#define PUMP_TEMP_L_ALARM                               96              //泵低温报警阈值
#define CAVITY_TEMP_L_ALARM                             97              //腔体低温报警阈值
#define COLLIMATOR_IR                                   98              //准直器红外PD的ADC值
#define COLLIMATOR_IR_STATE                             99              //准直器红外状态，1-ON,0-OFF
#define STARTING                                        100             //上电初始化完成标志位，1-yes,0-no
#define INIT_TIME                                       101             //给定初始化计时长
#define INIT_TIME_CNT                                   102             //初始化用时
#define INIT_ALARM_CNT                                  103             //初始化报警次数
#define ERR_DETAIL                                      104             //详细报警信息
#define ERASE_ERR_RECORD                                105             //擦除报警信息记录
#define SEED_ALARM_CNT                                  106             //种子源报警次数
#define SQ_ALARM_CNT                                    107             //空间Q报警次数
#define PUMP1_H_ALARM_CNT                               108             //泵1高温报警次数
#define PUMP1_L_ALARM_CNT                               109             //泵1低温报警次数
#define PUMP2_H_ALARM_CNT                               110             //泵2高温报警次数
#define PUMP2_L_ALARM_CNT                               111             //泵2低温报警次数
#define PUMP3_H_ALARM_CNT                               112             //泵3高温报警次数
#define PUMP3_L_ALARM_CNT                               113             //泵3低温报警次数
#define PUMP4_H_ALARM_CNT                               114             //泵4高温报警次数
#define PUMP4_L_ALARM_CNT                               115             //泵4低温报警次数
#define CAVITY_H_ALARM_CNT                              116             //腔体1处高温报警次数
#define CAVITY_L_ALARM_CNT                              117             //腔体1处低温报警次数
#define CAVITY2_H_ALARM_CNT                             118             //腔体2处高温报警次数
#define CAVITY2_L_ALARM_CNT                             119             //腔体2处低温报警次数
#define CAVITY2_TEMP_READ                               120             //腔体2处温度
#define IR_HM_RH                                        121             //红外腔湿度
#define IR_HM_T                                         122             //红外腔温度
#define AIRPUMP_EN                                      123             //气泵开关，1-ON,0-OFF
//#define PUMP5_ENA                                       124				//泵浦5使能，1-ON,0-OFF
//#define PUMP5_I_SET                                     125 			//泵浦5电流设置
//#define PUMP5_I_MAX                                     126				//泵浦5电流上限
//#define PUMP5_I_READ                                    127				//泵浦5电流读取
//#define PUMP5_TEMP_READ                                 128				//泵浦5温度读取
//#define PUMP5_H_ALARM_CNT                               129             //泵5高温报警次数
//#define PUMP5_L_ALARM_CNT                               130             //泵5低温报警次数
#define SEED_TYPE                                       131             //种子源型号
#define SQ_TYPE                                         132             //AOM2型号
/******************************************QQ  record******************************************************/
#define IR_HM_ALARM_ENA                                 133             //红外湿度监测使能
#define UV_HM_ALARM_ENA                                 134             //紫外湿度监测使能
#define IR_HM_LIMIT_SET                                 135             //红外湿度阈值设置
#define UV_HM_LIMIT_SET                                 136             //紫外湿度阈值设置
#define IR_HM_ALARM_CNT                                 137             //红外湿度报警次数
#define UV_HM_ALARM_CNT                                 138             //紫外湿度报警次数

#define LASER_TYPE                                      139
#define LASER_POWSEL                                    140
#define GL_POWER                                        141
#define GL_TIME                                         142
#define ELECTRIC_PD_TRI                                 144             //电路腔PD的ADC值触发值，小于该值触发
#define ELECTRIC_PD_TRI_CNT                             145             //出光PD报警次数
#define LASER_OUT_PD_TRI                                147             //出光PD的目前功率的百分比，小于该值触发
#define LASER_OUT_PD_TRI_CNT                            148             //出光PD报警次数
#define SECLASER_OUT_PD_TRI                             150             //次级出光PD的ADC值触发值，小于该值触发
#define SECLASER_OUT_PD_TRI_CNT                         151             //次级出光PD报警次数
#define ELECTRIC_PDA                                    152             //电路腔PDA的ADC值
#define ELECTRIC_PDB                                    153             //电路腔PDB的ADC值
#define TIME_SYNC                                       154             //时间同步
#define LOG_SYNC                                        155             //日志同步
//参数在EEPROM的存储地址
typedef enum
{
	AddEepromCheck           = 0,                   //EEPROM读写位
	AddSn1Byte1              = 1,                    
	AddSn1Byte2              = 2,
	AddSn1Byte3              = 3,
	AddSn1Byte4              = 4, 
	AddSn2Byte1              = 5,
	AddSn2Byte2              = 6,
	AddSn2Byte3              = 7,
	AddSn2Byte4              = 8, 
	AddSn3Byte1              = 9,
	AddSn3Byte2              = 10,
	AddSn3Byte3              = 11,
	AddSn3Byte4              = 12, 
	AddPump1Ena              = 13,
	AddPump2Ena              = 14,
	AddPump3Ena              = 15,
	AddPump4Ena              = 16,
	AddPump1ISetByte1        = 17,
	AddPump1ISetByte2        = 18,
	AddPump2ISetByte1        = 19,
	AddPump2ISetByte2        = 20,
	AddPump3ISetByte1        = 21,
	AddPump3ISetByte2        = 22,
	AddPump4ISetByte1        = 23,
	AddPump4ISetByte2        = 24,
	AddPump1IMaxByte1        = 25,
	AddPump1IMaxByte2        = 26,
	AddPump2IMaxByte1        = 27,
	AddPump2IMaxByte2        = 28,
	AddPump3IMaxByte1        = 29,
	AddPump3IMaxByte2        = 30,
	AddPump4IMaxByte1        = 31,
	AddPump4IMaxByte2        = 32,
	AddPumpTempHAlarmB1      = 33,
	AddPumpTempHAlarmB2      = 34,
	AddPumpTempLAlarmB1      = 35,
	AddPumpTempLAlarmB2      = 36,
	AddCavityTempHAlarmB1    = 37,
	AddCavityTempHAlarmB2    = 38,
    AddCavityTempLAlarmB1    = 39,
	AddCavityTempLAlarmB2    = 40,
	AddSeedPower             = 41,
	AddSeedLaser             = 42,
	AddSeedEnergy            = 43,
	AddSeedLaserTimeB1       = 44,
	AddSeedLaserTimeB2       = 45,
	AddSeedLaserTimeB3       = 46,
	AddFqPower               = 47,
	AddFqBurst               = 48,
    AddFqRf                  = 49,
    AddFqFreB1               = 50,
	AddFqFreB2               = 51,
    AddFqDelay               = 52,
    AddSqPower               = 53,
    AddSqStandby             = 54,
    AddSqRfB1                = 55,
	AddSqRfB2                = 56,
    AddSqAttenSel            = 57,  
    AddSqAddTimeB1           = 58,
	AddSqAddTimeB2           = 59,
    AddLaserMode             = 60,
	AddModeSw                = 61,
	AddExtMode               = 62,
	AddModuloDividerB1       = 63,
	AddModuloDividerB2       = 64,
	AddBurstHoldOff          = 65,
	AddBurstLengthB1         = 66,
	AddBurstLengthB2         = 67,
    AddExtModeLevel          = 68,
	AddSwitch                = 69,
	AddShgTempSetB1          = 70,
	AddShgTempSetB2          = 71,
	AddThgTempSetB1          = 72,
	AddThgTempSetB2          = 73,
	AddAirPumpFre            = 74,
	AddAipPumpDuty           = 75,
	AddIrLaserTimeB1         = 76,
	AddIrLaserTimeB2         = 77,
	AddIrLaserTimeB3         = 78,
	AddUvLaserTimeB1         = 79,
	AddUvLaserTimeB2         = 80,
	AddUvLaserTimeB3         = 81,
	AddIrPdFactor            = 82,
    AddUvPdFactor            = 83,
    AddIrAdcMaxB1            = 84,
    AddIrAdcMaxB2            = 85, 
    AddIrAdcMinB1            = 86,
    AddIrAdcMinB2            = 87,
	AddIrPowerMaxB1          = 88,
    AddIrPowerMaxB2          = 89,
   	AddIrPowerMinB1          = 90,
    AddIrPowerMinB2          = 91,
	AddUvAdcMaxB1            = 92,
    AddUvAdcMaxB2            = 93, 
    AddUvAdcMinB1            = 94,
    AddUvAdcMinB2            = 95,
	AddUvPowerMaxB1          = 96,
    AddUvPowerMaxB2          = 97,
   	AddUvPowerMinB1          = 98,
    AddUvPowerMinB2          = 99,
	AddInitTime              = 100,
	AddInitAlarmCnt          = 101,
	AddSeedAlarmCnt          = 102,
	AddSqAlarmCnt            = 103,
	AddPump1HAlarmCnt        = 104,
	AddPump1LAlarmCnt        = 105,
	AddPump2HAlarmCnt        = 106,
	AddPump2LAlarmCnt        = 107,
	AddPump3HAlarmCnt        = 108,
	AddPump3LAlarmCnt        = 109,
	AddPump4HAlarmCnt        = 110,
	AddPump4LAlarmCnt        = 111,
	AddCavityHAlarmCnt       = 112,
	AddCavityLAlarmCnt       = 113,  
	AddCavity2HAlarmCnt      = 114,
	AddCavity2LAlarmCnt      = 115,
	AddAirPumpEna            = 116,
	AddSeedType              = 117,
	AddSqType                = 118,
    /*qq record*/
    AddIrhmEna                = 126, 
    AddUvhmEna                = 127,
    AddIrLimitSet1            = 128,
    AddIrLimitSet2            = 129,
    AddUvLimitSet1            = 130,
    AddUvLimitSet2            = 131,
    AddirhmAlarmCnt           = 132,
    AdduvhmAlarmCnt           = 133,

    AddLaserType              = 134,
    AddPowerSel               = 135,
    AddGlLaserTimeB1          = 136,
    AddGlLaserTimeB2          = 137,
    AddGlLaserTimeB3          = 138,
    AddElectricPdTriB1        = 139,
    AddElectricPdTriB2        = 140,
    AddElectricPdTriCnt       = 141,
    AddLaserPdTriB1           = 142,
    AddLaserPdTriB2           = 143,
    AddLaserPdPdTriCnt        = 144,
    AddSecLaserPdTriB1        = 145,
    AddSecLaserPdTriB2        = 146,
    AddSecLaserPdTriCnt       = 147,  
} ADD;

#define YEAR                            0
#define MONTH                           1
#define DATE                            2
#define HOURS                           3
#define MINUTES                         4
#define SECONDS                         5
#define FOREVER                         255

#define PUMP1                           0
#define PUMP2                           1
#define PUMP3                           2
#define PUMP4                           3

#define HM1                             1
#define HM2                             2

#define TOPTICA                         1
#define NPI                             2

#define GH                              1
#define SPY                             2

#define EEPROMPASS                      111

//定义开关状态
typedef enum
{
	off                               = 0,	        //关
	on                                = 1          //开	
}switchState;
//定义报警标志位
typedef enum 
{
    none                              = 0,          //无警告
    seedWarning                       = 1,          //种子源告警
	sqWarning                         = 2,          //空间Q告警
	pump1Warning                      = 3,          //泵1告警 
	pump2Warning                      = 4,          //泵2告警
	pump3Warning                      = 5,          //泵3告警
	pump4Warning                      = 6,          //泵4告警
	cavityWarning                     = 7,          //腔体1处告警
	cavity2Warning                    = 8,          //腔体2处告警
	shgWarning                        = 9,          //二倍频告警
	thgWarning                        = 10,         //三倍频告警
	initialWarning                    = 11,         //初始化告警
    eiectriPdWarning                  = 12,         //电腔光泄露告警
    pump5Warning                      = 13,         //泵5告警
    loaserpdWarning                   = 14,         //出光功率
    secloaserpdWarning                = 15,         //次级出光功率
    error_lvl_1                       = 16,         //错误等级1告警
    error_lvl_2                       = 17,         //错误等级2告警
    error_lvl_3                       = 18,         //错误等级3告警
}WarningState;

//通讯位，提示报警明细
typedef enum
{
	seedBit                = 1,                     //种子源报警
	sqBit                  = 2,                     //空间Q报警
	pump1HBit              = 4,                     //泵1高温报警
	pump1LBit              = 8,                     //泵1低温报警
	pump2HBit              = 16,                    //泵2高温报警
	pump2LBit              = 32,                    //泵2低温报警
	pump3HBit              = 64,                    //泵3高温报警
	pump3LBit              = 128,                   //泵3低温报警
	pump4HBit              = 256,                   //泵4高温报警
	pump4LBit              = 512,                   //泵4低温报警
	cavityHBit             = 1024,                  //腔体1处高温报警
	cavityLBit             = 2048,                  //腔体1处低温报警
    initailBit             = 4096,                  //初始化报警
	cavity2HBit            = 8192,                  //腔体2处高温报警
	cavity2LBit            = 16384,                 //腔体2处低温报警
}ALARM_BIT;

#endif
