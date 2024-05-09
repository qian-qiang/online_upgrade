#include "thermistor.h" 

#define SUPPORT_VOLTAGE_A   5.0f     //�����ѹ,unit: V
#define SUPPORT_VOLTAGE_B   3.26f     //�����ѹ,unit: V

#define REFERENCE_VOLTAGE   3.28f     //�ο���ѹ,unit:V
#define DIVIDE              4095     //ADCλ��

#define PULL_UP_RES_A       20       //��������,unit:k     
#define PULL_UP_RES_B       10       //��������,unit:k
#define TEMP_NUM            17       //�¶��ܹ��ĵ�λ��

//��һ�д����¶ȣ�ʮ��һ��
//�ڶ��д����Ӧ������������ֵ
const float temp_table[2][TEMP_NUM] = {
 {   -30,    -20,    -10,      0,     10,    15,     20,    25,   30,    35,    40,    50,    60,    70,    80,    90,   100},
 {122.0f, 72.04f, 44.09f, 27.86f, 18.13f, 14.77, 12.12f, 10.0f, 8.3f, 6.92f, 5.81f, 4.14f, 3.01f, 2.23f, 1.67f, 1.27f, 0.97f}
};

//���ݶ�ά���������¶�
int Temp_Cnt(int temp, rt_uint8_t cntmeans)
{ 
    float resist  = 0;  //����ֵ
    float voltage = 0;  //��ѹֵ
    float k = 0;        //kֵ
    float b = 0;        //bֵ    
	
	if(cntmeans)
	{
		//�����ѹֵ
		voltage = (float)temp / DIVIDE * REFERENCE_VOLTAGE;
		//��ѹ�޷�
		voltage = LIMIT(voltage,0,SUPPORT_VOLTAGE_A - 0.01f);   
		//�������ֵ
		resist  = (PULL_UP_RES_A * voltage) / (SUPPORT_VOLTAGE_A - voltage);
	}
	else
	{
		//�����ѹֵ
		voltage = (float)temp / DIVIDE * REFERENCE_VOLTAGE;
		//��ѹ�޷�
		voltage = LIMIT(voltage,0,SUPPORT_VOLTAGE_B - 0.01f);   
		//�������ֵ
		resist  = (PULL_UP_RES_B * voltage) / (SUPPORT_VOLTAGE_B - voltage);
	}
  
    
    //����֮�������
    if(resist <= temp_table[1][TEMP_NUM - 1])     //����ֵС����Сֵ������100��
        return (int)(1000);
    else if(resist >= temp_table[1][0])           //����ֵ�������ֵ������-30��
        return (int)(-300);
    else
    {
        //������ά����
        for(uint8_t i = 0; i < TEMP_NUM - 1; i++)
        {
            //�ж��Ƿ��ڼ��㷶Χ��
            if(IsInSide(resist,temp_table[1][i+1],temp_table[1][i]))
            {
                //����kֵ��bֵ
                k = (temp_table[0][i] - temp_table[0][i+1])/(temp_table[1][i] - temp_table[1][i+1]);
                b = temp_table[0][i] - k * temp_table[1][i];
                
                break;
            }
        }
        //�����¶�ֵ
        return (int)((k * resist + b) * 10);
    } 
}
