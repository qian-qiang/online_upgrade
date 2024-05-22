/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-31     qq           the first version
 */

#define LOG_TAG      "unpack"
#define LOG_LVL      LOG_LVL_DBG

#include <rtthread.h>
#include <board.h>
#include "rtdbg.h"
#include "serial_unpack.h"
#include "serial_pc.h"
#include "stdbool.h"
#include "task.h"
#include "upacker.h"
#include "fpga.h"
#include <stdlib.h>
#include <sys/time.h>
#include "mcu_upgrade.h"
#include "fpga_upgrade.h"
#include "drv_uart_ymodem.h"

static void uart_protocol_unpack(uart_cmd_t *pack, uint8_t* data, uint16_t size);
static void pc_handle_cb(uint8_t *d, uint16_t size);

upacker_inst pc_uart_packer;

static uint8_t cmd_crc(uart_cmd_t *buf)
{
		uint8_t tmp=0xff;
		tmp=tmp^buf->send_buf[2];
		tmp=tmp^buf->send_buf[3];
		tmp=tmp^buf->send_buf[4];
		tmp=tmp^buf->send_buf[5];
		tmp=tmp^buf->send_buf[6];
		tmp=tmp^buf->send_buf[7];
		tmp=tmp^buf->send_buf[8];
		tmp=tmp^0xa2;
		return tmp;
}

static uint8_t bcd2dec(uint8_t bcd)
{
	return (bcd - (bcd >> 4) * 6);
}

static void uart_pc_process_send(uint8_t *d, uint16_t size)
{
    uart_pc_send(d, size);
}

int serial_pc_msg_unpack(uint8_t *buf, uint32_t length)
{
    upacker_unpack(&pc_uart_packer, buf, length);
    return RT_EOK;
}

void run_cmd_set(rt_uint8_t cmd, uart_cmd_t *pack)
{
	switch(cmd)
	{ 				
		default:break;	
	}		
}

static void pc_handle_cb(uint8_t *d, uint16_t size)
{
    uart_cmd_t cmd_unpack;
	uart_protocol_unpack(&cmd_unpack, d, size);
    uart_cmd_t uart_cmd_send;

    uart_cmd_send.wr = cmd_unpack.wr;
    uart_cmd_send.cmd = cmd_unpack.cmd; 
    uart_cmd_send.data[0] = false;
    uart_cmd_send.data[1] = false;
    uart_cmd_send.data[2] = false;
    uart_cmd_send.data[3] = false;
    uart_cmd_send.size = 4;

    switch(cmd_unpack.cmd)
    {
         case MCU_UPGRADE:
            log_d("pc send mcu upgrade cmd");	
            uint32_t mcu_crc32_value = 0;
            rt_memcpy(&mcu_crc32_value, &cmd_unpack.data[0], 4);
            mcu_bin_file_rec(mcu_crc32_value);
			break;	
         
         case FPGA_UPGRADE:
            log_d("pc send fpga upgrade cmd");	
            uint32_t foga_crc32_value = 0;
            rt_memcpy(&foga_crc32_value, &cmd_unpack.data[0], 4);
            fpga_upgarde_file_rec(foga_crc32_value);
			break;	
		default:    
            break;            				
	}
}

static void uart_protocol_unpack(uart_cmd_t *pack, uint8_t* data, uint16_t size)
{
	pack->wr = data[0];
	pack->cmd = data[1];
	pack->size = size;
	rt_memcpy(pack->data, &data[2], 4);
}

void uart_pc_protocol_send(uart_cmd_t *pack)
{
    pack->send_buf[0] = 0xfe;
    pack->send_buf[1] = 0xef;
    pack->send_buf[2] = 0xff;
	pack->send_buf[3] = pack->wr;
    pack->send_buf[4] = pack->cmd;

	rt_memcpy(&(pack->send_buf[5]), pack->data, pack->size);
	
	upacker_pack(&pc_uart_packer, pack->send_buf, pack->size + 5);
}

void uart_process_init(void)
{
	//init packer
    upacker_init("pc_upacker", &pc_uart_packer, pc_handle_cb, uart_pc_process_send);
}



