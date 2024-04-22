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
#ifndef __ERROR_ID_H
#define __ERROR_ID_H

#define ERROR_OPEN  1
#define ERROR_CLOSE 2

/* 错误代码ID */
typedef enum
{
	ERROR_ID_NONE  = 0x0000,
    ERROR_ID_COOL_BOARD_LOST,
    ERROR_ID_SEED,
    ERROR_ID_SQ,
    ERROR_ID_PUMP1_THERMISTOR,
    ERROR_ID_PUMP2_THERMISTOR,          //5
    ERROR_ID_PUMP1_HTEMP,
    ERROR_ID_PUMP1_LTEMP,
    ERROR_ID_PUMP2_HTEMP,
    ERROR_ID_PUMP2_LTEMP,
    ERROR_ID_COOL_BOX1_THERMISTOR,      //10
    ERROR_ID_COOL_BOX2_THERMISTOR,
    ERROR_ID_COOL_BOX_HTEMP,
    ERROR_ID_COOL_BOX_LTEMP,
    ERROR_ID_LVL_1 = 0x00f1,
    ERROR_ID_LVL_2,
    ERROR_ID_LVL_3,
    ERROR_ID_NUM = 0xFFFF,
}error_id_t;

/* 错误列表 */
#define ERROR_BUF                                               \
{                                                               \
    ERROR_ID_COOL_BOARD_LOST,                                   \
    ERROR_ID_SEED,                                              \
    ERROR_ID_SQ,                                                \
    ERROR_ID_PUMP1_THERMISTOR,                                  \
    ERROR_ID_PUMP2_THERMISTOR,                                  \
    ERROR_ID_PUMP1_HTEMP,                                       \
    ERROR_ID_PUMP1_LTEMP,                                       \
    ERROR_ID_PUMP2_HTEMP,                                       \
    ERROR_ID_PUMP2_LTEMP,                                       \
    ERROR_ID_COOL_BOX1_THERMISTOR,                              \
    ERROR_ID_COOL_BOX2_THERMISTOR,                              \
    ERROR_ID_COOL_BOX_HTEMP,                                    \
    ERROR_ID_COOL_BOX_LTEMP,                                    \
    ERROR_ID_LVL_1,                                             \
    ERROR_ID_LVL_2,                                             \
    ERROR_ID_LVL_3,                                             \
};

/* 1级错误列表 */
#define ERROR_BUF_LVL_1                                         \
{                                                               \
    ERROR_ID_COOL_BOX_HTEMP,                                    \
    ERROR_ID_COOL_BOX_LTEMP,                                    \
    ERROR_ID_LVL_1,                                             \
};

/* 2级错误列表 */
#define ERROR_BUF_LVL_2                                         \
{                                                               \
    ERROR_ID_PUMP1_HTEMP,                                       \
    ERROR_ID_PUMP1_LTEMP,                                       \
    ERROR_ID_PUMP2_HTEMP,                                       \
    ERROR_ID_PUMP2_LTEMP,                                       \
    ERROR_ID_SEED,                                              \
    ERROR_ID_SQ,                                                \
    ERROR_ID_LVL_2,                                             \
};                                                              \
          
/* 3级错误列表 */
#define ERROR_BUF_LVL_3                                         \
{                                                               \
    ERROR_ID_COOL_BOARD_LOST,                                   \
    ERROR_ID_PUMP1_THERMISTOR,                                  \
    ERROR_ID_PUMP2_THERMISTOR,                                  \
    ERROR_ID_COOL_BOX1_THERMISTOR,                              \
    ERROR_ID_COOL_BOX2_THERMISTOR,                              \
    ERROR_ID_LVL_3,                                             \
}
#endif
													
