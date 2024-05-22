/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-31     qq           the first version
 */

#define LOG_TAG      "serail.pc"
#define LOG_LVL      LOG_LVL_DBG

#include <rtthread.h>
#include <board.h>
#include "rtdbg.h"
#include "serial_pc.h"
#include "serial_unpack.h"
#include "drv_uart_ymodem.h"

#define UART_NAME_SCREEN        "uart1"      /* 串口设备名称 */
#define PIN_UART_LED            PIN_LED2   

uint32_t GD_BUF[GD_LEN]={0};
uint16_t uart_mode;

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

#define PC_SERIAL_CONFIG_DEFAULT           \
{                                          \
    BAUD_RATE_19200, /* 19200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_RB_BUFSZ, /* Buffer size */  \
    0                                      \
}
struct serial_configure config = PC_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

static rt_device_t serial_screen;
static struct rt_messagequeue rx_mq_screen;
rt_sem_t   sem_screen_send;

static rt_err_t uart_get_from_pc(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;

    result = rt_mq_send(&rx_mq_screen, &msg, sizeof(msg));
    if (result == -RT_EFULL)
    {
        rt_kprintf("screen message queue full \n");
    }
    return result;
}

static void serial_thread_entry(void *parameter)
{
    struct rx_msg msg;
    rt_err_t result;
    rt_uint32_t rx_length;
    static uint8_t rx_buffer[512];
    uint16_t cmd;
    uart_mode = UART_CMD_MODE;
    
    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));

        result = rt_mq_recv(&rx_mq_screen, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rt_pin_write(PIN_UART_LED,!rt_pin_read(PIN_UART_LED));
//            rt_kprintf("pc send: ");
//            for(uint16_t i = 0; i < rx_length; i++)
//            {
//                rt_kprintf("%x ",rx_buffer[i]);
//            }
//            rt_kprintf("\r\n");
            //这里设计两个模式：1：用户自定义数据模式  2：ymodem数据模式
            if(uart_mode == UART_CMD_MODE)
            {
                rx_buffer[rx_length] = '\0';
                serial_pc_msg_unpack(rx_buffer, rx_length);
            }
            else
            {
                drv_uart_ymodem_pc_to_mcu(rx_buffer, rx_length);
            }
            
        }
    }
}


rt_err_t uart_pc_send(uint8_t* buf, rt_size_t len)
{
    RT_ASSERT(sem_screen_send != RT_NULL);
	rt_err_t result;
	
	rt_sem_take(sem_screen_send, RT_WAITING_FOREVER);
	
	result = rt_device_write(serial_screen, 0, buf, len);
    
	rt_sem_release(sem_screen_send);
	
	return result;
}

int serial_pc_init(void)
{
    rt_err_t ret = RT_EOK;
    static char msg_pool_screen[1024];

    rt_pin_mode(UART1_EN, PIN_MODE_OUTPUT);
    rt_pin_write(UART1_EN, PIN_HIGH);
    
    rt_pin_mode(PIN_UART_LED, PIN_MODE_OUTPUT);
    
    serial_screen = rt_device_find(UART_NAME_SCREEN);
    if (!serial_screen)
    {
        rt_kprintf("find %s failed!\n", UART_NAME_SCREEN);
        return RT_ERROR;
    }
    rt_device_control(serial_screen, RT_DEVICE_CTRL_CONFIG, &config);

    rt_device_open(serial_screen, RT_DEVICE_FLAG_DMA_RX);
    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial_screen, uart_get_from_pc);
               
    uart_process_init();
    
    ret = rt_mq_init(&rx_mq_screen, "rx_mq_screen",
                       msg_pool_screen,                 /* 存放消息的缓冲区 */
                       sizeof(struct rx_msg),           /* 一条消息的最大长度 */
                       sizeof(msg_pool_screen),         /* 存放消息的缓冲区大小 */
                       RT_IPC_FLAG_FIFO);               /* 如果有多个线程等待，按照先来先得到的方法分配消息 */

				
         
    sem_screen_send = rt_sem_create("pc_send",1,RT_IPC_FLAG_PRIO);                       
                    
    if (ret != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    } 
    
    rt_thread_t thread_serial_screen = rt_thread_create("pc_cmd", serial_thread_entry, RT_NULL, 2048, 4, 20);
    /* 创建成功则启动线程 */
    if (thread_serial_screen != RT_NULL)
    {
        rt_thread_startup(thread_serial_screen);
    }
    else
    {
        ret = RT_ERROR;
    }
    
    return ret;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void uart_mode_read(uint8_t argc, char **argv)
{
    rt_kprintf("uart_mode   		= %s      \n", uart_mode?"UART_YMODEM_MODE":"UART_CMD_MODE");
}
MSH_CMD_EXPORT(uart_mode_read, uart_mode read);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */