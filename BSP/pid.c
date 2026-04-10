#include "pid.h"


// PID初始化
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float  out_max, float out_min)
 { 
    pid->Kp = kp;// 比例系数
    pid->Ki = ki;// 积分系数
    pid->Kd = kd;// 微分系数
    
    pid->target = 0.0f;// 目标值（设定值）
    pid->actual = 0.0f;// 实际值（反馈值）

    pid->error = 0.0f;// 当前误差
    pid->last_error = 0.0f;// 上次误差
    pid->integral = 0.0f;// 误差积分累加值
    
    pid->output = 0.0f;// PID输出值
    pid->output_max = out_max;// 输出上限
    pid->output_min = out_min;// 输出下限
    // 积分上限设置为输出上限的一半
    // 防止积分饱和导致输出异常
    pid->integral_max = out_max/2.0f;// 积分上限（防止积分饱和）
}

// 设置目标值
void PID_SetTarget(PID_TypeDef *pid, float target)
{
    pid->target = target;
}

// 计算PID输出
float PID_Calculate(PID_TypeDef *pid, float actual)
{
    float p_out, i_out, d_out;
    
    // 更新实际值 （反馈值）
    pid->actual = actual;
    
    // 计算当前误差 （目标值 - 实际值）
    pid->error = pid->target - pid->actual;
    
    // 积分累加
    // 消除静态误差，让系统最终精准到达目标，不飘离目标
    pid->integral += pid->error;
    
    // 积分限幅，防止积分饱和 
    if (pid->integral > pid->integral_max)
    {
        pid->integral = pid->integral_max;
    }
    else if (pid->integral < -pid->integral_max)
    {
        pid->integral = -pid->integral_max;
    }
    
    // 计算PID三个分量
    p_out = pid->Kp * pid->error;                           // 比例项
    i_out = pid->Ki * pid->integral;                        // 积分项
    d_out = pid->Kd * (pid->error - pid->last_error);       // 微分项
    
    // 计算PID输出
    pid->output = p_out + i_out + d_out;
    
    // 输出限幅
    if (pid->output > pid->output_max)
    {
        pid->output = pid->output_max;
    }
    else if (pid->output < pid->output_min)
    {
        pid->output = pid->output_min;
    }
    
    // 保存当前误差供下次使用
    pid->last_error = pid->error;
    
    return pid->output;
}

//复位PID控制器
void PID_Reset(PID_TypeDef *pid)
{
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}























