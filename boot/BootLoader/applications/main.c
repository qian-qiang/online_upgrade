/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-4-18      qianqiang    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <fal.h>
#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
#include <shell.h>
#endif

#define DBG_ENABLE
#define DBG_SECTION_NAME                    "main"

#ifdef RT_MAIN_DEBUG
#define DBG_LEVEL                           DBG_LOG
#else
#define DBG_LEVEL                           DBG_INFO
#endif

#define DBG_COLOR
#include <rtdbg.h>
#include <rt_fota.h>

/* defined the LED0 pin: PB1 */
#define LED0_PIN    GET_PIN(C, 13)


const char boot_log_buf[] = 
"\r\n\
                                     ,----,     \r\n\
         ,---._  ,-.----.          ,/   .`|     \r\n\
       .-- -.' | |    /  |       ,`   .'  :     \r\n\
       |    |   :|   :    |    ;    ;     /     \r\n\
       :    ;   ||   |  .| : .'___,/    ,'      \r\n\
       :        |.   :  |: | |    :     |       \r\n\
       |    :   :|   |   | : ;    |.';  ;       \r\n\
       :         |   : .   / `----'  |  |       \r\n\
       |    ;   |;   | |`-'      '   :  ;       \r\n\
   ___ l         |   | ;         |   |  '       \r\n\
 /    /|    J   ::   ' |         '   :  |       \r\n\
/  ../  `..-    ,:   : :         ;   |.'        \r\n\
|    |         ; |   | :         '---'          \r\n\
 |    |      ,'  `---'.|                        \r\n\
  ---....--'      `---`                         \r\n";

void rt_fota_print_log(void)
{   
    LOG_RAW("%s", boot_log_buf);    
	LOG_RAW("2024-4-22 Copyright by Radiation @ qianqiang \r\n");
    LOG_RAW("Port to RT-Thread full version by datasheep \r\n");
	LOG_RAW("Version: %s build %s\r\n\r\n", RT_FOTA_SW_VERSION, __DATE__);
}


int main(void)
{
    #if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
        finsh_set_prompt("rt-qq />");
    #endif
    extern void rt_fota_init(void);
    rt_fota_init();

    return RT_EOK;
}
