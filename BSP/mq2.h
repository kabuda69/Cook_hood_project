#ifndef __MQ2_H
#define __MQ2_H

#include "sys.h"

// MQ2引脚定义
// PA4连接MQ2传感器的模拟输出，作为ADC输入,通道为4
#define MQ2_ADC_GPIO_PORT       GPIOA
#define MQ2_ADC_GPIO_PIN        GPIO_Pin_4
#define MQ2_ADC_GPIO_CLK        RCC_APB2Periph_GPIOA 
#define MQ2_ADC_CHANNEL         ADC_Channel_4      

//气体浓度阈值定义（用于防回流模式）
#define GAS_THRESHOLD_NORMAL    100.0f     //正常阈值 
#define GAS_THRESHOLD_HIGH      2000.0f    //切换后的高阈值

void MQ2_Init(void);                 //MQ2传感器初始化
u16 MQ2_GetAdcValue(void);           //获取MQ2传感器的ADC值
float MQ2_GetGasConcentration(void);   //获取气体浓度（0-500），单位为ppm
u8 MQ2_IsGasDetected(u16 threshold);  //检测气体浓度是否超过指定阈值，返回1表示检测到气体，0表示未检测到




#endif
