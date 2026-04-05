#include "lcd.h"
#include "stdlib.h"
#include "sys.h"	 
#include "spi.h"
#include "delay.h"



//默认为竖屏
_lcd_dev lcddev;

//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 

//LCD写读寄存器函数
void LCD_WR_REG(u8 data)
{
    LCD_CS_CLR;//片选开始
    LCD_RS_CLR;//写寄存器
    SPI_WriteByte(SPI1,data);//发送要写入的寄存器地址
    LCD_CS_SET;//片选结束
}

//LCD写读数据函数
void LCD_WR_DATA(u8 data)
{
    LCD_CS_CLR;//片选开始
    LCD_RS_SET;//数据输出
    SPI_WriteByte(SPI1,data);//发送数据
    LCD_CS_SET;//片选结束
}

//LCD写入寄存器值函数
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)//
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}
//LCD写入GRAM准备函数
void LCD_WriteRAM_Prepare(void)//写入GRAM
{
	LCD_WR_REG(lcddev.wramcmd);
}	

//LCD写入数据函数
void Lcd_WriteData_16Bit(u16 Data)
{	
   LCD_CS_CLR;
   LCD_RS_SET;  
   SPI_WriteByte(SPI1,Data>>8);
	 SPI_WriteByte(SPI1,Data);
   LCD_CS_SET;
}

//LCD读出数据函数
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);//设置光标位置 
	Lcd_WriteData_16Bit(POINT_COLOR); 
}

//LCD清屏函数
void LCD_Clear(u16 Color)
{
  unsigned int i,m;  
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);   
	LCD_CS_CLR;
	LCD_RS_SET;
	for(i=0;i<lcddev.height;i++)
	{
    for(m=0;m<lcddev.width;m++)
    {	
			Lcd_WriteData_16Bit(Color);
		}
	}
	 LCD_CS_SET;
} 


//LCD GPIO初始化函数
void LCD_GPIOInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB ,ENABLE);	//使能GPIOB时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6| GPIO_Pin_7| GPIO_Pin_8|GPIO_Pin_9; //GPIOB6,7,8,9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化
}

//LCD复位函数
void LCD_RESET(void)
{
	LCD_RST_CLR;
	delay_ms(100);	
	LCD_RST_SET;
	delay_ms(50);
}

//LCD初始化函数
void LCD_Init(void)
{  
	SPI1_Init(); //硬件SPI1初始化
	LCD_GPIOInit();//LCD GPIO初始化										 
 	LCD_RESET(); //LCD 复位

    //ST7735S初始化
	LCD_WR_REG(0x11);//Sleep exit 
	delay_ms(120);	
	//ST7735R Frame Rate
	LCD_WR_REG(0xB1); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 
	LCD_WR_REG(0xB2); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 
	LCD_WR_REG(0xB3); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 
	LCD_WR_DATA(0x01); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x2D); 	
	LCD_WR_REG(0xB4); //Column inversion    
	LCD_WR_DATA(0x07); 	

	//ST7735R Power Sequence
	LCD_WR_REG(0xC0); 
	LCD_WR_DATA(0xA2); 
	LCD_WR_DATA(0x02); 
	LCD_WR_DATA(0x84); 
	LCD_WR_REG(0xC1); 
	LCD_WR_DATA(0xC5); 
	LCD_WR_REG(0xC2); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0xC3); 
	LCD_WR_DATA(0x8A); 
	LCD_WR_DATA(0x2A); 
	LCD_WR_REG(0xC4); 
	LCD_WR_DATA(0x8A); 
	LCD_WR_DATA(0xEE); 
	LCD_WR_REG(0xC5); //VCOM 
	LCD_WR_DATA(0x0E); 	
	LCD_WR_REG(0x36); //MX, MY, RGB mode 
	LCD_WR_DATA(0xC0); 

	//ST7735R Gamma Sequence
	LCD_WR_REG(0xe0); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x1a); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x18); 
	LCD_WR_DATA(0x2f); 
	LCD_WR_DATA(0x28); 
	LCD_WR_DATA(0x20); 
	LCD_WR_DATA(0x22); 
	LCD_WR_DATA(0x1f); 
	LCD_WR_DATA(0x1b); 
	LCD_WR_DATA(0x23); 
	LCD_WR_DATA(0x37); 
	LCD_WR_DATA(0x00); 	
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x02); 
	LCD_WR_DATA(0x10); 
	LCD_WR_REG(0xe1); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x1b); 
	LCD_WR_DATA(0x0f); 
	LCD_WR_DATA(0x17); 
	LCD_WR_DATA(0x33); 
	LCD_WR_DATA(0x2c); 
	LCD_WR_DATA(0x29); 
	LCD_WR_DATA(0x2e); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x30); 
	LCD_WR_DATA(0x39); 
	LCD_WR_DATA(0x3f); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x03); 
	LCD_WR_DATA(0x10);  	
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x7f);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x9f);
	LCD_WR_REG(0xF0); //Enable test command  
	LCD_WR_DATA(0x01); 
	LCD_WR_REG(0xF6); //Disable ram power save mode 
	LCD_WR_DATA(0x00); 	
	LCD_WR_REG(0x3A); //65k mode 
	LCD_WR_DATA(0x05); 
	LCD_WR_REG(0x29);//Display on	 	
    LCD_direction(USE_HORIZONTAL);//设置LCD显示方向
	LCD_LED=1;//点亮背光	 
	LCD_Clear(WHITE);//清全屏白色
}

//LCD显示方向设置函数
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(xStar);		
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(xEnd);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(yStar);		
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(yEnd);

	LCD_WriteRAM_Prepare();	//开始写入GRAM			
}

//LCD设置光标函数
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);	
} 

//LCD显示方向设置函数
void LCD_direction(u8 direction)
{ 
			lcddev.setxcmd=0x2A;
			lcddev.setycmd=0x2B;
			lcddev.wramcmd=0x2C;
	switch(direction){		  
		case 0:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;		
			LCD_WriteReg(0x36,(0<<3)|(1<<6)|(1<<7));//BGR==1,MY==0,MX==0,MV==0
		break;
		case 1:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			LCD_WriteReg(0x36,(0<<3)|(1<<7)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
		break;
		case 2:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;	
			LCD_WriteReg(0x36,(0<<3)|(0<<6)|(0<<7));//BGR==1,MY==0,MX==0,MV==0
		break;
		case 3:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
			LCD_WriteReg(0x36,(0<<3)|(0<<7)|(1<<6)|(1<<5));//BGR==1,MY==1,MX==0,MV==1
		break;	
		default:break;
	}		
}	 


