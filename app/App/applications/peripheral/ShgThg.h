#ifndef _ShgThg_H
#define _ShgThg_H

#include "dac7568.h"
#include "cbm128s085.h"

#define SHG_ENA_ON()    				rt_pin_write(SHT_ENA_PIN, PIN_HIGH)
#define SHG_ENA_OFF()   				rt_pin_write(SHT_ENA_PIN, PIN_LOW)
#define THG_ENA_ON()    				rt_pin_write(THG_ENA_PIN, PIN_HIGH)
#define THG_ENA_OFF()   				rt_pin_write(THG_ENA_PIN, PIN_LOW)

void STHG_Init(void);
rt_uint16_t STHG_TEMP_READ(rt_uint16_t Arrval);
void STHG_TEMP_SET(rt_uint32_t value,rt_uint8_t Channel);

#endif



