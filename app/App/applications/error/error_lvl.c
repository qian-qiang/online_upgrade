/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-05     qianqinag        the first version
 */

/* Includes ------------------------------------------------------------------*/

#include "error.h"

/*  将错误等级正确添加到列表中 */
error_id_t error_buf[] = ERROR_BUF;
error_id_t error_lvl_1_buf[] = ERROR_BUF_LVL_1;
error_id_t error_lvl_2_buf[] = ERROR_BUF_LVL_2;
error_id_t error_lvl_3_buf[] = ERROR_BUF_LVL_3;

/* 查询对对应ID错误在lvl_3列表中 */
error_exist_t is_error_in_lvl_3_buf(error_id_t id)
{
	error_exist_t exist = ERROR_EXIST_NO;
	uint16_t error_lvl_buf_len = sizeof(error_lvl_3_buf)/sizeof(error_id_t);
    
	for(uint16_t i = 0; i < error_lvl_buf_len; i++)
	{
		if(id == error_lvl_3_buf[i])
		{
			exist = ERROR_EXIST_YES;
			return exist;
		}
	}
	
	return exist;
}

/* 查询对对应ID错误在lvl_2列表中 */
error_exist_t is_error_in_lvl_2_buf(error_id_t id)
{
	error_exist_t exist = ERROR_EXIST_NO;
	uint16_t error_lvl_buf_len = sizeof(error_lvl_2_buf)/sizeof(error_id_t);
    
	for(uint16_t i = 0; i < error_lvl_buf_len; i++)
	{
		if(id == error_lvl_2_buf[i])
		{
			exist = ERROR_EXIST_YES;
			return exist;
		}
	}
	
	return exist;
}

/* 查询对对应ID错误在lvl_1列表中 */
error_exist_t is_error_in_lvl_1_buf(error_id_t id)
{
	error_exist_t exist = ERROR_EXIST_NO;
	uint16_t error_lvl_buf_len = sizeof(error_lvl_1_buf)/sizeof(error_id_t);
    
	for(uint16_t i = 0; i < error_lvl_buf_len; i++)
	{
		if(id == error_lvl_1_buf[i])
		{
			exist = ERROR_EXIST_YES;
			return exist;
		}
	}
	
	return exist;
}

/* 查询对对应ID错误在列表中 */
error_exist_t is_error_in_buf(error_id_t id)
{
	error_exist_t exist = ERROR_EXIST_NO;
	uint16_t error_lvl_buf_len = sizeof(error_buf)/sizeof(error_id_t);
    
	for(uint16_t i = 0; i < error_lvl_buf_len; i++)
	{
		if(id == error_buf[i])
		{
			exist = ERROR_EXIST_YES;
			return exist;
		}
	}
	
	return exist;
}
