#include "wind_speed.h"

/*
     算法说明：
 * 1. 传感器归一化：
 *    f_T = (T - T_base) / (T_max - T_base)
 *    f_H = (H - H_base) / (H_max - H_base)
 *     f_G = (G - G_base) / (G_max - G_base)
 * 
 * 2. 权重融合：
 *    F = w_t * f_T + w_h * f_H + w_g * f_G
 * 
 * 3. 映射到PWM：
 *    PWM = PWM_min + (PWM_max - PWM_min) * F  
*/

// 全局风速变量数据
static WindSpeed_t g_windSpeedData = {0};

//限制浮点数在指定范围内
static float Constrain(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// 初始化风速数据
void WindSpeed_Init(void)
{
    g_windSpeedData.f_T = 0.0f;
    g_windSpeedData.f_H = 0.0f;
    g_windSpeedData.f_G = 0.0f;
    g_windSpeedData.fusionValue = 0.0f;
    g_windSpeedData.pwmValue = 0.0f;
    g_windSpeedData.isCookingEvent = 0;
}

//更新传感器数据并计算风速
void WindSpeed_Update(u8 temp, u8 humidity, float gas)
{

    // 1. 传感器归一化,将这些系数限制在0.0至1.0之间，防止异常值影响系统稳定性。
    // 温度影响系数: f_T = (T - T_base) / (T_max - T_base)
    g_windSpeedData.f_T = (temp - TEMP_BASE) / (TEMP_MAX - TEMP_BASE);
    g_windSpeedData.f_T = Constrain(g_windSpeedData.f_T, 0.0f, 1.0f);
    
    // 湿度影响系数: f_H = (H - H_base) / (H_max - H_base)
    g_windSpeedData.f_H = (humidity - HUMIDITY_BASE) / (HUMIDITY_MAX - HUMIDITY_BASE);
    g_windSpeedData.f_H = Constrain(g_windSpeedData.f_H, 0.0f, 1.0f);
    
    // 气体浓度影响系数: f_G = (G - G_base) / (G_max - G_base)
    g_windSpeedData.f_G = (gas - GAS_BASE) / (GAS_MAX - GAS_BASE);
    g_windSpeedData.f_G = Constrain(g_windSpeedData.f_G, 0.0f, 1.0f);
    
    // 2. 权重融合: F = w_t * f_T + w_h * f_H + w_g * f_G
    g_windSpeedData.fusionValue = WEIGHT_TEMP * g_windSpeedData.f_T +
                                  WEIGHT_HUMIDITY * g_windSpeedData.f_H +
                                  WEIGHT_GAS * g_windSpeedData.f_G;
    
    // 3. 映射到PWM: PWM = PWM_min + (PWM_max - PWM_min) * F
    g_windSpeedData.pwmValue = PWM_MIN + (PWM_MAX - PWM_MIN) * g_windSpeedData.fusionValue;
    // 限制PWM值在PWM_MIN和PWM_MAX之间
    g_windSpeedData.pwmValue = Constrain(g_windSpeedData.pwmValue, PWM_MIN, PWM_MAX);
    
    // 4. 判断是否为Cooking Event: 温度 > 26 && (湿度 > 50 && 气体浓度 > 100)
    if ((temp > COOKING_TEMP_THRESHOLD) &&
        ((humidity > COOKING_HUMIDITY_THRESHOLD) && (gas > COOKING_GAS_THRESHOLD)))
    {
        g_windSpeedData.isCookingEvent = 1;
    }
    else
    {
        g_windSpeedData.isCookingEvent = 0;
    }
}


// 获取计算得到的PWM占空比
float WindSpeed_GetPWM(void)
{
    return g_windSpeedData.pwmValue;
}


// 获取PWM占空比对应的CCR值
// maxCompare: 定时器最大比较值（ARR值），单位：计数器周期
u16 WindSpeed_GetPWMCompare(u16 maxCompare)
{
    return (u16)(g_windSpeedData.pwmValue * maxCompare / 100.0f);
}


// 获取是否是烹饪事件
u8 WindSpeed_IsCookingEvent(void)
{
    return g_windSpeedData.isCookingEvent;
}

// 获取风速数据结构指针
WindSpeed_t* WindSpeed_GetData(void)
{
    return &g_windSpeedData;
}

// 获取风速等级对应的目标转速 (RPM)
// level: 风速等级，0为低档，1为高档    
u16 WindSpeed_GetTargetRPM(u8 level)
{
    switch (level)
    {
        case 0:
            return SPEED_LOW_RPM;
        case 1:
            return SPEED_HIGH_RPM;
        default:
            return SPEED_LOW_RPM;
    }
}


