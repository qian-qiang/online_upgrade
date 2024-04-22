/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 */
#define LOG_TAG      "main"
#define LOG_LVL      LOG_LVL_DBG

#include "ulog.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include "error.h"
#include "at24c08.h"
#include "no_bug.h"
#include "serial_pc.h"
#include "cd74xx.h"
#include "task.h"
#include "fpga.h"
#include "funtimer.h"
#include "fan.h"
#include "fal.h"

#define HEART_LED    PIN_LED1

const char product_tab[32]    = {"main_board"};          	        /**<  ��Ʒ����   */
const char project_tab[32]    = {"main_board_app"};                         /**<  ��������   */
char versions_tab[16]   = {"1.0.240429"};                                   /**<  �汾��Ϣ   */
const char hard_board_tab[16] = {"1.0.0"};                                  /**<  Ӳ����Ϣ   */
const char username_tab[16]   = {"qianqiang"};                              /**<  ������Ϣ   */

int parseDate(const char *dateStr, int *year, int *month, int *day) {
    const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    char monthStr[4];
    int i;

    if (sscanf(dateStr, "%s %d %d", monthStr, day, year) != 3) {
        return 0;
    }

    for (i = 0; i < 12; i++) {
        if (strcmp(monthStr, monthNames[i]) == 0) {
            *month = i + 1;
            return 1;
        }
    }

    return 0;
}

int parseTime(const char *timeStr, int *hour, int *minute, int *second) {
    if (sscanf(timeStr, "%d:%d:%d", hour, minute, second) != 3) {
        return 0;
    }

    return 1;
}

static void get_version()
{
    const char *compileDate = __DATE__;
    const char *compileTime = __TIME__;
    int year, month, day, hour, minute, second;
    if (parseDate(compileDate, &year, &month, &day) && parseTime(compileTime, &hour, &minute, &second)) {
        printf("Compilation date: %d-%02d-%02d\n", year, month, day);
        printf("Compilation time: %02d:%02d:%02d\n", hour, minute, second);
        year %=100;//��ȡ����ֺ���λ
        sprintf(versions_tab,"1.0.%02d%02d%02d",year,month,day);
        printf("version is %10s\r\n",versions_tab);
    } else {
        printf("Failed to parse compilation date and time.\n");
    }
}


void board_init(void)
{
    rt_pin_mode(ALARM_EN_PIN, PIN_MODE_INPUT_PULLUP);
    fal_init();
    AT24CXX_Init();
    cd74xx_adc_init();
    fpga_spi2_init();
    Fan_Init();
}

void thread_init(void)
{
    int  ret = 0;
    
    ret = serial_pc_init();
    if(ret != 0)
    {
        log_e("thread serial pc init failed");   
    } 
    
    ret = error_init();
    if(ret != 0)
    {
        log_e("thread can init failed");   
    } 
    
    ret = timer_init();
    if(ret != 0)
    {
        log_e("thread can init failed");   
    } 
    thread_seed_init();
    thread_aom_init();
    thread_pump_init();
    task_init();
    cd74_thread_init();
}

int main(void)
{
    int count = 1;

    get_version();
    no_bug();
    board_init();
    thread_init();
    rt_pin_mode(HEART_LED, PIN_MODE_OUTPUT);
    
    while (count++)
    {
        rt_pin_write(HEART_LED, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(HEART_LED, PIN_LOW);
        rt_thread_mdelay(500);
    }
    
    return RT_EOK;
}

/**
 * Function    ota_app_vtor_reconfig
 * Description Set Vector Table base location to the start addr of app(RT_APP_PART_ADDR).
*/
static int ota_app_vtor_reconfig(void)
{
    #define NVIC_VTOR_MASK   0x3FFFFF80
    /* Set the Vector Table base location by user application firmware definition */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(ota_app_vtor_reconfig);