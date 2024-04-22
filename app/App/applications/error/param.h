/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-16     qianqinag        the first version
 */
 
#ifndef __PARAM_H
#define __PARAM_H
#include <rtthread.h>


void param_key_value_init(char* key, uint32_t *value, uint32_t value_default);
void param_key_value_set(char* key, uint32_t *value);
int param_init(void);

#endif


