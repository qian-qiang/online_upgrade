/*
 * Copyright (c) 2019, Anke Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-10-31     qq           the first version
 */
 
#define LOG_TAG      "fpgaupdate.file"
#define LOG_LVL      LOG_LVL_INFO

#include <ulog.h>
#include <rthw.h>
#include "fpgaupdate.h"
/*
#define CS 			 PAout(4)
#define NCONFIG		 PGout(4)
#define NCE 		 PGout(3)
#define EP_CLK       PAout(5)
#define EP_ASDI      PAout(7)
#define EP_DATA      PAin(6)
*/

/*注意ECPS读写关键字/命令是MSB在前，读写数据是LSB在前*/

int EPCS_device = 0;
/**
 * @brief 将输入字节的 LSB (Least Significant Bit，最低有效位) 转换为 MSB (Most Significant Bit，最高有效位)
 * 
 * @param in_byte 输入字节的指针
 * @param out_byte 输出字节的指针
 * 
 * @note 该函数将输入字节的 LSB 转换为 MSB，并将结果存储在输出字节中。
 *       这对于在字节级别上进行大小端转换或者位级别的操作非常有用。
 */
void as_lsb_to_msb( rt_uint8_t *in_byte, rt_uint8_t *out_byte)
{
	rt_uint8_t	mask;
	rt_uint8_t	i;
	rt_uint8_t  temp;
	*out_byte = 0x00;
	for ( i = 0; i < 8; i++ )
	{	
		temp = *in_byte >> i;
		mask = 0x80 >> i;
		if(temp & 0x01)*out_byte |= mask;	//if lsb is set inbyte, set msb for outbyte
	}
}

/**
 * @brief 启动 FPGA 程序下载前的初始化操作
 * 
 * @details
 * 在开始下载 FPGA 程序之前，需要进行一些初始化操作，确保下载过程顺利进行：
 * - 将 NCONFIG 引脚置低：将 FPGA 的配置引脚（NCONFIG）拉低，以确保 FPGA 在下载前被重置。
 * - 将 NCE 引脚置高：将 FPGA 的片选引脚（NCE）置高，以禁止 FPGA 访问 EPCS。
 * - 将 CS 引脚置高：将 EPCS 的片选引脚（CS）置高，表示当前不访问 EPCS。
 */
void as_program_start(void)
{
	rt_pin_write(NCONFIG,PIN_LOW);	// Drive NCONFIG to reset FPGA before programming EPCS
    log_i("as_program_start: NCONFIG set low");
	rt_pin_write(NCE,PIN_HIGH);		// Drive NCE to disable FPGA from accessing EPCS
    log_i("as_program_start: NCE set high");
	rt_pin_write(EP_CS,PIN_HIGH);	// Drive NCS to high when not acessing EPCS
    log_i("as_program_start: EP_CS set high");
}

/**
 * @brief FPGA 程序下载完成后的清理操作
 * 
 * @details
 * 在 FPGA 程序下载完成后，需要进行一些清理操作以恢复引脚状态：
 * - 将 NCE 引脚置低：使能 FPGA，允许其访问外部设备。
 * - 将 NCONFIG 引脚从低到高：将 FPGA 的配置引脚（NCONFIG）从低电平转换为高电平，完成 FPGA 的复位操作。
 * - 将 CS 引脚置高：将 EPCS 的片选引脚（CS）置高，表示当前不访问 EPCS。
 */
void as_program_done(void)
{
    rt_pin_write(NCONFIG,PIN_HIGH);	// Drive NCONFIG from low to high to reset FPGA
    log_i("as_program_start: NCONFIG set high");
	rt_pin_write(NCE,PIN_LOW);		// Drive NCE to enable FPGA
    log_i("as_program_start: NCE set low");
	rt_pin_write(EP_CS,PIN_HIGH);		// Drive NCS to high when not acessing EPCS
    log_i("as_program_start: EP_CS set high");
}

