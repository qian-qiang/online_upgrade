/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-24     qianqiang    the first version
 */

#ifndef __DRV_UART_YMODEM_H__
#define __DRV_UART_YMODEM_H__

#include <rtthread.h>

typedef enum
{
    UART_CMD_FILE_CMD_MASTER = 0,
    UART_CMD_FILE_CMD_SLAVE,
    UART_CMD_FILE_FILE_MASTER,
    UART_CMD_FILE_FILE_SLAVE,
    
}uart_cmd_file_t;

void drv_uart_ymodem_pc_to_mcu(const void *buffer, rt_size_t size);

#endif
