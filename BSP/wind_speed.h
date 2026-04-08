#ifndef __WIND_SPEED_H
#define __WIND_SPEED_H
#include "stm32f10x.h"
// 最大CCR值定义
#define MAXCCR              1000   // 最大CCR值
// 温度范围定义
#define TEMP_BASE           20   // 基础温度，单位：℃
#define TEMP_MAX            35   // 最大温度，单位：℃
// 湿度范围定义
#define HUMIDITY_BASE       40    // 基础湿度，单位：%
#define HUMIDITY_MAX        75    // 最大湿度，单位：%%
// 气体浓度范围定义
#define GAS_BASE            80.0f // 基础气体浓度，单位：ppm
#define GAS_MAX             450.0f  // 最大气体浓度，单位：ppm
// 权重定义
#define WEIGHT_TEMP         0.2f  // 温度权重
#define WEIGHT_HUMIDITY     0.2f  // 湿度权重
#define WEIGHT_GAS          0.6f  // 大气体浓度权重
// PWM输出范围定义
#define PWM_MIN             20.0f   // 最小PWM值，单位：%
#define PWM_MAX             100.0f  // 最大PWM值，单位：
// Cooking Event判定条件
//温度 > 26 && (湿度 > 50 && 气体浓度 > 100)
#define COOKING_TEMP_THRESHOLD      26       // Cooking Event温度阈值
#define COOKING_HUMIDITY_THRESHOLD  50       // Cooking Event湿度阈值
#define COOKING_GAS_THRESHOLD       100.0f   // Cooking Event气体浓度阈值

// 手动模式档位对应转速 (RPM)
#define SPEED_LOW_RPM       190         // 低档转速
#define SPEED_HIGH_RPM      220         // 高档转速

typedef struct {
    
    float f_T;                  /* 温度影响系数 (归一化后) */
    float f_H;                  /* 湿度影响系数 (归一化后) */
    float f_G;                  /* 气体浓度影响系数 (归一化后) */
    
    float fusionValue;          /* 融合后的值 F */
    float pwmValue;             /* 计算得到的PWM值 (%) */
    
    u8 isCookingEvent;          /* 是否为Cooking Event */
} WindSpeed_t;

void WindSpeed_Init(void);
void WindSpeed_Update(u8 temp, u8 humidity, float gas);
float WindSpeed_GetPWM(void);
u16 WindSpeed_GetPWMCompare(u16 maxCompare);
u8 WindSpeed_IsCookingEvent(void);
WindSpeed_t* WindSpeed_GetData(void);
u16 WindSpeed_GetTargetRPM(u8 level);


#endif



