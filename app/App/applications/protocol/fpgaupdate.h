/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-24     qianqiang    the first version
 */

#ifndef _FPGAUPDATE_H_
#define _FPGAUPDATE_H_

#include <rtthread.h>
#include <ymodem.h>
#include <board.h>

#define CHECK_EVERY_X_BYTE	10240
#define INIT_CYCLE			200

/* AS Instruction Set    */
#define AS_WRITE_ENABLE				0x06
#define AS_WRITE_DISABLE			0x04
#define AS_READ_STATUS	    		0x05
#define AS_WRITE_STATUS	    		0x01
#define AS_READ_BYTES   			0x03
#define AS_FAST_READ_BYTES  		0x0B
#define AS_PAGE_PROGRAM				0x02
#define AS_ERASE_SECTOR				0xD8
#define AS_ERASE_BULK				0xC7
#define AS_READ_SILICON_ID			0xAB
#define AS_CHECK_SILICON_ID			0x9F


/* Silicon ID for EPCS   */
#define EPCS1_ID	0x10
#define EPCS4_ID	0x12
#define EPCS16_ID	0x14
#define EPCS64_ID	0x16
#define EPCS128_ID	0x18


/* EPCS device			 */
#define EPCS1		1	
#define EPCS4		4	
#define EPCS16		16	
#define EPCS64		64
#define EPCS128		128

#define DEV_READBACK   0xFF //Special bypass indicator during EPCS data readback	

/* Error Code Start */
#define	CB_OK								0

#define CB_FS_OPEN_FILE_ERROR				-1
#define CB_FS_CLOSE_FILE_ERROR				-2
#define CB_FS_SIZE_EOF_NOT_FOUND			-3
#define CB_FS_READ_ERROR					-4

#define CB_BB_OPEN_ERROR_OPEN_PORT			-5
#define CB_BB_OPEN_VERSION_INCOMPATIBLE		-6
#define CB_BB_OPEN_DRIVER_INCOMPATIBLE		-7
#define CB_BB_OPEN_DEVICEIOCONTROL_FAIL		-8
#define CB_BB_CLOSE_BYTEBLASTER_NOT_OPEN	-9
#define CB_BB_FLUSH_ERROR					-10
#define CB_BB_VERIFY_BYTEBLASTER_NOT_FOUND	-11
#define CB_BB_LPTREAD_ERROR					-12
#define CB_BB_LPTWRITE_ERROR				-13

#define CB_PS_CONF_NSTATUS_LOW				-14
#define CB_PS_CONF_CONFDONE_LOW				-15
#define CB_PS_INIT_NSTATUS_LOW				-16
#define CB_PS_INIT_CONFDONE_LOW				-17

#define CB_AS_VERIFY_FAIL					-18		
#define CB_AS_UNSUPPORTED_DEVICE			-19 
#define CB_AS_WRONG_RPD_FILE				-20

#define CB_INVALID_NUMBER_OF_ARGUMENTS		-21
#define CB_INVALID_COMMAND					-22
#define CB_INVALID_EPCS_DENSITY				-23

/* Error Code END  */

void gpio_led_init(void);
void gpio_update_init(void);
void as_program_start( void );
void as_program_done(void);
int as_bulk_erase( void );
rt_uint8_t as_prog( rt_uint8_t* , int ,int );
int as_silicon_id(int, int);
rt_uint8_t as_program_byte_lsb( rt_uint8_t );
rt_uint8_t as_read_byte_lsb( rt_uint8_t* );
rt_uint8_t as_program_byte_msb( rt_uint8_t );
rt_uint8_t as_read_byte_msb( rt_uint8_t* );
rt_uint8_t as_verify( rt_uint8_t * , int , int );
void as_lsb_to_msb( rt_uint8_t *, rt_uint8_t *);
#endif



