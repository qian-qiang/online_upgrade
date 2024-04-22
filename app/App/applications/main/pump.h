#ifndef _PUMP_H
#define _PUMP_H

#include "cbm128s085.h"
#include "protocol_id.h"
#include <rtthread.h>
#include <board.h>
#include <stdbool.h>
#include "error.h"

#define PUMP_NUM 2
#define PUMP1_ID 0
#define PUMP2_ID 1

#define PUMP1_ENA_ON()                  rt_pin_write(PUMP1_ENA_PIN, PIN_LOW)          //��1ʹ��
#define PUMP1_ENA_OFF()                 rt_pin_write(PUMP1_ENA_PIN, PIN_HIGH)           //��1ʧ��
#define PUMP2_ENA_ON()                  rt_pin_write(PUMP2_ENA_PIN, PIN_LOW)          //��2ʹ��
#define PUMP2_ENA_OFF()                 rt_pin_write(PUMP2_ENA_PIN, PIN_HIGH)          //��2ʧ��
#define PUMP3_ENA_ON()                  rt_pin_write(PUMP3_ENA_PIN, PIN_LOW)          //��3ʹ��
#define PUMP3_ENA_OFF()                 rt_pin_write(PUMP3_ENA_PIN, PIN_HIGH)          //��3ʧ��
#define PUMP4_ENA_ON()                  rt_pin_write(PUMP4_ENA_PIN, PIN_LOW)          //��4ʹ��
#define PUMP4_ENA_OFF()                 rt_pin_write(PUMP4_ENA_PIN, PIN_HIGH)          //��4ʧ��

typedef union{
    struct{
        rt_uint16_t pump_id;
        rt_uint16_t pump_t;
    }temp;
    rt_uint32_t pump_mb;
}pump_temp;

typedef struct{
    pump_temp pump_tem[PUMP_NUM];
}pump;

void PUMP_Init(void);  //�ÿ������ų�ʼ��
int thread_pump_init(void);
void ACTIVE_PUMP_I_SET(rt_uint8_t pumpSwFlag, rt_uint16_t Pump_I_Set,rt_uint8_t Channel); //���Ʊÿ���
void PUMP_I_SET_ON(rt_uint16_t Pump_I_Set,rt_uint8_t Channel);  //�򿪱õ���
void PUMP_I_SET_OFF(rt_uint16_t Pump_I_Set,rt_uint8_t Channel); //�ضϱõ���
rt_uint16_t PUMP_I_READ(rt_uint16_t Arrval);                    //��ȡ�õ���
void pump_temp_get(rt_uint8_t id, rt_uint16_t temp);
#endif



