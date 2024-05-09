
/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 * CONFIG_DONE  PGout(5)
 * NCONFIG		PGout(4)
 * NCE 			PGout(3)
 * EP_CLK       PAout(5)
 * EP_ASDI      PAout(7)     发送数据
 * EP_DATA      PAin(6)      接收数据
 * EP_CS        PAout(4)
 */
#define LOG_TAG   "fpga"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "fpga.h"
#include "drv_spi.h"
#include "stm32f4xx_hal_spi.h"

#define FPGA_SPI_DEVICE_NAME     "spi20"
struct rt_spi_device *spi_dev_fpga;

#define RT_FPGA_DEFAULT_SPI_CFG                             \
{                                                           \
    .mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB,     \
    .data_width = 16,                                       \
    .max_hz = 5 * 1000 * 1000,                             \
}

static int spi_fpga_init(void)
{
    //CONFIG_DONE  NCONFIG不需要在这里初始化 否则上电fpga有问题  
    //因为上电fpga下载引脚是有时序要求的  引脚在cubemx中初始化了
    rt_pin_mode(NCE, PIN_MODE_OUTPUT);
    rt_pin_mode(EP_CLK, PIN_MODE_OUTPUT);
    rt_pin_mode(EP_ASDI, PIN_MODE_OUTPUT);
    rt_pin_mode(EP_CS, PIN_MODE_OUTPUT);
   
    struct rt_spi_configuration cfg = RT_FPGA_DEFAULT_SPI_CFG;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    rt_hw_spi_device_attach("spi2", "spi20", GPIOB, GPIO_PIN_12);
    rt_pin_mode(SPI2_CS, PIN_MODE_OUTPUT);
    
    spi_dev_fpga = (struct rt_spi_device *)rt_device_find(FPGA_SPI_DEVICE_NAME);
    if(spi_dev_fpga == RT_NULL)
    {
        log_e("spi_dev_fpga spi device not find.");
        return -RT_ERROR;
    }
    spi_dev_fpga->bus->owner = spi_dev_fpga;   
    rt_spi_configure(spi_dev_fpga, &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(spi_fpga_init);
//spitools init spi20 spi2 28

//以下是SPI模块的初始化代码，配置成主机模式 						  
//SPI口初始化
//这里针是对SPI1的初始化
void fpga_spi2_init(void)
{	 
    rt_uint16_t Value = 0xffff;
    rt_spi_send(spi_dev_fpga, &Value, 1); 	 
}   

//SPI1速度设置函数
//SPI速度=fAPB2/分频系数
//@ref SPI_BaudRate_Prescaler:SPI_BaudRatePrescaler_2~SPI_BaudRatePrescaler_256  
//fAPB2时钟一般为84Mhz：
void SPI2_SetSpeed(rt_uint8_t SPI_BaudRatePrescaler)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
	SPI2->CR1&=0XFFC7;//位3-5清零，用来设置波特率
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI1速度 
} 

/********************************************************************************************
函数名：FPGA_Read
功能  ：从FPGA读数据
参数  ：ReadAddr 地址位
返回值：Value 数据值
*********************************************************************************************/ 
rt_uint16_t FPGA_Read(rt_uint16_t ReadAddr)   
{  
	rt_uint16_t Value = 0;
		
    rt_pin_write(SPI2_CS, PIN_LOW);
    rt_spi_send_then_recv(spi_dev_fpga, &ReadAddr, 1, &Value, 1);
    rt_pin_write(SPI2_CS, PIN_HIGH);
	return Value;
}  

/********************************************************************************************
函数名：FPGA_Write
功能  ：往FPGA写数据
参数  ：WriteAddr 地址，Data数据
返回值：无
*********************************************************************************************/ 
void FPGA_Write(rt_uint16_t WriteAddr, rt_uint16_t Data)
{	
    rt_pin_write(SPI2_CS, PIN_LOW);
    rt_spi_send_then_send(spi_dev_fpga, &WriteAddr, 1, &Data, 1);
    rt_pin_write(SPI2_CS, PIN_HIGH);
}

/********************************************************************************************
函数名：ACTIVE_WR_FPGA
功能  ：往FPGA写入数据并进行读取
参数  ：无
返回值：无
*********************************************************************************************/ 
void ACTIVE_WR_FPGA(void)
{
	FPGA_Write(Burst_Num_W, GD_BUF[BURST_NUM]); 
	FPGA_Write(FQ_RF_SCALE_W, GD_BUF[FQ_RF_SCALE]); 
	FPGA_Write(FQ_FRE_W, GD_BUF[FQ_FRE]); 
	FPGA_Write(SQ_RF_SCALE_W, aom2RfSet(GD_BUF[SQ_RF_SCALE]) ); 
	FPGA_Write(Mode_W, GD_BUF[LASER_MODE]);
	FPGA_Write(Mode_SW_W, GD_BUF[MODE_SW]);
	//FPGA_Write(DELAY_W, GD_BUF[FQ_DELAY]);
	FPGA_Write(EXT_MODE_W, GD_BUF[EXT_MODE]);
	FPGA_Write(MODULO_DIVIDER_W, GD_BUF[MODULO_DIVIDER]);  
	FPGA_Write(BURST_HOLDOFF_W, GD_BUF[BURST_HOLDOFF]);  
	FPGA_Write(BURST_LENGTH_W, GD_BUF[BURST_LENGTH]);
	FPGA_Write(EXT_MODE_ACTLEVEL_W, GD_BUF[EXT_MODE_ACTLEVEL]);
	//FPGA_Write(SQ_ADD_TIME_W, GD_BUF[SQ_ADD_TIME]);
	
	
	GD_BUF[BURST_NUM]	        = FPGA_Read(Burst_Num_R);	
	GD_BUF[FQ_RF_SCALE]         = FPGA_Read(FQ_RF_SCALE_R); 
	GD_BUF[FQ_FRE]              = FPGA_Read(FQ_FRE_R);
    GD_BUF[MODE_SW]             = FPGA_Read(Mode_SW_R);	
	GD_BUF[LASER_MODE]          = FPGA_Read(Mode_R);		 
	//GD_BUF[FQ_DELAY]            = FPGA_Read(DELAY_R); 
	GD_BUF[EXT_MODE]            = FPGA_Read(EXT_MODE_R);  
    GD_BUF[MODULO_DIVIDER]      = FPGA_Read(MODULO_DIVIDER_R);
	GD_BUF[BURST_HOLDOFF]       = FPGA_Read(BURST_HOLDOFF_R);
	GD_BUF[BURST_LENGTH]        = FPGA_Read(BURST_LENGTH_R);
	GD_BUF[EXT_MODE_ACTLEVEL]   = FPGA_Read(EXT_MODE_ACTLEVEL_R);
	//GD_BUF[SQ_ADD_TIME]         = FPGA_Read(SQ_ADD_TIME_R);
	GD_BUF[FPGA_VERS]           = FPGA_Read(FPAG_VERS_R);
} 

