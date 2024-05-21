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

/*ע��ECPS��д�ؼ���/������MSB��ǰ����д������LSB��ǰ*/

int EPCS_device = 0;
/**
 * @brief �������ֽڵ� LSB (Least Significant Bit�������Чλ) ת��Ϊ MSB (Most Significant Bit�������Чλ)
 * 
 * @param in_byte �����ֽڵ�ָ��
 * @param out_byte ����ֽڵ�ָ��
 * 
 * @note �ú����������ֽڵ� LSB ת��Ϊ MSB����������洢������ֽ��С�
 *       ��������ֽڼ����Ͻ��д�С��ת������λ����Ĳ����ǳ����á�
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
 * @brief ���� FPGA ��������ǰ�ĳ�ʼ������
 * 
 * @details
 * �ڿ�ʼ���� FPGA ����֮ǰ����Ҫ����һЩ��ʼ��������ȷ�����ع���˳�����У�
 * - �� NCONFIG �����õͣ��� FPGA ���������ţ�NCONFIG�����ͣ���ȷ�� FPGA ������ǰ�����á�
 * - �� NCE �����øߣ��� FPGA ��Ƭѡ���ţ�NCE���øߣ��Խ�ֹ FPGA ���� EPCS��
 * - �� CS �����øߣ��� EPCS ��Ƭѡ���ţ�CS���øߣ���ʾ��ǰ������ EPCS��
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
 * @brief FPGA ����������ɺ���������
 * 
 * @details
 * �� FPGA ����������ɺ���Ҫ����һЩ��������Իָ�����״̬��
 * - �� NCE �����õͣ�ʹ�� FPGA������������ⲿ�豸��
 * - �� NCONFIG ���Ŵӵ͵��ߣ��� FPGA ���������ţ�NCONFIG���ӵ͵�ƽת��Ϊ�ߵ�ƽ����� FPGA �ĸ�λ������
 * - �� CS �����øߣ��� EPCS ��Ƭѡ���ţ�CS���øߣ���ʾ��ǰ������ EPCS��
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
 * @brief �����ݱ�̵��ⲿ�豸��
 * 
 * @param file ���ݵ�ָ�룬����̵�����
 * @param file_size ���ݵĴ�С�����ֽ�Ϊ��λ
 * @param EPCS_Address ��̵���ʼ��ַ
 * 
 * @return ���ز���״̬���ɹ����� CB_OK��ʧ�ܷ�������������
 * 
 * @details
 * ����������ڽ����ݱ�̵��ⲿ�豸���� EPCS���С��������ݷ�ҳ��̵�ָ����ַ��Χ�ڵĴ洢���У�
 * ����ÿҳ��̺�ִ��״̬�����ȷ�������ɡ�
 * - ���ȼ�����Ҫ��̵�ҳ�������һҳ���ֽ�����
 * - Ȼ��ѭ������ÿһҳ�����ݣ����α�̵�ָ����ַ�У����ڱ����ɺ����״̬��顣
 * - ��������һҳ�����µ��ֽڣ�����һҳ������ֻ������µ��ֽڣ���������ҳ��
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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
 * @brief ���ⲿ�豸�ж�ȡһ���ֽ����ݣ�MSB˳��
 * 
 * @param one_byte ���ڴ洢��ȡ�����ֽ����ݵ�ָ��
 * 
 * @return ���ز���״̬��ͨ��Ϊ CB_OK ��ʾ�ɹ�
 * 
 * @details
 * �ú������ⲿ�豸���� EPCS���ж�ȡһ���ֽڵ����ݣ���ͨ������ as_lsb_to_msb ���� LSB �� MSB ��ת����
 * Ȼ��ת����Ľ���洢��ָ���ĵ�ַ�С�
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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
 * @brief ��ָ�����ֽ����ݣ�MSB˳��д���ⲿ�豸
 * 
 * @param one_byte ��д����ֽ����ݣ�MSB˳��
 * 
 * @return ���ز���״̬��ͨ��Ϊ CB_OK ��ʾ�ɹ�
 * 
 * @details
 * �ú������ڽ�ָ�����ֽ����ݣ�MSB˳��д���ⲿ�豸���� EPCS������д��֮ǰ���������� as_lsb_to_msb ����
 * �� MSB ˳�������ת��Ϊ LSB ˳��Ȼ����� as_program_byte_lsb ������ת���������д���ⲿ�豸��
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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
 * @brief ��ָ�����ֽ����ݣ�LSB˳��д���ⲿ�豸
 * 
 * @param one_byte ��д����ֽ����ݣ�LSB˳��
 * 
 * @return ���ز���״̬��ͨ��Ϊ CB_OK ��ʾ�ɹ�
 * 
 * @details
 * �ú������ڽ�ָ�����ֽ����ݣ�LSB˳��д���ⲿ�豸���� EPCS������ͨ���� LSB �� MSB ��˳����λд�����ݣ�
 * �Ƚ����ݵ�ÿ��λ��������ȡ����Ȼ��λֵд�뵽�ⲿ�豸����Ӧ���ţ�EP_ASDI��������ÿ��д������һ��
 * ʱ�����壨EP_CLK��ʹ���ⲿ�豸�������ݡ�
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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
 * @brief ���ⲿ�豸ִ��������������
 * 
 * @return ���ز���״̬��ͨ��Ϊ CB_OK ��ʾ�ɹ�
 * 
 * @details
 * �ú������ڶ��ⲿ�豸���� EPCS��ִ������������������ͨ������Ӧ������͵��ⲿ�豸��
 * Ȼ����ѯ״̬�Ĵ����Եȴ�����������ɡ�
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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
 * @brief ��֤�ⲿ�豸�е���������������Ƿ�һ��
 * 
 * @param file ����֤�����ݵ�ָ��
 * @param file_size ����֤�����ݵĴ�С�����ֽ�Ϊ��λ
 * @param EPCS_Address ��֤�������ⲿ�豸�е���ʼ��ַ
 * 
 * @return ����ⲿ�豸�е��������������һ�£��򷵻�0�����򷵻�1
 * 
 * @details
 * �ú���������֤�ⲿ�豸���� EPCS���е���������������Ƿ�һ�¡������ȴ��ⲿ�豸�ж�ȡ��Ӧ��ַ�����ݣ�
 * Ȼ������Ƚ϶�ȡ����������������ݵ�ÿ���ֽڣ�������ڲ�һ�µ�������򷵻�1�����򷵻�0��
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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
        //��ʱ���ڶ���У��ʱ���͵������ź�ıȽ�
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
 * @brief ��ȡ�ⲿ�豸�Ĺ�Ƭʶ���벢ȷ���豸����
 * 
 * @param file_size �ⲿ�豸�Ĵ�С�����ֽ�Ϊ��λ
 * @param epcsDensity �ⲿ�豸���ܶȣ��� Mb Ϊ��λ
 * 
 * @return ���ز���״̬��ͨ��Ϊ CB_OK ��ʾ�ɹ�
 * 
 * @details
 * �ú������ڶ�ȡ�ⲿ�豸���� EPCS���Ĺ�Ƭʶ���룬������ʶ����ȷ���豸���͡������ȷ��Ͷ�ȡ��Ƭʶ��������
 * Ȼ������豸���ܶȣ��� EPCS1��EPCS4��EPCS16 �ȣ�ѡ���Եط�����Ӧ������ Dummy Bytes������ȡ��Ƭʶ���벢
 * ����ʶ����ȷ���豸���ͣ�������洢��ȫ�ֱ��� EPCS_device �С�
 * 
 * ע�⣺�ڵ��ô˺���֮ǰ����ȷ���Ѿ�ͨ����Ӧ�ĳ�ʼ��������������ȷ������״̬��
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