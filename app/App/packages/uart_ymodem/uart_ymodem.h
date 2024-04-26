/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-24     qianqiang    the first version
 */

#ifndef __UART_YMODEM_H__
#define __UART_YMODEM_H__

#include <rtthread.h>

#ifndef RT_UART_YMODEM_DEVICE_SIZE
#define RT_UART_YMODEM_DEVICE_SIZE   1100
#endif

#ifndef RT_UART_YMODEM_PACK_SIZE_TX    
#define RT_UART_YMODEM_PACK_SIZE_TX    6
#endif


struct rt_uart_ymodem_rx_fifo
{
    /* software fifo */
    rt_uint8_t *buffer;

    rt_uint16_t put_index, get_index;

    rt_bool_t is_full;
};


struct rt_uart_ymodem_device
{
    struct rt_device          parent;
    
    const struct rt_uart_ymodem_ops *ops;
    
    void *uart_ymodem_rx;
    
    rt_uint32_t channel;
};
typedef struct rt_uart_ymodem_device rt_uart_ymodem_t;

/**
 * uart operators
 */
struct rt_uart_ymodem_ops
{
    int (*puts)(struct rt_uart_ymodem_device *uart_ymodem,
                                const void       *buffer,
                                rt_size_t         size);
};

void rt_uart_ymodem_rx(struct rt_uart_ymodem_device *uart_ymodem, const void  *buffer, rt_size_t size);

rt_err_t rt_hw_uart_ymodem_register(struct rt_uart_ymodem_device *uart_ymodem,
                                   const char              *name,
                                   rt_uint32_t              flag,
                                   void                    *data);

#endif
