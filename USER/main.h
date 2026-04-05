#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f10x.h"
#include "motor.h"
#include "dma.h"
#include "gpiox.h"
#include "delay.h"

// 主函数中使用的函数声明
void DMA_Test_Init(void);
void LED_Init(void);

#endif

