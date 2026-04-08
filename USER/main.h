#ifndef __MAIN_H
#define __MAIN_H

#include "gpiox.h"
#include "delay.h"
#include "key.h"
#include "usart.h"
#include "lcd.h"
#include "gui.h"
#include "beep.h"
#include "dma.h"
#include "motor.h"
#include "wind_speed.h"
#include "mq2.h"
#include "pid.h"
#include "sys.h"


// 速度环PID控制器相关变量
PID_TypeDef g_speedPID;             //速度环PID控制器-结构体变量
led_d bep;							//beep结构体变量-蜂鸣器
led_d dht;							//DHT11结构体变量-湿度传感器LED












// 主函数中使用的函数声明
void DMA_Test_Init(void);
void LED_Init(void);

#endif

