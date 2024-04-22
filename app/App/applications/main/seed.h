#ifndef __SEED_H
#define __SEED_H	
 
#include <rtthread.h>
#include <board.h>
#include <stdbool.h>
#include "dac7568.h"
#include "cbm128s085.h"
#include "protocol_id.h"
#include "error.h"

typedef struct{
    rt_bool_t seedType;
    rt_bool_t seedPowerFlag;
    rt_bool_t seedLaserFlag;
    rt_uint8_t pumpRealIoff;
    rt_uint8_t Seed_PowerUp_Delay_cnt;
    rt_uint8_t readSeedEnergy;
    rt_uint16_t writeSeedEnergy;
}seed_t;

#define PUMPREALIOFF                    5
#define SEED_POWER_ON()                 rt_pin_write(SEED_POWER_ENA_PIN, PIN_HIGH)         //种子源上电
#define SEED_POWER_OFF()                rt_pin_write(SEED_POWER_ENA_PIN, PIN_LOW)         //种子源掉电
#define SEED_LASER_ON()                 rt_pin_write(SEED_LASER_ENA_PIN, PIN_HIGH)         //种子源上电
#define SEED_LASER_OFF()                rt_pin_write(SEED_LASER_ENA_PIN, PIN_LOW)         //种子源掉电

#define ReadSeedLaserPinState           (rt_pin_read(GET_PIN(E,8)))  
	
void Seed_Init(void);  //种子源初始化

rt_uint8_t ACTIVE_SEED_ENERGY_READ(rt_uint16_t arrVal, rt_uint8_t seedType); //种子源激光功率读取
void ACTIVE_SEED_ENERGY_CTRL(rt_uint16_t Value,rt_uint8_t Channel); //设置种子源激光功率
void SeedCtrlAct(rt_uint8_t seedPowerState, rt_uint8_t seedLaserState, rt_uint8_t seedLaserEnergy,rt_uint8_t seedType, rt_uint16_t Pump1RealI,rt_uint16_t Pump2RealI, rt_uint16_t Pump3RealI, rt_uint16_t Pump4RealI); //种子源控制
void SeedSwAct(rt_uint8_t seedPowerFlag, rt_uint8_t seedLaserFlag); //种子源电源和激光控制
int thread_seed_init(void);
#endif  




