/*
 * Copyright (c) 2019-2020, qianqinag
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-12-05     qianqinag        the first version
 */

#define LOG_TAG   "error"  
#define LOG_LVL   LOG_LVL_DBG 
/* Includes ------------------------------------------------------------------*/
#include "error.h"
#include "ulog.h"

/* types ------------------------------------------------------------*/

/* define -----------------------------------------------------------*/
/* variables --------------------------------------------------------*/
static error_node_t* error_handle = NULL;
static error_node_t* target_tr_read = NULL;
/* macro ------------------------------------------------------------*/
/* functions --------------------------------------------------------*/
static error_node_t *error_exist(error_id_t id);
static error_exist_t error_find(void);
static error_exist_t is_error_lvl_3_exist(void);
static error_exist_t is_error_lvl_2_exist(void);
static error_exist_t is_error_lvl_1_exist(void);
extern error_exist_t is_error_in_buf(error_id_t id);
extern error_exist_t is_error_in_lvl_3_buf(error_id_t id);
extern error_exist_t is_error_in_lvl_2_buf(error_id_t id);
extern error_exist_t is_error_in_lvl_1_buf(error_id_t id);

/* 查询是否有错误在lvl2列表中 */
static error_exist_t is_error_lvl_3_exist(void)
{
	error_exist_t exist = ERROR_EXIST_NO;
	
	struct error_node_t** curr;
    
	for(curr = &error_handle; *curr; ) 
	{
		struct error_node_t* entry = *curr;
		if (is_error_in_lvl_3_buf(entry->id)) 
		{
			exist = ERROR_EXIST_YES;
			return exist;
		} 
		else
		{
			curr = &entry->next;
		}	
	}
	
	return exist; 
}

/* 查询是否有错误在lvl2列表中 */
static error_exist_t is_error_lvl_2_exist(void)
{
	error_exist_t exist = ERROR_EXIST_NO;
	
	struct error_node_t** curr;
    
	for(curr = &error_handle; *curr; ) 
	{
		struct error_node_t* entry = *curr;
		if (is_error_in_lvl_2_buf(entry->id)) 
		{
			exist = ERROR_EXIST_YES;
			return exist;
		} 
		else
		{
			curr = &entry->next;
		}	
	}
	
	return exist; 
}

/* 查询是否有错误在lvl2列表中 */
static error_exist_t is_error_lvl_1_exist(void)
{
	error_exist_t exist = ERROR_EXIST_NO;
	
	struct error_node_t** curr;
    
	for(curr = &error_handle; *curr; ) 
	{
		struct error_node_t* entry = *curr;
		if (is_error_in_lvl_1_buf(entry->id)) 
		{
			exist = ERROR_EXIST_YES;
			return exist;
		} 
		else
		{
			curr = &entry->next;
		}	
	}
	
	return exist; 
}

/* 查询节点 成功返回地址 否则返回空 */
static error_node_t *error_exist(error_id_t id)
{
	struct error_node_t** curr;
    
    for(curr = &error_handle; *curr; ) 
    {
        struct error_node_t* entry = *curr;
        if (entry->id == id) 
        {
            return entry;
        } 
        else
        {
            curr = &entry->next;
        }   
    }
    
    return RT_NULL;
}

/* 查询是否有错误在列表中 */
static error_exist_t error_find(void)
{
	error_exist_t exist = ERROR_EXIST_NO;
	
	struct error_node_t** curr;
    
	for(curr = &error_handle; *curr; ) 
	{
		struct error_node_t* entry = *curr;
		if (entry) 
		{
			exist = ERROR_EXIST_YES;
			return exist;
		} 
		else
		{
			curr = &entry->next;
		}	
	}
	
	return exist; 
}

/* 插入节点   节点存在ERROR_EXIST_NO 节点不存在返回 ERROR_EXIST_YES */
error_exist_t error_insert(error_id_t id)
{
	error_node_t *error_node_ptr = RT_NULL;
	error_exist_t result = ERROR_EXIST_YES;
	
	error_lock();
	
	error_node_ptr = error_exist(id);
	
    if(RT_NULL != error_node_ptr)
    {
        result = ERROR_EXIST_YES;
		goto __exit;
    }
	else
	{
		error_node_ptr = rt_malloc(sizeof(error_node_t));
		
		if(RT_NULL == error_node_ptr)
		{
			log_e("rt malloc error.");
			result = ERROR_EXIST_NO;
			goto __exit;
		}
		
		error_node_ptr->id = id;
		error_node_ptr->next = error_handle;
		error_handle = error_node_ptr;

		log_e("error insert %d",id);
		
		result = ERROR_EXIST_NO;
		goto __exit;
	} 

__exit:	
	
	error_unlock();
	
	return result;	
}

/* 插入节点   节点存在ERROR_EXIST_NO 节点不存在返回 ERROR_EXIST_YES */
error_exist_t popup_insert(error_id_t id)
{
	error_node_t *error_node_ptr = RT_NULL;
	error_exist_t result = ERROR_EXIST_YES;
	
	error_lock();
	
	error_node_ptr = error_exist(id);
	
    if(RT_NULL != error_node_ptr)
    {
        result = ERROR_EXIST_YES;
		goto __exit;
    }
	else
	{
		error_node_ptr = rt_malloc(sizeof(error_node_t));
		
		if(RT_NULL == error_node_ptr)
		{
			log_e("rt malloc error.");
			result = ERROR_EXIST_NO;
			goto __exit;
		}
		
		error_node_ptr->id = id;
		error_node_ptr->next = error_handle;
		error_handle = error_node_ptr;
 
		log_e("error insert %d",id);
		
		result = ERROR_EXIST_NO;
		goto __exit;
	} 

__exit:	
	
	error_unlock();
	
	return result;	
}

