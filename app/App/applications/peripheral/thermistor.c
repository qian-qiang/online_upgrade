#include "thermistor.h" 

#define SUPPORT_VOLTAGE_A   5.0f     //供电电压,unit: V
#define SUPPORT_VOLTAGE_B   3.26f     //供电电压,unit: V

#define REFERENCE_VOLTAGE   3.28f     //参考电压,unit:V
#define DIVIDE              4095     //ADC位数

#define PULL_UP_RES_A       20       //上拉电阻,unit:k     
#define PULL_UP_RES_B       10       //上拉电阻,unit:k
#define TEMP_NUM            17       //温度总共的档位数

//第一行代表温度，十度一档
//第二行代表对应的热敏电阻阻值
const float temp_table[2][TEMP_NUM] = {
 {   -30,    -20,    -10,      0,     10,    15,     20,    25,   30,    35,    40,    50,    60,    70,    80,    90,   100},
 {122.0f, 72.04f, 44.09f, 27.86f, 18.13f, 14.77, 12.12f, 10.0f, 8.3f, 6.92f, 5.81f, 4.14f, 3.01f, 2.23f, 1.67f, 1.27f, 0.97f}
};

//根据二维数组表计算温度
int Temp_Cnt(int temp, rt_uint8_t cntmeans)
{ 
    float resist  = 0;  //电阻值
    float voltage = 0;  //电压值
    float k = 0;        //k值
    float b = 0;        //b值    
	
	if(cntmeans)
	{
		//计算电压值
		voltage = (float)temp / DIVIDE * REFERENCE_VOLTAGE;
		//电压限幅
		voltage = LIMIT(voltage,0,SUPPORT_VOLTAGE_A - 0.01f);   
		//计算电阻值
		resist  = (PULL_UP_RES_A * voltage) / (SUPPORT_VOLTAGE_A - voltage);
	}
	else
	{
		//计算电压值
		voltage = (float)temp / DIVIDE * REFERENCE_VOLTAGE;
		//电压限幅
		voltage = LIMIT(voltage,0,SUPPORT_VOLTAGE_B - 0.01f);   
		//计算电阻值
		resist  = (PULL_UP_RES_B * voltage) / (SUPPORT_VOLTAGE_B - voltage);
	}
  
    
    //数组之外的数据
    if(resist <= temp_table[1][TEMP_NUM - 1])     //电阻值小于最小值，返回100℃
        return (int)(1000);
    else if(resist >= temp_table[1][0])           //电阻值大于最大值，返回-30℃
        return (int)(-300);
    else
    {
        //遍历二维数组
        for(uint8_t i = 0; i < TEMP_NUM - 1; i++)
        {
            //判定是否在计算范围内
            if(IsInSide(resist,temp_table[1][i+1],temp_table[1][i]))
            {
                //计算k值和b值
                k = (temp_table[0][i] - temp_table[0][i+1])/(temp_table[1][i] - temp_table[1][i+1]);
                b = temp_table[0][i] - k * temp_table[1][i];
                
                break;
            }
        }
        //返回温度值
        return (int)((k * resist + b) * 10);
    } 
}
