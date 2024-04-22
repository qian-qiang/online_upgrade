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

#define CMD_READ											0x01        //��PC�������ݵĶ�дλ   
#define CMD_WRITE											0x02           

#define MCU_VERSION	            10606

#define aom1FreChaFlag    0                                             //aom1Ƶ���޸ı�־λ
#define aom1BurstChaFlag  1                                             //aom1�������޸ı�־λ
#define aom1DelayChaFlag  2 											//aom1��ʱ�޸ı�־λ
#define aom2DelayChaFlag  3												//aom2��ʱ�޸ı�־λ

#define CMD_NUM											14              //��TEC�彻�����ݵ����鳤��

#define MCU_VERS									    0               //MCU����汾����#define ��������
#define FPGA_VERS									    1               //FPGA����汾����FPGA��������
#define SN1                       				     	2               //SN��1���ɴ�4���ַ�
#define SN2                       					    3               //SN��2
#define SN3                      					    4               //SN��3
#define ERR                       				     	5               //�澯��־
#define IR_TIME                  				     	6               //���������ʱ��
#define PUMP1_ENA                                       7               //����1ʹ�ܣ�1-ON,0-OFF
#define PUMP2_ENA                                       8 				//����2ʹ�ܣ�1-ON,0-OFF
#define PUMP3_ENA                                       9				//����3ʹ�ܣ�1-ON,0-OFF
#define PUMP4_ENA                                       10				//����4ʹ�ܣ�1-ON,0-OFF
#define PUMP1_I_SET                                     11              //����1��������
#define PUMP2_I_SET                                     12				//����2��������
#define PUMP3_I_SET                                     13				//����3��������
#define PUMP4_I_SET                                     14				//����4��������
#define PUMP1_I_MAX                                     15				//����1��������
#define PUMP2_I_MAX                                     16				//����2��������
#define PUMP3_I_MAX                                     17				//����3��������
#define PUMP4_I_MAX                                     18				//����4��������
#define PUMP1_I_READ                                    19				//����1������ȡ
#define PUMP2_I_READ                                    20				//����2������ȡ
#define PUMP3_I_READ                                    21				//����3������ȡ
#define PUMP4_I_READ                                    22				//����4������ȡ
#define PUMP1_TEMP_READ                                 23              //����1�¶ȶ�ȡ
#define PUMP2_TEMP_READ                                 24				//����2�¶ȶ�ȡ
#define PUMP3_TEMP_READ                                 25				//����3�¶ȶ�ȡ
#define PUMP4_TEMP_READ                                 26				//����4�¶ȶ�ȡ
#define PUMP_TEMP_H_ALARM                               27              //���ָ��±�����ֵ
#define CAVITY_TEMP_READ                                28              //�����¶�ֵ
#define CAVITY_TEMP_H_ALARM                             29              //���Ǹ��±�����ֵ
#define PUMP1_VOL_READ                                  30              //����1��ѹ��ȡ-δʹ��
#define PUMP2_VOL_READ                                  31              //����2��ѹ��ȡ-δʹ��
#define PUMP3_VOL_READ                                  32              //����3��ѹ��ȡ-δʹ��
#define PUMP4_VOL_READ                                  33              //����4��ѹ��ȡ-δʹ��
#define SEED_POWER_ENA                                  34              //����Դ�ϵ�ʹ�ܣ�1-ON,0-OFF
#define SEED_LASER_ENA                                  35              //����Դ����ʹ�ܣ�1-ON,0-OFF
#define SEED_LASER_ENERGY_CTRL                          36              //����Դ��������
#define SEED_LASER_ENERGY_MON                           37              //����Դ���ʶ�ȡ
#define SEED_ERROR_READ                                 38              //����Դ�澯
#define SEED_FRE                                        39              //����ԴƵ��
#define SQ_POWER_ENA                                    40              //�ռ�Q�ϵ�ʹ�ܣ�1-ON,0-OFF
#define SQ_STANDBY_ENA                                  41              //�ռ�Q����ʹ�ܣ�1-����,0-������
#define SQ_ADD_TIME	      	                            42              //AOM2ƫ�ó���ʱ��
#define SQ_RF_SCALE                                     43              //�ռ�Q��Ƶ����
#define FQ_POWER_ENA                                    44              //����Q�ϵ�ʹ�ܣ�1-ON,0-OFF
#define FQ_FRE		                                    45              //����QƵ��
#define FQ_RF_SCALE                                     46              //����Q��Ƶ����
#define FQ_DELAY                                        64              //AOM1�ӳ�
#define BURST_NUM                                       47              //�����������
#define LASER_MODE                                      48              //���ģʽ 0-�ڿأ�1-���
#define MODE_SW                                         49              //ģʽʹ�� 1-����0-��
/********************************************δӦ��**************************************************/
#define RTC_DATE_SET                                    50              //RTCУ׼����
#define RTC_DATE_REAL                                   51              //RTCʵʱ����
#define RTC_TIME_SET							        52              //RTCУ׼ʱ��
#define RTC_TIME_REAL                                   53              //RTCʵʱʱ��
#define USER_LIMITDAY                                   54              //������
#define USER_LIMITDATE                                  55              //��ֹ����  
#define REMAIN_DAY                                      56              //ʣ����������
#define USER_STATE                                      57              //ʹ��״̬
#define SerialNumA1                                     58              //ԭ��ID1
#define SerialNumB1                                     59              //У��ID1
#define RTC_STATE                                       60              //RTC�쳣��־λ
#define FACTORY_I                                       61              //��������
#define INIT_FACTORY_SET                                62              //�ָ���������
#define SAVE_FACTORY_SET                                63              //�����������
/*****************************************************************************************************/
#define EXT_MODE                                        65              //0-GATE,1-BURST
#define MODULO_DIVIDER                                  66              //��Ƶֵ
#define BURST_HOLDOFF                                   67              //BURST�ӳٸ���
#define BURST_LENGTH                                    68              //BURST�������
#define EXT_MODE_ACTLEVEL                               69              //���ģʽ��Ч��ƽ,0-�͵�ƽ(�½���)��Ч��1-�ߵ�ƽ(������)��Ч
#define SQ_ATTEN_SEL                                    70              //SQ��Ƶ����,0-�ڿأ�1-���
#define SHG_SET                                         71              //����Ƶ�¶�����ֵ
#define SHG_REAL                                        72              //����Ƶ�¶�ʵʱֵ
#define THG_SET                                         73              //����Ƶ�¶�����ֵ
#define THG_REAL                                        74              //����Ƶ�¶�ʵʱֵ
#define IR_POWER                                        75              //������⹦��
#define UV_POWER                                        76              //�������⹦��
#define SWITCH                                          77              //һ����/�ػ���1-������0-�ػ�
#define SEED_TIME                                       78              //����Դ�����ʱ
#define UV_TIME                                         79              //���⼤���ʱ
#define AIRPUMP_FREQ                                    80              //����Ƶ��
#define AIRPUMP_DUTY                                    81              //����ռ�ձ�
#define IR_PD_FACTOR                                    82              //����PD����У��ϵ��
#define UV_PD_FACTOR                                    83              //����PD����У��ϵ��
#define HM_RH                                           84              //����ǻʪ��
#define HM_T                                            85              //����ǻ�¶�
#define IR_A                                            86              //����PD��ADCֵ             
#define IR_A_MAX                                        87              //���⹦��90%ʱ��ADCֵ        
#define IR_A_MIN                                        88              //���⹦��10%ʱ��ADCֵ
#define IR_P_MAX                                        89              //ʵ����⹦��90%ֵ
#define IR_P_MIN                                        90              //ʵ����⹦��10%ֵ
#define UV_A                                            91              //����PD��ADCֵ
#define UV_A_MAX                                        92              //���⹦��90%ʱ��ADCֵ
#define UV_A_MIN                                        93              //���⹦��10%ʱ��ADCֵ
#define UV_P_MAX                                        94              //ʵ�����⹦��90%ֵ
#define UV_P_MIN                                        95              //ʵ�����⹦��10%ֵ
#define PUMP_TEMP_L_ALARM                               96              //�õ��±�����ֵ
#define CAVITY_TEMP_L_ALARM                             97              //ǻ����±�����ֵ
#define COLLIMATOR_IR                                   98              //׼ֱ������PD��ADCֵ
#define COLLIMATOR_IR_STATE                             99              //׼ֱ������״̬��1-ON,0-OFF
#define STARTING                                        100             //�ϵ��ʼ����ɱ�־λ��1-yes,0-no
#define INIT_TIME                                       101             //������ʼ����ʱ��
#define INIT_TIME_CNT                                   102             //��ʼ����ʱ
#define INIT_ALARM_CNT                                  103             //��ʼ����������
#define ERR_DETAIL                                      104             //��ϸ������Ϣ
#define ERASE_ERR_RECORD                                105             //����������Ϣ��¼
#define SEED_ALARM_CNT                                  106             //����Դ��������
#define SQ_ALARM_CNT                                    107             //�ռ�Q��������
#define PUMP1_H_ALARM_CNT                               108             //��1���±�������
#define PUMP1_L_ALARM_CNT                               109             //��1���±�������
#define PUMP2_H_ALARM_CNT                               110             //��2���±�������
#define PUMP2_L_ALARM_CNT                               111             //��2���±�������
#define PUMP3_H_ALARM_CNT                               112             //��3���±�������
#define PUMP3_L_ALARM_CNT                               113             //��3���±�������
#define PUMP4_H_ALARM_CNT                               114             //��4���±�������
#define PUMP4_L_ALARM_CNT                               115             //��4���±�������
#define CAVITY_H_ALARM_CNT                              116             //ǻ��1�����±�������
#define CAVITY_L_ALARM_CNT                              117             //ǻ��1�����±�������
#define CAVITY2_H_ALARM_CNT                             118             //ǻ��2�����±�������
#define CAVITY2_L_ALARM_CNT                             119             //ǻ��2�����±�������
#define CAVITY2_TEMP_READ                               120             //ǻ��2���¶�
#define IR_HM_RH                                        121             //����ǻʪ��
#define IR_HM_T                                         122             //����ǻ�¶�
#define AIRPUMP_EN                                      123             //���ÿ��أ�1-ON,0-OFF
//#define PUMP5_ENA                                       124				//����5ʹ�ܣ�1-ON,0-OFF
//#define PUMP5_I_SET                                     125 			//����5��������
//#define PUMP5_I_MAX                                     126				//����5��������
//#define PUMP5_I_READ                                    127				//����5������ȡ
//#define PUMP5_TEMP_READ                                 128				//����5�¶ȶ�ȡ
//#define PUMP5_H_ALARM_CNT                               129             //��5���±�������
//#define PUMP5_L_ALARM_CNT                               130             //��5���±�������
#define SEED_TYPE                                       131             //����Դ�ͺ�
#define SQ_TYPE                                         132             //AOM2�ͺ�
/******************************************QQ  record******************************************************/
#define IR_HM_ALARM_ENA                                 133             //����ʪ�ȼ��ʹ��
#define UV_HM_ALARM_ENA                                 134             //����ʪ�ȼ��ʹ��
#define IR_HM_LIMIT_SET                                 135             //����ʪ����ֵ����
#define UV_HM_LIMIT_SET                                 136             //����ʪ����ֵ����
#define IR_HM_ALARM_CNT                                 137             //����ʪ�ȱ�������
#define UV_HM_ALARM_CNT                                 138             //����ʪ�ȱ�������

#define LASER_TYPE                                      139
#define LASER_POWSEL                                    140
#define GL_POWER                                        141
#define GL_TIME                                         142
#define ELECTRIC_PD_TRI                                 144             //��·ǻPD��ADCֵ����ֵ��С�ڸ�ֵ����
#define ELECTRIC_PD_TRI_CNT                             145             //����PD��������
#define LASER_OUT_PD_TRI                                147             //����PD��Ŀǰ���ʵİٷֱȣ�С�ڸ�ֵ����
#define LASER_OUT_PD_TRI_CNT                            148             //����PD��������
#define SECLASER_OUT_PD_TRI                             150             //�μ�����PD��ADCֵ����ֵ��С�ڸ�ֵ����
#define SECLASER_OUT_PD_TRI_CNT                         151             //�μ�����PD��������
#define ELECTRIC_PDA                                    152             //��·ǻPDA��ADCֵ
#define ELECTRIC_PDB                                    153             //��·ǻPDB��ADCֵ
#define TIME_SYNC                                       154             //ʱ��ͬ��
#define LOG_SYNC                                        155             //��־ͬ��
//������EEPROM�Ĵ洢��ַ
typedef enum
{
	AddEepromCheck           = 0,                   //EEPROM��дλ
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

//���忪��״̬
typedef enum
{
	off                               = 0,	        //��
	on                                = 1          //��	
}switchState;
//���屨����־λ
typedef enum 
{
    none                              = 0,          //�޾���
    seedWarning                       = 1,          //����Դ�澯
	sqWarning                         = 2,          //�ռ�Q�澯
	pump1Warning                      = 3,          //��1�澯 
	pump2Warning                      = 4,          //��2�澯
	pump3Warning                      = 5,          //��3�澯
	pump4Warning                      = 6,          //��4�澯
	cavityWarning                     = 7,          //ǻ��1���澯
	cavity2Warning                    = 8,          //ǻ��2���澯
	shgWarning                        = 9,          //����Ƶ�澯
	thgWarning                        = 10,         //����Ƶ�澯
	initialWarning                    = 11,         //��ʼ���澯
    eiectriPdWarning                  = 12,         //��ǻ��й¶�澯
    pump5Warning                      = 13,         //��5�澯
    loaserpdWarning                   = 14,         //���⹦��
    secloaserpdWarning                = 15,         //�μ����⹦��
    error_lvl_1                       = 16,         //����ȼ�1�澯
    error_lvl_2                       = 17,         //����ȼ�2�澯
    error_lvl_3                       = 18,         //����ȼ�3�澯
}WarningState;

//ͨѶλ����ʾ������ϸ
typedef enum
{
	seedBit                = 1,                     //����Դ����
	sqBit                  = 2,                     //�ռ�Q����
	pump1HBit              = 4,                     //��1���±���
	pump1LBit              = 8,                     //��1���±���
	pump2HBit              = 16,                    //��2���±���
	pump2LBit              = 32,                    //��2���±���
	pump3HBit              = 64,                    //��3���±���
	pump3LBit              = 128,                   //��3���±���
	pump4HBit              = 256,                   //��4���±���
	pump4LBit              = 512,                   //��4���±���
	cavityHBit             = 1024,                  //ǻ��1�����±���
	cavityLBit             = 2048,                  //ǻ��1�����±���
    initailBit             = 4096,                  //��ʼ������
	cavity2HBit            = 8192,                  //ǻ��2�����±���
	cavity2LBit            = 16384,                 //ǻ��2�����±���
}ALARM_BIT;

#endif