/**
 * @brief 将数据编程到外部设备中
 * 
 * @param file 数据的指针，待编程的数据
 * @param file_size 数据的大小，以字节为单位
 * @param EPCS_Address 编程的起始地址
 * 
 * @return 返回操作状态，成功返回 CB_OK，失败返回其他错误码
 * 
 * @details
 * 这个函数用于将数据编程到外部设备（如 EPCS）中。它将数据分页编程到指定地址范围内的存储器中，
 * 并在每页编程后执行状态检查以确保编程完成。
 * - 首先计算需要编程的页数和最后一页的字节数。
 * - 然后循环处理每一页的数据，依次编程到指定地址中，并在编程完成后进行状态检查。
 * - 如果在最后一页有余下的字节（不足一页），则只编程余下的字节，而不是整页。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
rt_uint8_t as_prog( rt_uint8_t * file, int file_size ,int EPCS_Address)
{
	int	page=0;
	rt_uint8_t StatusReg =0;
	int temp_Address=0;
	int	i,j;		
	int bal_byte = 0;
	int byte_per_page = 256;

	temp_Address=EPCS_Address;
	page = file_size/256;
	bal_byte = file_size%256;	
	
	if(bal_byte) //if there is balance after divide, program the balance in the next page
	{
		page++;
	}
	for(i=0; i<page; i++ )
	{
		rt_pin_write(EP_CS,PIN_LOW);
		as_program_byte_msb( AS_WRITE_ENABLE );
		rt_pin_write(EP_CS,PIN_HIGH);
		
        rt_pin_write(EP_CS,PIN_LOW);
        as_program_byte_msb( AS_PAGE_PROGRAM );
		
		as_program_byte_msb( ((temp_Address & 0xFF0000)>>16));
		as_program_byte_msb( ((temp_Address & 0x00FF00)>>8) );
		as_program_byte_msb( temp_Address & 0xFF);
		temp_Address = temp_Address + 256;

		if((i == (page - 1)) && (bal_byte != 0))	//if the last page has has been truncated less than 256
			byte_per_page = bal_byte;
		
		for(j=0; j<byte_per_page; j++)as_program_byte_lsb(*(file+j+i*256));
		rt_pin_write(EP_CS,PIN_HIGH);
		rt_pin_write(EP_CS,PIN_LOW);
		as_program_byte_msb( AS_READ_STATUS );	
		as_read_byte_msb(&StatusReg);
		while((StatusReg & 0x01))
		{
			as_read_byte_msb(&StatusReg);
		}

		rt_pin_write(EP_CS,PIN_HIGH);
		
	}
	return CB_OK;
}

/**
 * @brief 从外部设备中读取一个字节数据（MSB顺序）
 * 
 * @param one_byte 用于存储读取到的字节数据的指针
 * 
 * @return 返回操作状态，通常为 CB_OK 表示成功
 * 
 * @details
 * 该函数从外部设备（如 EPCS）中读取一个字节的数据，并通过函数 as_lsb_to_msb 进行 LSB 到 MSB 的转换，
 * 然后将转换后的结果存储到指定的地址中。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
rt_uint8_t as_read_byte_msb( rt_uint8_t *one_byte )
{
	rt_uint8_t data_byte = 0;
	as_read_byte_lsb(&data_byte);
	//After conversion, MSB will come in first
	as_lsb_to_msb(&data_byte, one_byte);	
	return CB_OK;	
}

rt_uint8_t as_read_byte_lsb( rt_uint8_t *one_byte )
{
	rt_uint8_t	bit = 0;
	rt_uint8_t mask = 0x01;
	rt_uint8_t i;
    rt_uint16_t delay;
	*one_byte = 0;
	// read from from LSB to MSB 
	for ( i = 0; i < 8; i++ )
	{
		rt_pin_write(EP_CLK,PIN_LOW);	
		rt_pin_write(EP_CLK,PIN_HIGH);
		bit=rt_pin_read(EP_DATA);;
		if (bit!=0) //if bit is true
			*one_byte |= (mask << i); 
	}
		
	return CB_OK;	
}

/**
 * @brief 将指定的字节数据（MSB顺序）写入外部设备
 * 
 * @param one_byte 待写入的字节数据（MSB顺序）
 * 
 * @return 返回操作状态，通常为 CB_OK 表示成功
 * 
 * @details
 * 该函数用于将指定的字节数据（MSB顺序）写入外部设备（如 EPCS）。在写入之前，它将调用 as_lsb_to_msb 函数
 * 将 MSB 顺序的数据转换为 LSB 顺序，然后调用 as_program_byte_lsb 函数将转换后的数据写入外部设备。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
rt_uint8_t as_program_byte_msb( rt_uint8_t one_byte )
{

	rt_uint8_t data_byte = 0;
	//Convert MSB to LSB before programming
	as_lsb_to_msb(&one_byte, &data_byte);
	as_program_byte_lsb(data_byte); 
	
	return CB_OK;
}

/**
 * @brief 将指定的字节数据（LSB顺序）写入外部设备
 * 
 * @param one_byte 待写入的字节数据（LSB顺序）
 * 
 * @return 返回操作状态，通常为 CB_OK 表示成功
 * 
 * @details
 * 该函数用于将指定的字节数据（LSB顺序）写入外部设备（如 EPCS）。它通过从 LSB 到 MSB 的顺序逐位写入数据，
 * 先将数据的每个位从右向左取出，然后将位值写入到外部设备的相应引脚（EP_ASDI），并在每次写入后产生一个
 * 时钟脉冲（EP_CLK）使得外部设备接收数据。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
rt_uint8_t as_program_byte_lsb( rt_uint8_t one_byte )
{
	rt_uint8_t	bit = 0;
	rt_uint8_t i = 0;
	rt_uint16_t delay;
	// write from LSB to MSB 
	for ( i = 0; i < 8; i++ )
	{
		bit = one_byte >> i;
		bit = bit & 0x1;
		rt_pin_write(EP_CLK,PIN_LOW);
        rt_pin_write(EP_ASDI,bit);
		rt_pin_write(EP_CLK,PIN_HIGH);
	}
	return CB_OK;
}

/**
 * @brief 对外部设备执行批量擦除操作
 * 
 * @return 返回操作状态，通常为 CB_OK 表示成功
 * 
 * @details
 * 该函数用于对外部设备（如 EPCS）执行批量擦除操作。它通过将相应的命令发送到外部设备，
 * 然后轮询状态寄存器以等待擦除操作完成。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
int as_bulk_erase( void )
{
	rt_uint8_t StatusReg =0;

	rt_pin_write(EP_CS,PIN_LOW);
    as_program_byte_msb(AS_WRITE_ENABLE);
	rt_pin_write(EP_CS,PIN_HIGH);
	rt_pin_write(EP_CS,PIN_LOW);
    as_program_byte_msb(AS_ERASE_BULK);	 
	rt_pin_write(EP_CS,PIN_HIGH);
	rt_pin_write(EP_CS,PIN_LOW);
    as_program_byte_msb(AS_READ_STATUS);
	as_read_byte_msb(&StatusReg);
	while((StatusReg & 0x01)) 
        as_read_byte_msb(&StatusReg);	//Keep on polling if the WIP is high
	rt_pin_write(EP_CS,PIN_HIGH);
	return CB_OK;
}

/**
 * @brief 验证外部设备中的数据与给定数据是否一致
 * 
 * @param file 待验证的数据的指针
 * @param file_size 待验证的数据的大小，以字节为单位
 * @param EPCS_Address 验证数据在外部设备中的起始地址
 * 
 * @return 如果外部设备中的数据与给定数据一致，则返回0；否则返回1
 * 
 * @details
 * 该函数用于验证外部设备（如 EPCS）中的数据与给定数据是否一致。它首先从外部设备中读取相应地址的数据，
 * 然后逐个比较读取到的数据与给定数据的每个字节，如果存在不一致的情况，则返回1；否则返回0。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
rt_uint8_t as_verify( rt_uint8_t * file, int file_size,int EPCS_Address)
{
    int i;
    rt_uint8_t read_byte =0;
    rt_pin_write(EP_CS,PIN_LOW);
    as_program_byte_msb( AS_READ_BYTES );	
    as_program_byte_msb( ((EPCS_Address & 0xFF0000)>>16));
    as_program_byte_msb( ((EPCS_Address & 0x00FF00)>>8) );
    as_program_byte_msb( EPCS_Address & 0xFF);
    //rt_kprintf("as_verify:fpga falsh read adress 0x%x len %d:",EPCS_Address, file_size);
    for(i=0; i<file_size; i++)
    {
        as_read_byte_lsb(&read_byte);
        //到时候在读出校验时，就得与重排后的比较
        //rt_kprintf("0x%x ",read_byte);
        if(read_byte !=*(file+i))
        {
            //rt_kprintf("adress 0x%x date 0x%x is error\r\n",EPCS_Address+i, read_byte);
            return 1; 	
        }		     
    }
    rt_pin_write(EP_CS,PIN_HIGH);
    //rt_kprintf("\r\n");
    return 0;
}

/**
 * @brief 读取外部设备的硅片识别码并确定设备类型
 * 
 * @param file_size 外部设备的大小，以字节为单位
 * @param epcsDensity 外部设备的密度，以 Mb 为单位
 * 
 * @return 返回操作状态，通常为 CB_OK 表示成功
 * 
 * @details
 * 该函数用于读取外部设备（如 EPCS）的硅片识别码，并根据识别码确定设备类型。它首先发送读取硅片识别码的命令，
 * 然后根据设备的密度（如 EPCS1、EPCS4、EPCS16 等）选择性地发送相应数量的 Dummy Bytes，最后读取硅片识别码并
 * 根据识别码确定设备类型，并将其存储到全局变量 EPCS_device 中。
 * 
 * 注意：在调用此函数之前，请确保已经通过相应的初始化函数设置了正确的引脚状态。
 */
