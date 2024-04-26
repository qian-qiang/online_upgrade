/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-24     qianqiang    the first version
 */
 
#define LOG_TAG      "mcu_upgrade"
#define LOG_LVL      LOG_LVL_DBG
#include <ulog.h>
#include "mcu_upgrade.h"
#include "easyflash.h"
#include <rtthread.h>
#include <dfs_posix.h>
#include "fal.h"
#include "serial_unpack.h"

#define PART_DOWNLOAD_NAME    "fm_area"
#define APP_BIN_NAME   		  "mcu_app.rbl"

static uint8_t mcu_upgrade_flag = false;
static uint8_t per_printf = 0;
    
static void log_out_print(size_t size, size_t total_size)
{
    float per = size * 100 / total_size;
    
    if(per - per_printf >= 5)
    {
        per_printf = per;
        rt_kprintf("log output %3d%\r\n",per_printf);
        //screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_UPGRADE_CTRLBOX_PER, per_printf);
    }
}

static int upgrade_app_size_get(void)
{
	int ret;
	struct stat buf;
	char addr_buf[32] = {0};
	
	sprintf(addr_buf,"/%s",APP_BIN_NAME);
	
	ret = stat(addr_buf, &buf);
	if(ret == 0)
	{
		log_d("%s file size = %d",APP_BIN_NAME, buf.st_size);
		return buf.st_size;
	}
	else
	{
		log_e("%s file not fonud",APP_BIN_NAME);
		
//		error_insert(ERROR_ID_SD_INSTALL_FAULT);
		return 0;
	}
}

static int upgrade_app_crc_cal(uint32_t cal_size)
{
	int fd, cur_size, size_else, size;
	uint32_t buff[32];
	char addr_buf[32] = {0};
	size_t crc_value_get = 0;
	
	sprintf(addr_buf,"/%s",APP_BIN_NAME);
	
	size_else = cal_size % sizeof(buff);
	
	fd = open(addr_buf, O_RDONLY);
	
	if (fd>= 0)
    {
		lseek(fd, 0, 0);
		
		for(cur_size = 0; cur_size < (cal_size - size_else); cur_size += sizeof(buff))
		{
			size = read(fd, buff, sizeof(buff));
			
			if(size < 0)
			{
				log_d("%s file read error",APP_BIN_NAME);
				close(fd);
				return 0;
			}
			
			crc_value_get = ef_calc_crc32(crc_value_get, (uint8_t*)buff, sizeof(buff));
		}
		
		if( 0 != size_else)
		{
			size = read(fd, buff, size_else);
			crc_value_get = ef_calc_crc32(crc_value_get, (uint8_t*)buff, size_else);
			
			if(size < 0)
			{
				log_d("%s file read error",APP_BIN_NAME);
				close(fd);
				return 0;
			}
        }
        
    }
	else
	{
		log_e("%s file not fonud",APP_BIN_NAME);
		close(fd);
		return 0;
	}
	
	return crc_value_get;
}

static uint32_t dfu_crc_cal(uint32_t size)
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

