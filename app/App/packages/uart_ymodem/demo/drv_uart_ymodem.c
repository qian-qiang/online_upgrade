/*
 * Copyright (c) 2019-2020, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-24     qianqiang    the first version
 */

#include "board.h"
#include "thread_uart.h"
#include "uart_ymodem.h"
#include "drv_uart_ymodem.h"

//#define DRV_DEBUG
#define LOG_TAG             "drv.uart_ymodem"
#include <drv_log.h>

#define UART_ID_FPGA_TO_SC_FILE   (0x30)

struct rt_uart_ymodem_device uart_ymodem_device;

static rt_err_t uart_file_send(uint32_t uart_id, uart_cmd_file_t cmd, uint8_t *buffer, uint8_t size);

static rt_err_t uart_file_send(uint32_t uart_id, uart_cmd_file_t cmd, uint8_t *buffer, uint8_t size)
{
    uint8_t *pdata = rt_calloc(1, 8);
    
    if(0 == size)
    {
        log_e("uart file send size can't be zero!");
        goto __exit;
    }
    
    *pdata = cmd;
    rt_memcpy(pdata + UART_FILE_CMD_SIZE, buffer, size);

    uart_send(can_id, pdata, size + UART_FILE_CMD_SIZE);
    
    rt_free(pdata);
    
    return RT_EOK;
    
__exit:
    
    rt_free(pdata);
    
    return -RT_ERROR;
}


static int rt_hw_uart_ymodem_send(struct rt_uart_ymodem_device *uart_ymodem,
                                 const void       *buffer,
                                 rt_size_t         size)
{
    if(UART_ID_FPGA_TO_SC_FILE == can_ymodem->channel)
    {
        uart_file_send(uart_ymodem->channel, UART_CMD_FILE_FILE_SLAVE, (uint8_t*)buffer, size);
    }

    return RT_EOK;
}    

void drv_uart_ymodem_recv(uint32_t ch, const void *buffer, rt_size_t size)
{
    if(UART_ID_FPGA_TO_SC_FILE == ch)
    {
        rt_uart_ymodem_rx(&uart_ymodem_device, buffer, size);
    }
}

static const struct rt_uart_ymodem_ops mainboard_uart_ymodem_ops =
{
    .puts = rt_hw_uart_ymodem_send,
};

int rt_hw_uart_ymodem_init(void)
{
    rt_err_t result = 0;
    
    uart_ymodem_device.ops = &mainboard_uart_ymodem_ops;
    
    uart_ymodem_device.channel = UART_ID_FPGA_TO_SC_FILE;
    
    result = rt_hw_uart_ymodem_register(&uart_ymodem_device, "uart_ymodem", RT_DEVICE_FLAG_RDWR, RT_NULL);
    
    return result;
}
INIT_DEVICE_EXPORT(rt_hw_uart_ymodem_init);