int as_silicon_id(int file_size, int epcsDensity)
{
	rt_uint8_t silicon_ID = 0;
    rt_pin_write(EP_CS,PIN_LOW);
    if (epcsDensity != 128)		//for EPCS1, EPCS4, EPCS16, EPCS64
    {
         as_program_byte_msb( AS_READ_SILICON_ID );
         as_program_byte_msb(0x00);		//3 Dummy bytes
         as_program_byte_msb(0x00);
         as_program_byte_msb(0x00);
    }
    as_read_byte_msb(&silicon_ID);
    rt_pin_write(EP_CS,PIN_HIGH);
    if(silicon_ID == EPCS1_ID)
    {
        EPCS_device = EPCS1;
        rt_kprintf("1");
    }
    else if(silicon_ID == EPCS4_ID)
    {
        EPCS_device = EPCS4;
        rt_kprintf("4");
    }
    else if(silicon_ID == EPCS16_ID)
    {
        EPCS_device = EPCS16;
        rt_kprintf("16");
    }
    else if(silicon_ID == EPCS64_ID)
    {
        EPCS_device = EPCS64;
        rt_kprintf("64");
    }
    else if(silicon_ID == EPCS128_ID)
    {
        EPCS_device = EPCS128;
        rt_kprintf("128");
    }
    else
    {
        rt_kprintf("NON");
    }
	return CB_OK;
}

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void ff_read(uint8_t argc, char **argv)
{
    rt_uint32_t address;
    rt_uint32_t len;
    
    if(argc != 3)
    {
        rt_kprintf("cmd error . please eg : ff_read [address] [len]");   
    }
    else{
        address = atoi(argv[1]);
        len = atoi(argv[2]);
        rt_uint8_t read_byte =0;
        rt_pin_write(EP_CS,PIN_LOW);
        as_program_byte_msb( AS_READ_BYTES );	
        as_program_byte_msb( ((address & 0xFF0000)>>16));
        as_program_byte_msb( ((address & 0x00FF00)>>8) );
        as_program_byte_msb( address & 0xFF);
        rt_kprintf("fpga falsh read adress 0x%x len %d:",address, len);
        for(int i=0; i<len; i++)
        {
            as_read_byte_lsb(&read_byte);
            rt_kprintf("0x%x ",read_byte);
        }
        rt_pin_write(EP_CS,PIN_HIGH);
        rt_kprintf("\r\n");
    }
}
MSH_CMD_EXPORT(ff_read, fpga flash read);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */