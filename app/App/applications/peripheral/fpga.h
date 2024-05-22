#ifndef __SPI_H
#define __SPI_H

#include <rtthread.h>
#include <board.h>
#include <rtdevice.h>
#include "protocol_id.h"

#define FPGA_CS                  SPI2_CS_B        //SPI2_cs
	
/*写数据到FPGA*/
#define FQ_FRE_W                 (0x1001)
#define Burst_Num_W              (0x1002)
#define Mode_W                   (0x1003)
#define Mode_SW_W                (0x1004)
#define SQ_RF_SCALE_W            (0x1005)
#define DELAY_W                  (0x1006)
#define FQ_RF_SCALE_W            (0x1007)
#define EXT_MODE_W               (0x1008)
#define MODULO_DIVIDER_W         (0x1009)
#define BURST_HOLDOFF_W          (0x100A)
#define BURST_LENGTH_W           (0x100B)
#define EXT_MODE_ACTLEVEL_W      (0x100C)
#define SQ_ADD_TIME_W            (0x100D)
/*newadd calculate result return addr*/
#define AOM1_CYCLE_TOTAL_CNT_W   (0x1010) //newadd
#define AOM1_DUTY_TOTAL_CNT_W    (0x1011)
#define AOM1_DELAY_DIVIDE5_W     (0x1012)
#define AOM2_DELAY_DIVIDE5_W     (0x1013)


/*从FPAG读数据*/
#define FQ_FRE_R                 (0x0001)
#define Burst_Num_R              (0x0002)
#define Mode_R                   (0x0003)
#define Mode_SW_R                (0x0004)
#define SQ_RF_SCALE_R            (0x0005)
#define DELAY_R                  (0x0006)
#define FQ_RF_SCALE_R            (0x0007)
#define EXT_MODE_R               (0x0008)
#define MODULO_DIVIDER_R         (0x0009)
#define BURST_HOLDOFF_R          (0x000A)
#define BURST_LENGTH_R           (0x000B)
#define EXT_MODE_ACTLEVEL_R      (0x000C)
#define SQ_ADD_TIME_R            (0X000D)
#define FPAG_VERS_R              (0X000E)
#define SEED_FRE_R               (0x000F)

#define AOM1_DELAY_DIVIDE_5_R    (0x0012)
#define AOM2_DELAY_DIVIDE_5_R    (0x0013)
	
void fpga_spi2_init(void);			 //初始化SPI2口
void SPI2_SetSpeed(rt_uint8_t SpeedSet); //设置SPI2速度   

rt_uint16_t FPGA_Read(rt_uint16_t ReadAddr); 
void FPGA_Write(rt_uint16_t WriteAddr, rt_uint16_t Data);
void ACTIVE_WR_FPGA(void);
int spi_fpga_init(void);
#endif

