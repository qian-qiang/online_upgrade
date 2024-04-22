/*
 * Copyright (c) 2019, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-30     qq          the first version
 */

#ifndef __SERIAL_PC_H
#define __SERIAL_PC_H

#include "rtthread.h"

rt_err_t uart_pc_send(uint8_t* buf, rt_size_t len);
int serial_pc_init(void);

#endif
