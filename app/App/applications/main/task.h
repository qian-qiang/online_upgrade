/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-05     qianqinag        the first version
 */
 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TASK_H
#define __TASK_H

/* Includes ------------------------------------------------------------------*/

#include "rtthread.h"

extern rt_uint8_t LASER_ON_FLAG; 
extern rt_uint8_t USER_TIME_FLAG[3];
extern rt_uint8_t keyState;
extern rt_bool_t laserSwitchFlag;
extern rt_bool_t UserSetValueChaFlag[4];
int task_init(void);
#endif
