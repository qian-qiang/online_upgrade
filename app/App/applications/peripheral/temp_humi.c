/*
 * Copyright (c) 2019-2020, QQ
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-3-24     qianqiang     the first version
 */
#define LOG_TAG      "lsm"
#define LOG_LVL      LOG_LVL_DBG

#include "temp_humi.h"
#include "ulog.h"

#define THREAD_TH_PRIORITY         (15)
#define THREAD_TH_STACK_SIZE       (1024)
#define THREAD_TH_TIMESLICE        (20)

#define AHT10_ADDR                  0x38    /* 从机地址 */
#define AHT10_CALIBRATION_CMD       0xE1    /* 校准命令 */
#define AHT10_NORMAL_CMD            0xA8    /* 一般命令 */
#define AHT10_GET_DATA              0xAC    /* 获取数据命令 */

static bool th_init_ok = false;
static struct th_class th = TH_CLASS_DEFAULT;
static rt_thread_t th_thread = RT_NULL;

static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;
    buf[0] = reg; //cmd
    buf[1] = data[0];
    buf[2] = data[1];
    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 3;
    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

/* 读传感器寄存器数据 */
static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;
    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;
    /* 调用I2C设备接口传输数据 */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

static void read_ir_temp_humi(float *cur_temp, float *cur_humi)
{
    rt_uint8_t temp[6];
    write_reg(th.ir_senser.th_dev, AHT10_GET_DATA, 0);      /* 发送命令 */
    read_regs(th.ir_senser.th_dev, 6, temp);                /* 获取传感器数据 */
    /* 湿度数据转换 */
    *cur_humi = (temp[1] << 12 | temp[2] << 4 | (temp[3] & 0xf0) >> 4) * 100.0 / (1 << 20);
    /* 温度数据转换 */
    *cur_temp = ((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50;
}

static void read_uv_temp_humi(float *cur_temp, float *cur_humi)
{
    rt_uint8_t temp[6];
    write_reg(th.uv_senser.th_dev, AHT10_GET_DATA, 0);      /* 发送命令 */
    read_regs(th.uv_senser.th_dev, 6, temp);                /* 获取传感器数据 */
    /* 湿度数据转换 */
    *cur_humi = (temp[1] << 12 | temp[2] << 4 | (temp[3] & 0xf0) >> 4) * 100.0 / (1 << 20);
    /* 温度数据转换 */
    *cur_temp = ((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50;
}

static void lsm6dsl_thread_entry(void *args)
{
    rt_uint8_t temp[2] = {0, 0};
    
    th.ir_senser.th_dev = (struct rt_i2c_bus_device *)rt_device_find("i2c2");
    th.ir_senser.humidity = 0.0;
    th.ir_senser.temperature = 0.0;
    
    th.uv_senser.th_dev = (struct rt_i2c_bus_device *)rt_device_find("i2c3");
    th.uv_senser.humidity = 0.0;
    th.uv_senser.temperature = 0.0;
    
    if (!th.ir_senser.th_dev) 
    {
        rt_kprintf("can't find th device.\n");
        return;
    }

    if (!th.uv_senser.th_dev) 
    {
        rt_kprintf("can't find th device.\n");
        return;
    }
    
    write_reg(th.ir_senser.th_dev, AHT10_NORMAL_CMD, temp);
    rt_thread_mdelay(400);
    temp[0] = 0x08;
    temp[1] = 0x00;
    write_reg(th.ir_senser.th_dev, AHT10_CALIBRATION_CMD, temp);
    rt_thread_mdelay(400);
    
    write_reg(th.uv_senser.th_dev, AHT10_NORMAL_CMD, temp);
    rt_thread_mdelay(400);
    temp[0] = 0x08;
    temp[1] = 0x00;
    write_reg(th.uv_senser.th_dev, AHT10_CALIBRATION_CMD, temp);
    rt_thread_mdelay(400);
    
    th_init_ok = RT_TRUE;
    
    while(1)
    {
        read_ir_temp_humi(&th.ir_senser.temperature, &th.ir_senser.humidity);
        read_uv_temp_humi(&th.uv_senser.temperature, &th.uv_senser.humidity);
        rt_thread_mdelay(1000);
    }
         
}

int thread_th_init(void)
{
    rt_err_t result;
    
    th_thread = rt_thread_create("th", lsm6dsl_thread_entry, 
                                        RT_NULL, 
                                        THREAD_TH_STACK_SIZE, 
                                        THREAD_TH_PRIORITY,
                                        THREAD_TH_TIMESLICE);
    
    result = rt_thread_startup(th_thread);

    return result;
}
/**********************************************************API*******************************************************************/
void ir_temp_humi_get(rt_uint32_t *cur_temp, rt_uint32_t *cur_humi)
{
    *cur_humi = th.ir_senser.humidity;
    *cur_temp = th.ir_senser.temperature;
}

void uv_temp_humi_get(rt_uint32_t *cur_temp, rt_uint32_t *cur_humi)
{
    *cur_humi = th.uv_senser.humidity;
    *cur_temp = th.uv_senser.temperature;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void th_read(uint8_t argc, char **argv)
{
    if(false == th_init_ok)
    {
        rt_thread_mdelay(1000);
        thread_th_init();
    }
    else{
        rt_kprintf("read aht10 ir_senser humidity   : %d.%d %%\n", (int)th.ir_senser.humidity, (int)(th.ir_senser.humidity * 10) % 10);
        rt_kprintf("read aht10 ir_senser temperature: %d.%d \n", (int)th.ir_senser.temperature, (int)(th.ir_senser.temperature * 10) % 10);
        
        rt_kprintf("read aht10 uv_senser humidity   : %d.%d %%\n", (int)th.uv_senser.humidity, (int)(th.uv_senser.humidity * 10) % 10);
        rt_kprintf("read aht10 uv_senser temperature: %d.%d \n", (int)th.uv_senser.temperature, (int)(th.uv_senser.temperature * 10) % 10);
        
    }
}
MSH_CMD_EXPORT(th_read, th_class read);

#endif
