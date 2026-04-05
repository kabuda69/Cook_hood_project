#include "spi.h"

//=======================================液晶屏数据线接线==========================================//
//本模块默认数据总线类型为SPI总线
//     LCD模块                CH32单片机    
//       SDA         接          PA7         //液晶屏SPI总线数据写信号
//=======================================液晶屏控制线接线==========================================//
//     LCD模块 					      CH32单片机 
//       LED         接          PB6         //液晶屏背光控制信号，如果不需要控制，接5V或3.3V
//       SCK         接          PA5         //液晶屏SPI总线时钟信号
//       A0          接          PB7         //液晶屏数据/命令控制信号
//       RESET       接          PB8         //液晶屏复位控制信号
//       CS          接          PB9         //液晶屏片选控制信号
//=========================================触摸屏触接线=========================================//


//SPI写读一个字节
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
{
    while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);//等待发送区空
    SPIx->DR=Byte;//写入数据
    while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//等待接收完成
    return SPIx->DR;//返回收到的数据
}

//设置SPI速度
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
{
    // 清空波特率(3,4,5位)
    SPIx->CR1&=0XFFC7;
	if(SpeedSet==1)//高速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2
	}
	else//低速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_32; //Fsck=Fpclk/32
	}
    // 打开SPI(第6位=1)
	SPIx->CR1|=1<<6; 
}

void SPI1_Init(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    //PA5、PA7配置为复用推挽输出,用来连接LCD的SCK和SDA
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;//PA5->SCK,PA7->SDA
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//高速
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    //PA6配置为输出，屏幕向MCU发送数据，但LCD只需要SDA数据线，所以PA6不连接LCD
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;    //PA6->MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //输入上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  //高速
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
    //配置SPI1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 ,ENABLE);
	   
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;//主机模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;//8位数据
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//空闲时SCK为低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//第一个时钟边沿采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;//软件控制NSS信号
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//MSB先传输
	SPI_InitStructure.SPI_CRCPolynomial = 7;//CRC校验
	SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE); 
}

