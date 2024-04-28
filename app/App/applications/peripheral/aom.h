#ifndef __AOM_H
#define __AOM_H	

#include <rtthread.h>
#include <board.h>
#include <stdbool.h>
#include "cbm128s085.h"
#include "protocol_id.h"
#include "error.h"

typedef struct{
    rt_bool_t aom1PowerFlag;
    rt_uint8_t aom2_type;
    rt_uint8_t aom2StandbyState;
    rt_uint8_t aom2PowerState;
    rt_uint8_t aom2RfInSelState;
    rt_uint16_t aom2PcSetPower;
    rt_uint16_t aom2FpgaSetPower;
}aom_t;

#define FQ_POWER_ON()                   rt_pin_write(FQ_POWER_ENA_PIN, PIN_HIGH)        
#define FQ_POWER_OFF()                  rt_pin_write(FQ_POWER_ENA_PIN, PIN_LOW)    

#define ReadFQPowerPinState             (rt_pin_read(FQ_POWER_ENA_PIN))  

#define SQ_ATTEN_INT()                  rt_pin_write(SQ_ATTEN_SEL_PIN, PIN_LOW)      //��Ƶ�����ڿ�   
#define SQ_ATTEN_EXT()                  rt_pin_write(SQ_ATTEN_SEL_PIN, PIN_HIGH)     //��Ƶ�������
#define SQ_POWER_ON()                   rt_pin_write(SQ_POWER_ENA_PIN, PIN_HIGH)     //�ռ�Q�ϵ�    
#define SQ_POWER_OFF()                  rt_pin_write(SQ_POWER_ENA_PIN, PIN_LOW)      //�ռ�Q����
#define SQ_STANDBY_ON()                 rt_pin_write(SQ_Standby_PIN, PIN_LOW)        //����,����Ƶ������� 
#define SQ_STANDBY_OFF()                rt_pin_write(SQ_Standby_PIN, PIN_HIGH)       //�ռ�Q����

void Aom_Init(void);  //aom1��aom2���ų�ʼ��
int thread_aom_init(void);
void Aom1CtrlAct(rt_uint8_t aom1PowerState, rt_uint8_t seedType, rt_uint8_t seedLaserEnergy);  //aom1��Դ��/�������ж�
void Aom1SwAct(rt_uint8_t aom1PowerFlag);    //ִ��aom1��Դ��/��
void Aom1RfSet(rt_uint16_t Value,rt_uint8_t Channel); //����aom1��Ƶ����

void Aom2CtrlAct(rt_uint8_t aom2PowerState, rt_uint8_t aom2StandbyState, rt_uint8_t aom2RfInSelStateu8,rt_uint8_t aom2Type);//aom2��Դ����������Ƶ����ѡ��
rt_uint16_t aom2RfSet(rt_uint16_t factor); //��FPGAдaom2��Ƶ����ֵ

#endif 

