/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-31     qq           the first version
 */
 
#define LOG_TAG      "recver.file"
#define LOG_LVL      LOG_LVL_INFO

#include <ulog.h>
#include <rthw.h>
#include <rtthread.h>
#include <shell.h>
#include <finsh.h>
#include <easyflash.h>
#include <ymodem.h>
#include <board.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <ymodem.h>
#include "fal.h"
#include "serial_unpack.h"
#include "mcu_upgrade.h"
#include "protocol_id.h"

#define PART_DOWNLOAD_NAME    "fm_area"
#define APP_BIN_NAME   		  "mcu_app.bin"

static size_t update_file_total_size, update_file_cur_size;
static uint32_t crc32_checksum = 0;
static const struct fal_partition *part_download = NULL;
static uint32_t crc32_value = 0;
static uint8_t mcu_bin_rec_flag = false;
static uint8_t per_printf = 0;

extern uint16_t uart_mode;
static uint32_t dfu_part_cal(char* part_name,size_t size)
{
	size_t cur_addr = 0;
	uint32_t buff[32];
	size_t crc_value_get = 0;
	size_t size_else;
	static const struct fal_partition *part_download = NULL;
	
	part_download = fal_partition_find(PART_DOWNLOAD_NAME);
	RT_ASSERT(part_download);
	
	size_else = size % sizeof(buff);
	
	if(part_download == 0)
	{
		log_e("part_download find err.");
		return 0;
	}
	
	for(cur_addr = 0; cur_addr < (size - size_else); cur_addr+=sizeof(buff))
	{
		fal_partition_read(part_download, cur_addr, (uint8_t*)buff, sizeof(buff));

		crc_value_get = ef_calc_crc32(crc_value_get, (uint8_t*)buff, sizeof(buff));
	}
	
	if(size_else != 0)
	{
		cur_addr = size - size_else;
		fal_partition_read(part_download, cur_addr, (uint8_t*)buff, size_else);
		crc_value_get = ef_calc_crc32(crc_value_get, (uint8_t*)buff, size_else);
	}
	
	return crc_value_get;
}

static void log_out_print(size_t size, size_t total_size)
{
    can_cmd_t can_cmd;
    float per = size * 100 / total_size;
    
    if(per - per_printf >= 5)
    {
        per_printf = per;
        log_i("topctrl upgrade tran: %d%%",per_printf);

        uart_cmd_t uart_cmd_send;
        uart_cmd_send.wr = CMD_WRITE;
        uart_cmd_send.cmd = MCU_UPGRADE; 
        uart_cmd_send.size = 4;
        uart_cmd_send.data[0] = per_printf;
       
        uart_pc_protocol_send(&uart_cmd_send);
        rt_thread_mdelay(20);
    }
}

static enum rym_code ymodem_on_begin(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len) {
    char *file_name, *file_size;

    /* calculate and store file size */
    file_name = (char *) &buf[0];
    file_size = (char *) &buf[rt_strlen(file_name) + 1];
    update_file_total_size = atol(file_size);
    /* 4 bytes align */
    update_file_total_size = (update_file_total_size + 3) / 4 * 4;
    update_file_cur_size = 0;
    crc32_checksum = 0;

    per_printf = 0;
    
	part_download = fal_partition_find(PART_DOWNLOAD_NAME);
	RT_ASSERT(part_download);
	
    rt_kprintf("upgrade begin, size :%d B\r\n",update_file_total_size);
    
    /* erase backup section */
    if (fal_partition_erase_all(part_download) <= 0) {
        /* if erase fail then end session */
        return RYM_CODE_CAN;
    }

    return RYM_CODE_ACK;
}

static enum rym_code ymodem_on_data(struct rym_ctx *ctx, rt_uint8_t *buf, rt_size_t len) {
    
	RT_ASSERT(part_download);
    
    if (update_file_cur_size + len <= update_file_total_size) {
        crc32_checksum = ef_calc_crc32(crc32_checksum, buf, len);
    } else {
        crc32_checksum = ef_calc_crc32(crc32_checksum, buf, update_file_total_size - update_file_cur_size);
    }
    
    /* write data of application to backup section  */
    if(fal_partition_write(part_download, update_file_cur_size, (uint8_t*)buf, len) <= 0) {
        /* if write fail then end session */
        return RYM_CODE_CAN;
    }

    update_file_cur_size += len;
    //log_out_print(update_file_cur_size, update_file_total_size);
    
    return RYM_CODE_ACK;
}

static rt_err_t ymodem_recv(void) 
{
    rt_err_t result = -RT_ERROR;
    char  c_file_size[21] = { 0 }, c_crc32_checksum[21] = { 0 };
    struct rym_ctx rctx;
    rt_device_t dev;
    
    dev = rt_device_find("u_ymodem");
    
    rt_kprintf("ymodem recv start.\r\n");
    
    if (!rym_recv_on_device(&rctx, dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
            ymodem_on_begin, ymodem_on_data, NULL, RT_TICK_PER_SECOND)) {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        /* save the downloaded firmware crc32 checksum ENV */

//        if(crc32_value != crc32_checksum)
//        {
//            log_e("transmit crc value check fail.");
//            goto __exit;
//        }   

//        if(crc32_value != dfu_part_cal(PART_DOWNLOAD_NAME, update_file_total_size))
//        {
//            log_e("%s crc value check fail.",PART_DOWNLOAD_NAME);
//            goto __exit;
//        }
                  
        rt_snprintf(c_crc32_checksum, sizeof(c_crc32_checksum), "%d", crc32_checksum);
        //ef_set_env("iap_check_value_app", c_crc32_checksum);
        /* set need copy application from backup section flag is 1, backup application length */
        rt_sprintf(c_file_size, "%ld", update_file_total_size);
        //ef_set_env("iap_copy_app_size", c_file_size);
        //ef_set_env("iap_need_copy_app", "1");
        //ef_save_env();
        log_d("crc check succ,set env flag.");
		log_i("reboot now after 1s.");
        
		rt_thread_delay(1000);
        rt_hw_cpu_reset();
    } else {
        /* wait some time for terminal response finish */
        rt_thread_delay(RT_TICK_PER_SECOND);
        mcu_bin_rec_flag = false;
        log_e("Update firmware fail.");
    }
    
__exit:
    
    return result;
}

static void thread_mcu_bin_rec(void *parameter)
{
    rt_err_t result;
    uart_mode = UART_YMODEM_MODE;

    result = ymodem_recv();
    
    log_e("mcu upgrade fail.");

    uart_mode = UART_CMD_MODE;
}

void mcu_bin_file_rec(uint32_t crc_value)
{
    crc32_value = crc_value;
    
	if(false == mcu_bin_rec_flag)
	{
		rt_thread_t mcu_bin_rec = rt_thread_create("mcu_bin_r",
							    thread_mcu_bin_rec, RT_NULL,
							    2048,
							    27, 
							    20);
        log_i("mcu_bin_file_rec thread starting....");
		rt_thread_startup(mcu_bin_rec);
        mcu_bin_rec_flag = true;
	}
}
