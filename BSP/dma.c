#include "dma.h"

DMA_InitTypeDef DMA_InitStructure;
u16 DMA1_MEM_LEN;

//DMA_CHx:DMA通道CHx
//cpar:外设地址
//cmar:存储器地址
//cndtr:数据传输量 
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx, u32 cpar, u32 cmar, u32 cndtr)
{
  NVIC_InitTypeDef NVIC_InitStructure; //中断配置结构体
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //使能DMA1时钟

  DMA_DeInit(DMA_CHx); //将DMA的通道寄存器重设为缺省值
  
  DMA1_MEM_LEN=cndtr; //保存数据传输量
  DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)cpar; //外设地址
  DMA_InitStructure.DMA_MemoryBaseAddr = (u32)cmar; //存储器地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //数据传输方向：外设作为数据源
  DMA_InitStructure.DMA_BufferSize = cndtr; //DMA通道的DMA缓存大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址递增禁止
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级 
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
  DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器

  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;//DMA1通道5中断
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; //抢    占优先级4
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //子优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能DMA1通道5中断
  NVIC_Init(&NVIC_InitStructure); 

  DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE); //使能DMA1通道5传输完成中断
  DMA_Cmd(DMA1_Channel5, ENABLE); //使能DMA通道
}

//重新开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef* DMA_CHx)
{ 
  DMA_Cmd(DMA_CHx, DISABLE);//先禁止DMA     
  DMA_SetCurrDataCounter(DMA_CHx, DMA1_MEM_LEN); //设置DMA传输数据量
  DMA_Cmd(DMA_CHx, ENABLE); //使能DMA通道
}





