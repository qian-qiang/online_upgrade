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

#define UART_NAME_SCREEN        "uart1"      /* �����豸���� */
uint32_t GD_BUF[GD_LEN]={0};

struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};

#define PC_SERIAL_CONFIG_DEFAULT           \
{                                          \
    BAUD_RATE_19200, /* 115200 bits/s */  \
    DATA_BITS_8,      /* 8 databits */     \
    STOP_BITS_1,      /* 1 stopbit */      \
    PARITY_NONE,      /* No parity  */     \
    BIT_ORDER_LSB,    /* LSB first sent */ \
    NRZ_NORMAL,       /* Normal mode */    \
    RT_SERIAL_RB_BUFSZ, /* Buffer size */  \
    0                                      \
}
struct serial_configure config = PC_SERIAL_CONFIG_DEFAULT;  /* ��ʼ�����ò��� */

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
    
    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));

        result = rt_mq_recv(&rx_mq_screen, &msg, sizeof(msg), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            rx_length = rt_device_read(msg.dev, 0, rx_buffer, msg.size);
            rx_buffer[rx_length] = '\0';
            
//            rt_kprintf("pc send: ");
//            for(uint16_t i = 0; i < rx_length; i++)
//            {
//                rt_kprintf("%x ",rx_buffer[i]);
//            }
//            //rt_kprintf("------cmd is %s",enum_type_to_string(cmd));
//            rt_kprintf("\r\n");
            serial_pc_msg_unpack(rx_buffer, rx_length);
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
    
    serial_screen = rt_device_find(UART_NAME_SCREEN);
    if (!serial_screen)
    {
        rt_kprintf("find %s failed!\n", UART_NAME_SCREEN);
        return RT_ERROR;
    }
    rt_device_control(serial_screen, RT_DEVICE_CTRL_CONFIG, &config);

    rt_device_open(serial_screen, RT_DEVICE_FLAG_DMA_RX);
    /* ���ý��ջص����� */
    rt_device_set_rx_indicate(serial_screen, uart_get_from_pc);
               
    ret = rt_mq_init(&rx_mq_screen, "rx_mq_screen",
                       msg_pool_screen,                 /* �����Ϣ�Ļ����� */
                       sizeof(struct rx_msg),           /* һ����Ϣ����󳤶� */
                       sizeof(msg_pool_screen),         /* �����Ϣ�Ļ�������С */
                       RT_IPC_FLAG_FIFO);               /* ����ж���̵߳ȴ������������ȵõ��ķ���������Ϣ */

				
         
    sem_screen_send = rt_sem_create("pc_send",1,RT_IPC_FLAG_PRIO);                       
                    
    if (ret != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    } 
    
    rt_thread_t thread_serial_screen = rt_thread_create("pc_cmd", serial_thread_entry, RT_NULL, 2048, 4, 20);
    /* �����ɹ��������߳� */
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
