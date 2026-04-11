#include "mq2.h"
#include "delay.h"
#include <math.h> 

#include "mq2.h"
#include "delay.h"
#include <math.h> 


void MQ2_Init(void)
{
 GPIO_InitTypeDef GPIO_InitStructure;
 ADC_InitTypeDef ADC_InitStructure;

 // 使能GPIOA和ADC1的时钟
 RCC_APB2PeriphClockCmd(MQ2_ADC_GPIO_CLK | RCC_APB2Periph_ADC1, ENABLE);
// 配置ADC时钟，ADC时钟频率必须小于或等于14MHz
 RCC_ADCCLKConfig(RCC_PCLK2_Div6);
// 配置MQ2传感器连接的PA4引脚为模拟输入模式
 GPIO_InitStructure.GPIO_Pin = MQ2_ADC_GPIO_PIN;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;   // 模拟输入模式 */
 GPIO_Init(MQ2_ADC_GPIO_PORT, &GPIO_InitStructure);

 ADC_DeInit(ADC1);

 // 配置ADC1的参数
 ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // 独立模式
 ADC_InitStructure.ADC_ScanConvMode = DISABLE;      // 单通道模式   
 ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // 单次转换模式
 ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
 ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // 右对齐   
 ADC_InitStructure.ADC_NbrOfChannel = 1; // 转换通道数量为1
 ADC_Init(ADC1, &ADC_InitStructure);

 ADC_Cmd(ADC1, ENABLE);

 // 复位校准寄存器，等待复位完成，然后开始校准，等待校准完成
 ADC_ResetCalibration(ADC1); // 复位校准寄存器
 while(ADC_GetResetCalibrationStatus(ADC1)); // 等待复位校准完成    
 ADC_StartCalibration(ADC1); // 开始校准
 while(ADC_GetCalibrationStatus(ADC1)); // 等待校准完成

}


// 获取MQ2传感器的ADC值，返回一个16位无符号整数，范围0-4095
u16 MQ2_GetAdcValue(void)
{
 u16 adc_Value = 0;

 // 配置ADC1的参数4，设置要转换的通道为MQ2_ADC_CHANNEL，采样时间为239.5周期，采样时间越长，转换结果越稳定，但转换速度越慢
 ADC_RegularChannelConfig(ADC1, MQ2_ADC_CHANNEL, 1, ADC_SampleTime_239Cycles5);
 // 启动ADC1的转换
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);
 // 等待转换完成
 while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
 // 获取转换结果
 adc_Value = ADC_GetConversionValue(ADC1);

 return adc_Value;

}
// 获取MQ2传感器的气体浓度，返回一个浮点数，单位为ppm
float MQ2_GetGasConcentration(void)
{
    u8 i;
    u32 sum = 0;
    u16 adc_avg;
	float tmep,RS;
    float gas_concentration;
   // 读取MQ2传感器的ADC值10次，取平均值，减少噪声干扰
    for(i = 0; i < 10; i++)
    {
        sum += MQ2_GetAdcValue();
        delay_ms(100); // 每次读取之间延迟100ms，确保数据稳定
    }
    adc_avg = sum / 10;
    // 计算气体浓度
    // 获取传感器的输出电压，并计算RS值，
    //假设传感器的输出电压与ADC值成正比，且在空气中RS为0.5
    tmep = (float)adc_avg / 4095 * 5; // 将ADC值转换为电压，假设ADC参考电压为5V
    RS = (5 - tmep)/tmep * 0.5; // 示例计算，假设传感器的输出电压与气体浓度成反比，且在空气中RS为0.5
    gas_concentration = pow(11.5428*2/RS,0.6549f)*100;// 计算气体浓度 
    return gas_concentration;
}

u8 MQ2_IsGasDetected(u16 threshold)
{
    u16 gas_value = MQ2_GetAdcValue();
    
    if (gas_value > threshold)
    {
        return 1; // 检测到气体
    }
    else
    {
        return 0; // 未检测到气体
    }
}



