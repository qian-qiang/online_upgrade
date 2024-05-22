/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-5-22      qq           first version
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
#include "easyflash.h"
#include "no_bug.h"
#include "serial_pc.h"
#include "fpga.h"
#include "fal.h"
#include <dfs_fs.h>
#include <ulog_file.h>
#include <dfs_posix.h> 

#define HEART_LED    PIN_LED1

const char product_tab[32]    = {"main_board"};          	                /**<  产品名称   */
const char project_tab[32]    = {"main_board_app"};                         /**<  工程名称   */
char versions_tab[16]   = {"1.0.240425"};                                   /**<  版本信息   */
const char hard_board_tab[16] = {"1.0.0"};                                  /**<  硬件信息   */
const char username_tab[16]   = {"qianqiang"};                              /**<  作者信息   */

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
        year %=100;//留取年代分后两位
        sprintf(versions_tab,"1.0.%02d%02d%02d",year,month,day);
        printf("version is %10s\r\n",versions_tab);
    } else {
        printf("Failed to parse compilation date and time.\n");
    }
}

#define FS_PARTITION_NAME              "filesystem"
void mount_littlefs (void)
{
    struct rt_device *mtd_dev = RT_NULL;
    mtd_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
    
    if (!mtd_dev)
    {
        LOG_E("Can't create a mtd device on '%s' partition.", FS_PARTITION_NAME);
    }
    else
    {
        /* 挂载 littlefs */
        if (dfs_mount(FS_PARTITION_NAME, "/", "lfs", 0, 0) == 0)
        {
            LOG_I("Filesystem initialized!");
        }
        else
        {
            /* 格式化文件系统 */
            dfs_mkfs("lfs", FS_PARTITION_NAME);
            /* 挂载 littlefs */
            if (dfs_mount("filesystem", "/", "lfs", 0, 0) == 0)
            {
                LOG_I("Filesystem initialized!");
            }
            else
            {
                LOG_E("Failed to initialize filesystem!");
            }
        }
    }
}

void board_init(void)
{
    fal_init();
    fpga_spi2_init();
    easyflash_init();
}

void thread_init(void)
{
    int  ret = 0;
    
    ret = serial_pc_init();
    if(ret != 0)
    {
        log_e("thread serial pc init failed");   
    } 
    mount_littlefs();
    ulog_file_backend_init();
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