static rt_err_t upgrade_copy_to_download(uint32_t app_size)
{
	rt_err_t result = RT_EOK;
	int fd;
	size_t cur_size,size,size_else;
    uint32_t cur_addr;
	static const struct fal_partition *part_download = NULL;
    uint32_t buff[32];
	char addr_buf[32] = {0};

	part_download = fal_partition_find(PART_DOWNLOAD_NAME);
	RT_ASSERT(part_download);
	
	fal_partition_erase_all(part_download);

	rt_thread_delay(1000);
	log_d("part download erase.");
	
	sprintf(addr_buf,"/%s",APP_BIN_NAME);
	fd = open(addr_buf, O_RDONLY);

	size_else = app_size % sizeof(buff);
	
    per_printf = 0;
    
	if(fd > 0)
	{
		for (cur_size = 0; cur_size < (app_size - size_else); cur_size += sizeof(buff)) 
		{
			cur_addr = 0 + cur_size;
			
			size = read(fd, buff, sizeof(buff));
			
			if(size != sizeof(buff))
			{
				log_e("file %s read error.",APP_BIN_NAME);
				result = -RT_ERROR;
			}
			
			size = fal_partition_write(part_download, cur_addr, (uint8_t*)buff, sizeof(buff));
			
			if(size != sizeof(buff))
			{
				printf("part download write error.\r\n");
				result = -RT_ERROR;
			}
            
            log_out_print(cur_size, app_size);
		}
		
		if(size_else)
		{
			cur_addr = app_size - size_else;
            
			cur_size += size_else;
			size = read(fd, buff, size_else);
			
			if(size != size_else)
			{
				log_e("file %s read error.",APP_BIN_NAME);
				result = -RT_ERROR;
			}
			
			size = fal_partition_write(part_download, cur_addr, (uint8_t*)buff, size_else);
			
			if(size != size_else)
			{
				printf("part download write error.\r\n");
				result = -RT_ERROR;
			}
            
            log_out_print(cur_size, app_size);
		}
	}
	else
	{
		log_e("%s file not fonud",APP_BIN_NAME);
		result = -RT_ERROR;
	}
	
	close(fd);
	
	return result;
}

#define UPGRADE_OK 0X0002
#define UPGRADE_FAILED 0X0001

static void thread_upgrade_entry(void *parameter)
{
	uint32_t app_size;
	int crc_value_fd,crc_value_download;
	char char_value[21] = { 0 };
    
    //screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_UPGRADE_CTRLBOX_CIRCLE, true);   
	app_size = upgrade_app_size_get();
  
    if(0 == app_size)
    {
        goto __exit;
    }
	
	crc_value_fd = upgrade_app_crc_cal(app_size);
	
	log_d("crc value: 0x%x",crc_value_fd);

	if(RT_EOK == upgrade_copy_to_download(app_size))
	{
		log_i("upgrade copy to download size:%d",app_size);
		
	}
	else
	{
		log_e("upgrade copy to download fail");
		goto __exit;
	}
	
	crc_value_download = dfu_crc_cal(app_size);
	
	log_d("dfu crc value: 0x%x",crc_value_download);
	
	if(crc_value_download == crc_value_fd)
	{
		rt_memset(char_value,0,20);
		snprintf(char_value, 20, "%d", crc_value_fd);
		ef_set_env("iap_check_value_app",char_value);
		
		rt_memset(char_value,0,20);
		snprintf(char_value, 20, "%d", app_size);
		ef_set_env("iap_copy_app_size",char_value);
        ef_set_env("iap_need_copy_app","1");
        ef_save_env();
		log_d("crc check succ,set env flag.");
		log_i("reboot now after 1s.");
        
        //screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_UPGRADE_CTRLBOX_CIRCLE, false);   
        //screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_UPGRADE_CTRLBOX_OK_FAIL, UPGRADE_OK);   
        
		rt_thread_delay(1000);
		
		rt_hw_cpu_reset();
	}
	else
	{
        rt_thread_delay(RT_TICK_PER_SECOND);
		log_e("Update firmware fail.");
        goto __exit;
	}

__exit:
    //screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_UPGRADE_CTRLBOX_CIRCLE, false);   
    //screen_protocol_16_send(PROTOCOL_CMD_CTRLBOX_TO_PROXIMAL_SCREEN_UPGRADE_CTRLBOX_OK_FAIL, UPGRADE_FAILED);   
	mcu_upgrade_flag = false;
}

void mcu_upgrade(void)
{
	if(false == mcu_upgrade_flag)
	{
		rt_thread_t tid_upgrade = rt_thread_create("mcu_upgrade",
							    thread_upgrade_entry, RT_NULL,
							    2048,
							    26, 
							    20);
	
		if(RT_EOK == rt_thread_startup(tid_upgrade))
		{
			mcu_upgrade_flag = true;
		}
	}
}


#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void ctrlbox_ota(uint8_t argc, char **argv)
{
	mcu_upgrade();
}
MSH_CMD_EXPORT(ctrlbox_ota, app upgrade);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */


