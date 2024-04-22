/*
 * @Description: 
 * @Version: 1.0
 * @Autor: JunQi Liu
 * @Date: 2020-06-26 23:10:43
 * @LastEditors: JunQi Liu
 * @LastEditTime: 2020-06-26 23:14:19
 * @FilePath: \STM32-RTThread-BootLoader\applications\spi_flash_init.c
 */ 
#include <rtthread.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"

/**
 * @description: 添加 spi flash 的初始化
 * @param {type} 
 * @return: 
 * @note: CS 引脚为 PA15
 * @author: JunQi Liu
 */
#if defined(BSP_USING_SPI_FLASH)
static int rt_hw_spi_flash_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    rt_hw_spi_device_attach("spi3", "spi30", GPIOA, GPIO_PIN_15);

    if (RT_NULL == rt_sfud_flash_probe("W25Q128", "spi30"))
    {
        return -RT_ERROR;
    };

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_spi_flash_init);
#endif
