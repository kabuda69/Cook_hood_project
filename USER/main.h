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
#include "iap.h"


// 速度环PID控制器相关变量
PID_TypeDef g_speedPID;             //速度环PID控制器-结构体变量
led_d bep;							//beep结构体变量-蜂鸣器
led_d dht;							//DHT11结构体变量-湿度传感器LED


volatile int overflow=0;            //定时器溢出次数
volatile float speed;	            //电机实际转速，单位为RPM

#if ifopen
	extern u8 receive_buff[buff_size]; 
#endif





// 主函数中使用的函数声明
void System_Init(void);
void StartTask_Create(void);
static void Hardware_Init(void);

#endif

