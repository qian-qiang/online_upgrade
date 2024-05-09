/*
 * Copyright (c) 2019-2020, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-24     qianqiang    the first version
 */

#ifndef __TH_H
#define __TH_H

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "stdbool.h"

struct th_senser
{
    struct rt_i2c_bus_device *th_dev;
    float humidity;
    float temperature;
    rt_int32_t th_update;  
};

struct th_class
{
	struct th_senser ir_senser;
    struct th_senser uv_senser;
};

#define TH_CLASS_DEFAULT                                                \
{                                                                       \
    .ir_senser.th_dev                 = RT_NULL,                        \
    .ir_senser.humidity               = false,                          \
    .ir_senser.temperature            = false,                          \
    .ir_senser.th_update              = false,                          \
    .uv_senser.th_dev                 = RT_NULL,                        \
    .uv_senser.humidity               = false,                          \
    .uv_senser.temperature            = false,                          \
    .uv_senser.th_update              = false,                          \
}

int thread_th_init(void);
/*************************************API**************************************/
void uv_temp_humi_get(rt_uint32_t *cur_temp, rt_uint32_t *cur_humi);
void ir_temp_humi_get(rt_uint32_t *cur_temp, rt_uint32_t *cur_humi);
#endif
