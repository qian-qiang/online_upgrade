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
#ifndef __ERROR_H
#define __ERROR_H

/* Includes ------------------------------------------------------------------*/

#include "rtthread.h"
#include "error_id.h"

/* types ------------------------------------------------------------*/

typedef enum
{
	ERROR_EXIST_NO = 0,
	ERROR_EXIST_YES,
	
}error_exist_t;

typedef enum
{
	ERROR_LVL_NONE = 0,  /*  没有错误	*/
	ERROR_LVL_1,
	ERROR_LVL_2,
    ERROR_LVL_3,
	
}error_lvl_t;


/* 错误链表结构体 */
typedef struct error_node_t
{
    error_id_t id;                        /*<!  错误                    */
    struct error_node_t *next;            /*<!  指向下一个节点的指针    */
    
}error_node_t;

/* define -----------------------------------------------------------*/
/*   error pintf debug   */
#define ERRROR_PRINT

/* variables --------------------------------------------------------*/


/* macro ------------------------------------------------------------*/
/* functions --------------------------------------------------------*/


void error_lock(void);
void error_unlock(void);
error_exist_t error_insert(error_id_t id);
error_exist_t popup_insert(error_id_t id);
error_exist_t error_delete(error_id_t id);
error_id_t error_code_get(void);
error_lvl_t error_lvl_get(void);
void error_display_sendout(error_id_t id);
void error_delete_by_screen(error_id_t id);
void error_heart_beat_send(error_id_t id);
void heart_beat_check_set(uint8_t flag);
int error_init(void);
error_id_t error_id_get(void);
#endif
