#ifndef __ADC_H
#define __ADC_H	
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "protocol_id.h"
#include "pump.h"
#include "ShgThg.h"
#include "pd.h"
#include "seed.h"

#define ADCA_DEV_CHANNEL_10    10           /* ADC 通道 */
#define ADCB_DEV_CHANNEL_12    12           /* ADC 通道 */
#define ADCC_DEV_CHANNEL_13    13           /* ADC 通道 */

#define CD74_NUM               3            /*CD74个数*/
#define CD74_USE_CHANNEL       8            /*每个CD74通道*/
typedef struct{
    rt_bool_t s1_pin;
    rt_bool_t s2_pin;
    rt_bool_t s3_pin;
    rt_uint16_t adcArr[CD74_NUM][CD74_USE_CHANNEL];
}cd74_t;

rt_err_t cd74xx_adc_init(void); 				
rt_uint16_t Get_Adc_Average2(rt_uint8_t ch,rt_uint8_t times);
rt_uint16_t Get_Adc2(rt_uint8_t ch);
rt_uint16_t Get_Average(rt_uint16_t arr[],rt_uint8_t arr_len);
void SW_AD_Channel(rt_uint8_t Channel);
int cd74_thread_init(void);
#endif 















