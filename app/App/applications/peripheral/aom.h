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

#define SQ_ATTEN_INT()                  rt_pin_write(SQ_ATTEN_SEL_PIN, PIN_LOW)      //射频功率内控   
#define SQ_ATTEN_EXT()                  rt_pin_write(SQ_ATTEN_SEL_PIN, PIN_HIGH)     //射频功率外控
#define SQ_POWER_ON()                   rt_pin_write(SQ_POWER_ENA_PIN, PIN_HIGH)     //空间Q上电    
#define SQ_POWER_OFF()                  rt_pin_write(SQ_POWER_ENA_PIN, PIN_LOW)      //空间Q掉电
#define SQ_STANDBY_ON()                 rt_pin_write(SQ_Standby_PIN, PIN_LOW)        //待机,无射频功率输出 
#define SQ_STANDBY_OFF()                rt_pin_write(SQ_Standby_PIN, PIN_HIGH)       //空间Q报警

void Aom_Init(void);  //aom1和aom2引脚初始化
int thread_aom_init(void);
void Aom1CtrlAct(rt_uint8_t aom1PowerState, rt_uint8_t seedType, rt_uint8_t seedLaserEnergy);  //aom1电源开/关条件判定
void Aom1SwAct(rt_uint8_t aom1PowerFlag);    //执行aom1电源开/关
void Aom1RfSet(rt_uint16_t Value,rt_uint8_t Channel); //设置aom1射频功率

void Aom2CtrlAct(rt_uint8_t aom2PowerState, rt_uint8_t aom2StandbyState, rt_uint8_t aom2RfInSelStateu8,rt_uint8_t aom2Type);//aom2电源，待机，射频输入选择
rt_uint16_t aom2RfSet(rt_uint16_t factor); //往FPGA写aom2射频功率值

#endif 

