/**
  ******************************************************************************
  * @file    drv_packer.c
  * @author  zpw
  * @version V1.0
  * @date    
  * @brief   链路层通讯协议
  ******************************************************************************
  * @attention
  *          链路层通讯协议，数据封包解包
  ******************************************************************************
  */
#define LOG_TAG      "uart.upk"
#define LOG_LVL      LOG_LVL_DBG 
  
#include "upacker.h"
#include "ulog.h"
#include "protocol_id.h"

static uint8_t frame_decode(upacker_inst_t packer, uint8_t d);
static uint8_t frame_encode(upacker_inst_t packer, uint8_t *data, uint16_t size);

const uint8_t auch_crc_hi[]=  
{  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,  
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,  
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,  
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,  
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,  
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40  
};  
  
const uint8_t auch_crc_lo[] =  
{  
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,  
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,  
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,  
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,  
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,  
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,  
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,  
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,  
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,  
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,  
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,  
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,  
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,  
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,  
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,  
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,  
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,  
0x40  
};  

uint8_t crc_16(uint8_t *buf, uint16_t len)
{
//	uint8_t crc_hi=0xff;  
//	uint8_t crc_lo=0xff;  
//	uint16_t index;  
//	uint16_t value;
//	while(len--)  
//	{  
//		index = crc_hi ^* buf++;  
//		crc_hi = crc_lo ^ auch_crc_hi[index];  
//		crc_lo = auch_crc_lo[index];  
//	}  
//	value = crc_hi;
//	value <<= 8;
//	value |= crc_lo;
//	
//    return value; 
    
    	uint8_t tmp=0xff;
        tmp=tmp^0xff;
		tmp=tmp^buf[0];
		tmp=tmp^buf[1];
		tmp=tmp^buf[2];
		tmp=tmp^buf[3];
		tmp=tmp^buf[4];
		tmp=tmp^buf[5];
		tmp=tmp^0xa2;
		return tmp;
}

/**
 * @brief  使用动态内存时需要初始化
 * @note   size pack缓存的长度，大于最大的数据包长度就行,使用PACK_SIZE
            无rtos最好用静态内存,不然要改heap
 * @param  *cmd_packer: 
 * @param  *handler: 
 * @retval None
 */
int upacker_init(char* name, upacker_inst_t packer, PACKER_CB h, PACKER_CB s)
{

#if USE_DYNAMIC_MEM
    packer->data = (uint8_t *)UP_MALLOC(MAX_PACK_SIZE);
    if (!packer->data)
    {
        return -1;
    }
#endif

    packer->cb = h;
    packer->send = s;

    packer->sem_pack = rt_sem_create(name, 1, RT_IPC_FLAG_PRIO);
    
    return 0;
}

/**
 * @brief  解包输入数据
 * @note   
 * @param  cmd_packer: 
 * @param  *buff: 
 * @param  size: 
 * @retval None
 */
void upacker_unpack(upacker_inst_t packer, uint8_t *buff, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        if (frame_decode(packer, buff[i]))
        {
            //解析成功,回调处理
            packer->cb(packer->data, packer->flen);
        }
    }
}

/**
 * @brief  封包数据并发送
 * @note   
 * @param  *packer: 
 * @param  *buff: 
 * @param  size: 
 * @retval None
 */
void upacker_pack(upacker_inst_t packer, uint8_t *buff, uint16_t size)
{
    RT_ASSERT(packer->sem_pack != RT_NULL);
    
    rt_sem_take(packer->sem_pack,RT_WAITING_FOREVER);
    frame_encode(packer, buff, size);
    rt_sem_release(packer->sem_pack);
}

static uint8_t frame_decode(upacker_inst_t packer, uint8_t d)
{
    //注意包断掉接续  
	//注意一包数据不完整新包又来临
	 
	if(d == STX_L)
	{
		packer->head_flag = 1;
	}
	else if(packer->head_flag && (d == STX_R))
	{
		packer->head_flag = 2;
		return 0;
        
	}else if((packer->head_flag ==2) && (d == STX_F))
	{
		packer->head_flag = 0;
		packer->state = 1;
		packer->cnt = 0;
        packer->flen = 8;
		return 0;
	}
	else
	{
		packer->head_flag = 0;
	}
	
    if (packer->state == 1)
    {
        packer->data[packer->cnt++] = d;
        
        if (packer->cnt == packer->flen)
        {
			packer->flen -= 2;
		
			packer->check = packer->data[packer->flen + 1];

			//校验
			packer->calc = crc_16(packer->data, packer->flen);
			packer->state = 0;
			
			//接收完，检查check
			if ((packer->calc) == (packer->check))
			{
				return 1;
			}
			else
			{
				log_e("crc check error %x for %x", packer->calc, packer->check);
				return 0;
			}
        }
    }
    else
    {
        packer->state = 0;
    }
    return 0;
}

static uint8_t frame_encode(upacker_inst_t packer, uint8_t *data, uint16_t size)
{
	uint8_t len = 0;
    uint8_t tmp[MAX_PACK_SIZE] = {0};
    uint8_t crc = 0;

    if (size > 16384)
    {
        return 0;
    }

    for (int i = 0; i < size; i++)
    {
        tmp[i] = data[i];
    }
    
    len += size;
	crc = crc_16(&tmp[3], ++size);

	tmp[len] = 0; 
    tmp[len + 1] = crc & 0xff;  
	
	len += 2;
	
    packer->send(tmp, len);

    return 1;
}
