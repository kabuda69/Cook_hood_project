#include "beep.h"
#include "usart.h"	
#include "delay.h"

//初始化BEEP的io口
//结构体变量
//端口（GPIOA-G）
//管脚（0-16）
void Beep_Init(led_d *io,gpioled port,u16 pin)
{
    //开启时钟
	if(port==GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 
	else if(port==GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	else if(port==GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	else if(port==GPIOD) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	else if(port==GPIOE) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	else if(port==GPIOF) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
	else if(port==GPIOG) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);
    //初始化io口
    io->port=port;
    io->pin=pin;
   //配置io口
    Beep_config(io);
}


//配置io口
void Beep_config(led_d *io)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = io->pin;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	GPIO_Init(io->port, &GPIO_InitStructure);					
	Beep_off(io);
}

//关闭BEEP
void Beep_off(led_d *io)
{
	GPIO_ResetBits(io->port,io->pin);
}

//开启BEEP
void Beep_on(led_d *io)
{
	GPIO_SetBits(io->port,io->pin);
}
void Beep_on(led_d *io)
{
	GPIO_SetBits(io->port,io->pin);
}

//BEEP鸣叫
void Buzzer_Beep(u16 duration_ms,led_d *io)
{
    Beep_on(io);
    delay_ms(duration_ms);
    Beep_off(io);
}





