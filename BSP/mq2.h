#ifndef __MQ2_H
#define __MQ2_H

#include "sys.h"

// MQ2引脚定义
// PA4连接MQ2传感器的模拟输出，作为ADC输入,通道为4
#define MQ2_ADC_GPIO_PORT       GPIOA
#define MQ2_ADC_GPIO_PIN        GPIO_Pin_4
#define MQ2_ADC_GPIO_CLK        RCC_APB2Periph_GPIOA 
#define MQ2_ADC_CHANNEL         ADC_Channel_4      


void MQ2_Init(void);
u16 MQ2_GetAdcValue(void);
float MQ2_GetGasValue(void);
float MQ2_GetGasConcentration(void);
u8 MQ2_IsGasDetected(u16 threshold);




#endif
