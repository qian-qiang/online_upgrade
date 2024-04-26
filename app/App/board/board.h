/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-5      misonyo   first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include <stm32f4xx.h>
#include "drv_common.h"
#include "drv_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

//限幅
#define LIMIT( x,min,max ) ( (x) <= (min)  ? (min) : ( (x) >= (max) ? (max) : (x) ) )
//范围内判定
#define IsInSide( x,min,max ) ( (x) < (min)  ? (0) : ( (x) > (max) ? (0) : (1) ) )
//范围外判断
#define IsOutSide( x,min,max ) ( (x) < (min)  ? (1) : ( (x) > (max) ? (1) : (0) ) )


#define TOPTICA                         1
#define NPI                             2

#define GH                              1
#define SPY                             2

//alarm
#define ALARM_EN_PIN        GET_PIN(B,1)
#define ALARM_EN            rt_pin_read(ALARM_EN_PIN)

//beep
#define BEEP_PIN            GET_PIN(B,0)

//fan
#define FAN_PIN             GET_PIN(D,12)

//i2c1
#define I2C1_WP             GET_PIN(B,7)
#define I2C1_SCL            GET_PIN(B,8)
#define I2C1_SDA            GET_PIN(B,9)

//DAC7568
#define DAC_SCLK_PIN        GET_PIN(F,4) 
#define DAC_DIN_PIN         GET_PIN(F,5) 
#define DAC_SYNC_PIN        GET_PIN(F,6) 

//SEED 
#define SEED_POWER_ENA_PIN  GET_PIN(E,7)         
#define SEED_LASER_ENA_PIN  GET_PIN(E,8)
#define SEED_ALARM_PIN      GET_PIN(E,9)
#define SEED_ALARM          rt_pin_read(SEED_ALARM_PIN)           //种子源报警

//AOM
#define FQ_POWER_ENA_PIN    GET_PIN(E,10) 
#define SQ_POWER_ENA_PIN    GET_PIN(E,11) 
#define SQ_SWR_ALARM_PIN    GET_PIN(E,12) 
#define SQ_Standby_PIN      GET_PIN(E,13) 
#define SQ_FRE_ENA_PIN      GET_PIN(E,14) 
#define SQ_ATTEN_SEL_PIN    GET_PIN(E,15) 
#define SQ_SWR_ALARM        rt_pin_read(SQ_SWR_ALARM_PIN)        //种子源报警

//PIMP
#define PUMP1_ENA_PIN       GET_PIN(F,3)
#define PUMP2_ENA_PIN       GET_PIN(F,2)
#define PUMP3_ENA_PIN		GET_PIN(F,1)
#define PUMP4_ENA_PIN		GET_PIN(F,0)

//LED
#define LED_ERR_PIN         GET_PIN(G,8)
#define LED_RUN_PIN         GET_PIN(G,7)
#define LED_LASER_PIN	    GET_PIN(G,6)
#define ERR_LED_ON()    	rt_pin_write(LED_ERR_PIN, PIN_LOW)
#define ERR_LED_OFF()   	rt_pin_write(LED_ERR_PIN, PIN_HIGH)
#define RUN_LED_ON()        rt_pin_write(LED_RUN_PIN, PIN_LOW)
#define RUN_LED_OFF()   	rt_pin_write(LED_RUN_PIN, PIN_HIGH)
#define LASER_LED_ON()    	rt_pin_write(LED_LASER_PIN, PIN_LOW)
#define LASER_LED_OFF()   	rt_pin_write(LED_LASER_PIN, PIN_HIGH)

//STHG
#define SHT_ENA_PIN         GET_PIN(G,3)
#define THG_ENA_PIN         GET_PIN(G,4)

//FPGA
#define SPI2_CS_B           GET_PIN(B,12)
#define SPI2_CLK_B          GET_PIN(B,13)
#define SPI2_MISO_B         GET_PIN(B,14)
#define SPI2_MOSI_B         GET_PIN(B,15)

//ADC
#define MUXA_ADC_PIN        GET_PIN(C,0)
#define MUXB_ADC_PIN        GET_PIN(C,2)
#define MUXC_ADC_PIN        GET_PIN(C,3)
#define A_IN_SEL0_PIN       GET_PIN(F,9)         
#define A_IN_SEL1_PIN       GET_PIN(F,8)         
#define A_IN_SEL2_PIN       GET_PIN(F,7)         

//KEY
#define KEY_SW_PIN			GET_PIN(G,5)    			
#define KEY_IN 				rt_pin_read(KEY_SW_PIN)      

//UART1
#define UART1_TX			GET_PIN(A,9)    			
#define UART1_RX			GET_PIN(A,10)
#define UART1_EN			GET_PIN(A,8)

//SPI
#define SPI2_CS			    GET_PIN(B,12)    			
#define SPI2_SCK			GET_PIN(B,13)
#define SPI2_MISO			GET_PIN(B,14)
#define SPI2_MOSI			GET_PIN(B,15)

// LED
#define PIN_LED1                        GET_PIN(D,4)    //呼吸灯
#define PIN_LED2                        GET_PIN(D,5)    //PC通信灯
#define PIN_LED3                        GET_PIN(D,6)    //
#define PIN_LED4                        GET_PIN(D,7)

#define STM32_FLASH_START_ADRESS     ((uint32_t)0x08000000)
#define STM32_FLASH_SIZE             (512 * 1024)
#define STM32_FLASH_END_ADDRESS      ((uint32_t)(STM32_FLASH_START_ADRESS + STM32_FLASH_SIZE))

#define STM32_SRAM_SIZE           128
#define STM32_SRAM_END            (0x20000000 + STM32_SRAM_SIZE * 1024)

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN      (&__bss_end)
#endif

#define HEAP_END        STM32_SRAM_END

void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif

#endif

