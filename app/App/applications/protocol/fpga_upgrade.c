/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-31     qq           the first version
 */
 
#define LOG_TAG      "fpga_up.file"
#define LOG_LVL      LOG_LVL_INFO

#include <ulog.h>
#include <rthw.h>
#include <rtthread.h>
#include <board.h>
#include <ymodem.h>
#include "fpga_upgrade.h"
#include "fpgaupdate.h"
#include "protocol_id.h"

static size_t update_file_total_size, update_file_cur_size;
static uint32_t crc32_checksum = 0;
static uint32_t crc32_value = 0;
static uint8_t per_printf = 0;
static uint32_t spi_f_addr=0;
static uint8_t fpga_upgrade_rec_flag = false;

extern uint16_t uart_mode;

static void log_out_print(size_t size, size_t total_size)
{
    can_cmd_t can_cmd;
    float per = size * 100 / total_size;
    
    if(per - per_printf >= 5)
    {
        per_printf = per;
        log_i("fpga upgrade tran: %d%%",per_printf);
    }
}

static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len) {
    char *file_name, *file_size;
    rt_pin_mode(NCONFIG, PIN_MODE_OUTPUT);
    /* calculate and store file size */
    file_name = (char *) &buf[0];
    file_size = (char *) &buf[rt_strlen(file_name) + 1];
    update_file_total_size = atol(file_size);
    /* 4 bytes align */
    update_file_total_size = (update_file_total_size + 3) / 4 * 4;
    per_printf = 0;
    update_file_cur_size = 0;
    spi_f_addr = 0;
    log_i("ymodem_on_begin: file name is %s, size :%d B",file_name, update_file_total_size);
    
    as_program_start();
    log_i("ymodem_on_begin: fpga flash program start");
	as_bulk_erase();       
    log_i("ymodem_on_begin: fpga flash erase bulk all");
    
//    /* fpga commuinicate failed */
//    if (fpga_comm() <= 0) {
//        as_program_done();
//        return RYM_CODE_CAN;
//    }

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len) 
{
    uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], *buf_ptr;
    /* write .pof file to fpga flash  */
    //memcpy(packet_data,buf,len);
    as_prog(buf,len,spi_f_addr);//写入ECPS//as_prog(buf_ptr,1024,j*1024)
    //rt_kprintf("ymodem_on_data write to fpga flash spi_f_addr is 0x%x ,len is %d \r\n",spi_f_addr, len);
    if(as_verify(buf,len,spi_f_addr)== 1)//读出ECPS与数据包校验//if(as_verify(buf_ptr,1024,j*1024)== 1)//读出ECPS与数据包校验
    {
        log_e("ymodem_on_data: fpga flash verify not success");
        return RYM_CODE_CAN;
    }
    else
    {
        spi_f_addr+=len;  
    }				
    
    update_file_cur_size += len;
    log_out_print(update_file_cur_size, update_file_total_size);
    
    return RYM_CODE_ACK;
}

static rt_err_t ymodem_recv(void) 
{
    rt_err_t result = -RT_ERROR;
    char  c_file_size[21] = { 0 }, c_crc32_checksum[21] = { 0 };
    struct rym_ctx rctx;
    rt_device_t dev;
    
    dev = rt_device_find("u_ymodem");
    
    if (!rym_recv_on_device(&rctx, dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
            ymodem_on_begin, ymodem_on_data, NULL, RT_TICK_PER_SECOND)) 
    {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);

        /* set need copy application from backup section flag is 1, backup application length */
        rt_sprintf(c_file_size, "%ld", update_file_total_size);

        log_e("fpga flash write rbf file success");
		log_i("reboot fpga now after 1s.");
		rt_thread_delay(1000);
    } 
    else 
    {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        fpga_upgrade_rec_flag = false;
        log_e("fpga flash write rbf file failed");
    }
    
__exit:
    as_program_done();
    rt_hw_cpu_reset();
    return result;
}

static void thread_fpga_ug_rec(void *parameter)
{
    rt_err_t result;
    uart_mode = UART_YMODEM_MODE;

    result = ymodem_recv();
    
    log_e("fpga upgrade over.");

    uart_mode = UART_CMD_MODE;
}

void fpga_upgarde_file_rec(uint32_t crc_value)
{
    crc32_value = crc_value;
    
	if(false == fpga_upgrade_rec_flag)
	{
		rt_thread_t fpga_upgrade_rec = rt_thread_create("fpga_ug_r",
							    thread_fpga_ug_rec, RT_NULL,
							    2048,
							    27, 
							    20);
        log_i("fpga_upgrade_file_rec thread starting....");
		rt_thread_startup(fpga_upgrade_rec);
        fpga_upgrade_rec_flag = true;
	}
}
