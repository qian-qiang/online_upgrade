/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     qq           first version
 */
#define LOG_TAG   "pd"  
#define LOG_LVL   LOG_LVL_DBG 

#include "ulog.h"
#include "pd.h" 

//#define Isinside( x,min,max ) ( (x) < (min)  ? (0) : ( (x) > (max) ? (0) : (1) ) )

TydefStruct uvStruct = {0,0,0,0};
TydefStruct irStruct = {0,0,0,0};

rt_uint16_t Update_IR_PD_Bias(rt_uint16_t IR_PD_AdcVal);      //更新红外PD-ADC偏置值
rt_uint16_t IR_PowerRead_wayA(rt_uint16_t IR_PD_AdcVal);      //红外功率算法A
rt_uint16_t IR_PowerRead_wayB(rt_uint16_t IR_PD_AdcV, rt_uint16_t IR_PD_BiasV, rt_uint16_t IR_factor); //红外功率算法B


/********************************************************************************************
函数名：UVParaUpdate
功能  ：更新紫外PD参数
参数  ：
返回值：无
*********************************************************************************************/

/********************************************************************************************
函数名：UvLaserPowerRead
功能  ：读取紫外功率
参数  ：Adcval - 紫外PD ADC值 
		 Adcmax - 紫外功率90%时的ADC值
         Adcmin - 紫外功率10%时的ADC值
         Pmax -  紫外功率90%的值
      	 Pmin -  紫外功率10%的值
局限  ：仅能对应固定LD电流下的功率，LD电流变化时需重新校准。误差200mW
返回值：UvPower -紫外功率
*********************************************************************************************/
rt_uint16_t UvLaserPowerRead(rt_uint16_t Adcval, rt_uint16_t Adcmax, rt_uint16_t Adcmin, rt_uint16_t Pmax, rt_uint16_t Pmin, rt_uint32_t divider)   
{
	rt_uint16_t UvPower;
	int temp; //计算紫外功率
//	float k,b;
	
	static rt_uint16_t adcValBuf = 100;
	
	uvStruct._adcMax   = Adcmax;
	uvStruct._adcMin   = Adcmin;
	uvStruct._powerMax = Pmax;
	uvStruct._powerMin = Pmin;
	uvStruct._adcVal   = Adcval;
	//这段if else 好像没啥意义  因为下面 if(uvStruct._adcVal <= 200（或<130）uvPower = 0)
	if((uvStruct._adcVal > adcValBuf - 3 ) && (uvStruct._adcVal < adcValBuf + 3))//过滤抖动
	{
		uvStruct._adcVal = adcValBuf;
	}
	else
	{
		adcValBuf = uvStruct._adcVal;
	}
	
	if(divider == 1)  //AOM2不分频，PD紫外功率监测
	{
		if(uvStruct._adcVal <= 200)
		{
			temp  = 0;//功率0
		}
		else if(uvStruct._adcVal <= uvStruct._adcMin + 200)
		{
			temp = (float)(uvStruct._adcVal - 200) / (uvStruct._adcMin - 200) * uvStruct._powerMin;
		}
		else 
		{	
			temp = (float)(uvStruct._adcVal - 400) / (uvStruct._adcMax - 400) * uvStruct._powerMax + 200;
		}
		UvPower = temp;
	}
	else                                   //AOM2分频，PD紫外功率监测
	{
		if(uvStruct._adcVal < 130)
		{
			UvPower = 0;
		}
		else
		{
//			k = (float)(UV_Struct.uv_power_max - UV_Struct.uv_power_max / 40 )  /  ((UV_Struct.uv_adc_max-120) - ( (UV_Struct.uv_adc_max-120) / 40) ) ;
//			b =  UV_Struct.uv_power_max - k * (UV_Struct.uv_adc_max-120);   //120偏置
//			
//			temp = k * ( Adcval - 240) + b;
//			
//			if(temp < 0)
//			{
//				temp = 0;
//			}
//			
   		    temp = (float)(uvStruct._adcVal - 120) / (uvStruct._adcMax >> 1) * (uvStruct._powerMax >> 1) ;
			UvPower = temp;
		}
	}
	
	return UvPower;
}
/********************************************************************************************
函数名：UvLaserPowerRead
功能  ：读取紫外功率
参数  ：Adcval - 紫外PD ADC值 
		 Adcmax - 紫外功率90%时的ADC值
         Adcmin - 紫外功率10%时的ADC值
         Pmax -  紫外功率90%的值
      	 Pmin -  紫外功率10%的值
局限  ：仅能对应固定LD电流下的功率，LD电流变化时需重新校准。误差200mW
返回值：UvPower -紫外功率
*********************************************************************************************/
rt_uint16_t IRLaserPowerRead(rt_uint16_t Adcval, rt_uint16_t Adcmax, rt_uint16_t Adcmin, rt_uint16_t Pmax, rt_uint16_t Pmin)
{
	rt_uint16_t IRPower; 
	static rt_uint16_t irAdcValBuf = 100;

	irStruct._adcMax   = Adcmax;
	irStruct._adcMin   = Adcmin;
	irStruct._powerMax = Pmax;
	irStruct._powerMin = Pmin;
	irStruct._adcVal   = Adcval;
		
	
	if((irStruct._adcVal > irAdcValBuf - 3 ) && (irStruct._adcVal < irAdcValBuf + 3))
	{
		irStruct._adcVal = irAdcValBuf;
	}
	else
	{
		irAdcValBuf = irStruct._adcVal;
	}
	
	if(irStruct._adcVal <= 200)
	{
		IRPower  = 0;
	}
	else if(irStruct._adcVal <= irStruct._adcMin + 200)
	{
		IRPower = (float)(irStruct._adcVal - 120) / (irStruct._adcMin - 120) * irStruct._powerMin;
	}
	else 
	{	
		IRPower = (float)(irStruct._adcVal - 120) / (irStruct._adcMax - 120) * irStruct._powerMax;
	}

	return  IRPower;
}

