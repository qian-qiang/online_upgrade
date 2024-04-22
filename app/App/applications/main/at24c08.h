/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-1-30      qianqiang   the first version
 */

#ifndef __AT24C08_H__
#define __AT24C08_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "at24cxx.h"

void AT24CXX_Init(void);
uint8_t AT24CXX_ReadOneByte(uint32_t ReadAddr);
void AT24CXX_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead);
void AT24CXX_WriteOneByte(uint32_t WriteAddr, uint8_t pBuffer);
void AT24CXX_Write(uint32_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite);
#endif