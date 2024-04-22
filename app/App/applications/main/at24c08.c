/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-1-30     qq           first version
 *
 * AT24C08:
 * GND_V__A0  VCC___VCC3V3
 * GND_V__A1  WP___EE_WP
 * GND_V__A2  SCL__EE_SCL
 * GND_V__GND SDA__EE_SDA
 *
 * EE_WP---------->PB7
 * EE_SCL--------->PB8
 * EE_SDA--------->PB9
 */
#define LOG_TAG   "AT24C08"  
#define LOG_LVL   LOG_LVL_DBG 

#include "at24c08.h"

static at24cxx_device_t dev = RT_NULL;

//��ʼ��IIC�ӿ�
void AT24CXX_Init(void)
{
    rt_pin_mode(I2C1_WP, PIN_MODE_OUTPUT);
    rt_pin_write(I2C1_WP, PIN_LOW);
    dev = at24cxx_init("i2c1",0x50);
}

//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������
uint8_t AT24CXX_ReadOneByte(uint32_t ReadAddr)
{				
    uint8_t temp=0;    
	at24cxx_read(dev, ReadAddr, &temp, 1);
    return temp;
}

//��AT24CXXָ����ַд������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//pBuffer:Ҫд�������
//NumToWrite:Ҫд��������ֽ���
void AT24CXX_Read(uint32_t ReadAddr, uint8_t *pBuffer, uint16_t NumToRead)
{	
   at24cxx_read(dev, ReadAddr, pBuffer, NumToRead);
}

//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(uint32_t WriteAddr, uint8_t pBuffer)
{	
   uint8_t pdate = pBuffer;
   at24cxx_write(dev, WriteAddr, &pdate, 1);
}

//��AT24CXXָ����ַд������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//pBuffer:Ҫд�������
//NumToWrite:Ҫд��������ֽ���
void AT24CXX_Write(uint32_t WriteAddr, uint8_t *pBuffer, uint16_t NumToWrite)
{	
   at24cxx_write(dev, WriteAddr, pBuffer, NumToWrite);
}