/* 删除节点 节点不存在ERROR_EXIST_NO  节点存在返回 ERROR_EXIST_YES*/
error_exist_t error_delete(error_id_t id)
{
	error_node_t *error_node_ptr = RT_NULL;
	error_exist_t result = ERROR_EXIST_YES;
	
	error_lock();
	
	error_node_ptr = error_exist(id);
	
    if(RT_NULL == error_node_ptr)
    {
        result = ERROR_EXIST_NO;
		goto __exit;
    }
	else
	{
		struct error_node_t** curr;
    
        if(target_tr_read == error_node_ptr)
        {
            target_tr_read = NULL;
        }
        
		for(curr = &error_handle; *curr; ) 
		{
			struct error_node_t* entry = *curr;
			if (entry == error_node_ptr) 
			{
				*curr = entry->next;
			} 
			else
			{
				curr = &entry->next;
			}
		}
		
        log_e("error delete %d",id);
        
		rt_free(error_node_ptr);
		
		result = ERROR_EXIST_YES;
		goto __exit;
	}

__exit:	
	error_unlock();
	
	return result;
}

/* 遍历节点 每次返回一个错误 */
error_id_t error_code_get(void)
{
    error_id_t id = ERROR_ID_NONE;
    
    error_lock();
    
    if(target_tr_read == NULL)
    {
        target_tr_read = error_handle;
    }
    
    if(target_tr_read)
    {
        id = target_tr_read->id;
        
        if(target_tr_read->next)
        {
            target_tr_read = target_tr_read->next;
        }
        else
        {
            target_tr_read = NULL;
        }
    }
    
    if(ERROR_EXIST_NO == is_error_in_buf(id))
    {
        id = ERROR_ID_NONE;
    }
    
    if(id == 255)
    {
        id = ERROR_ID_NONE;
    }
    
    error_unlock();
    
	return id;
}

/*  查询错误等级  */
error_lvl_t error_lvl_get(void)
{
	error_lvl_t lvl = ERROR_LVL_NONE;
	
    error_lock();
    
	/*   首先看有没有错误	*/
	if(ERROR_EXIST_NO == error_find())
	{
		lvl = ERROR_LVL_NONE;
	}
    else if(ERROR_EXIST_YES == is_error_lvl_3_exist())
	{
		lvl = ERROR_LVL_3;
	}
	else if(ERROR_EXIST_YES == is_error_lvl_2_exist())
	{
		lvl = ERROR_LVL_2;
	}
	else if(ERROR_EXIST_YES == is_error_lvl_1_exist())
	{
		lvl = ERROR_LVL_1;
	}
    else
    {
        //log_e("error is not in the list_lvl.");
    }
    
    error_unlock();
	
	return lvl;
}

static void error_print(void)
{
	error_node_t* target = RT_NULL;
	
	target = error_handle;
	
    rt_kprintf("error lvl: %d\r\n", error_lvl_get());
    
    error_lock();
	if(target == NULL)
	{
		rt_kprintf("no error!\r\n");
	}
	else
	{
		rt_kprintf("error id: ", target->id);
		for(; target != NULL; target = target->next)
		{
			rt_kprintf(" %d ", target->id);
		}
		rt_kprintf("\r\n");
	}
    error_unlock();
}

/*  返回第一个error_id */
error_id_t error_id_get(void)
{
    error_id_t error_id = ERROR_ID_NONE;
	error_node_t* target = error_handle;
	
    error_lock();
	if(target == NULL)
	{
        error_unlock();
		return error_id;
	}
	else
	{
		for(; target != NULL; target = target->next)
		{
            error_unlock();
            return error_id = target->id;
		}
	}
    error_unlock();
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>

static void error(uint8_t argc, char **argv)
{
#define CMD_ERROR_PRINT_INDEX                 0
#define CMD_ERROR_SET_INDEX                   1	
	
	const char* help_info[] =
    {
        [CMD_ERROR_PRINT_INDEX]   = "error print                         - all of the error id  print",
        [CMD_ERROR_SET_INDEX]     = "error <insert|delete> <1-255>       - error  set",
    };
	
	int i;
	
    if ((argc != 3) && (argc != 2))
    {
        rt_kprintf("Usage:\n");
        for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            rt_kprintf("%s\n", help_info[i]);
        }
        rt_kprintf("\n");
    }
    else
    {
        const char *operator = argv[1];
		
		if (!strcmp(operator, "print"))
        {
			if (argc != 2)
			{			
				rt_kprintf("Usage: %s.\n", help_info[CMD_ERROR_PRINT_INDEX]);
			}
			else
			{
				error_print();
			}
		}
		else
		{
			if (argc != 3)
			{
				rt_kprintf("Usage: %s.\n", help_info[CMD_ERROR_SET_INDEX]);
			}
			else
			{	
                uint16_t id = strtol(argv[2], NULL, 0);

                if((id > 255) || (0 == id ))
                {
                    rt_kprintf("Usage: %s.\n", help_info[CMD_ERROR_SET_INDEX]);
                }

                if (!strcmp(operator, "insert"))
                {
                    error_insert((error_id_t)id);
                    rt_kprintf("error insert %d\r\n",id);
                }
                else if (!strcmp(operator, "delete"))
                {
                    error_delete((error_id_t)id);
                    rt_kprintf("error delete %d\r\n",id);
                }
                else
                {
                    rt_kprintf("Usage: %s.\n", help_info[CMD_ERROR_SET_INDEX]);
                }
			}
		}
    }
}
MSH_CMD_EXPORT(error,error code set);
#endif

