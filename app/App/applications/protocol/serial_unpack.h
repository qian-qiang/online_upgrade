/*
 * Copyright (c) 2019, redoc
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-30     qq          the first version
 */

#ifndef __SERIAL_UNPACK_H
#define __SERIAL_UNPACK_H

#include "rtthread.h"
#include "protocol_id.h"


int serial_pc_msg_unpack(uint8_t *buf, uint32_t length);
void uart_pc_protocol_send(uart_cmd_t *pack);
void uart_process_init(void);
#endif
