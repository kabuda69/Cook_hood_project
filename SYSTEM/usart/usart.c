#include "sys.h"
#include "usart.h"

#if SYSTEM_SUPPORT_OS

#include "FreeRTOS.h"

#endif

//串口初始化模块
//支持printf函数，重新定义了fputc函数
//注意：要保证在使用printf函数之前先调用uart_init函数进行串口初始化
#if 1
#pragma import(__use_no_semihosting)//
//标准库需要的支持函数
struct __FILE
{
  int handle;
};

FILE __stdout;
//定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
  x = x;
}
//重新定义fputc函数,printf函数调用fputc函数来实现数据输出
int fputc(int ch,FILE *f)
{
  while((USART1->SR&0X40)==0);
	  USART1->DR = (u8) ch;
	return ch;
}
#endif

#if EN_USART1_RX 

u8 USART_RX_BUF[USART_REC_LEN];
u16 USART_RX_STA=0; 
//串口1初始化
void uart_init(u32 bound)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;
	//使能USART1和GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
	//配置 TX 引脚，PA9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//配置 RX 引脚，PA10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//Usart1 NVIC 配置
	/*NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3; //抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  //响应优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		    	//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	  */                      
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;                                    //波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //8λ数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                         //1λ停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;                            //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	          

	USART_Init(USART1, &USART_InitStructure);
	//用DMA1通道5传输数据到USART1
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);	
	USART_Cmd(USART1, ENABLE);                   	  
}


//串口1中断服务程序
/*void USART1_IRQHandler(void)                	
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 		
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
				}
			else //��û�յ�0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
	 } 
} */

#endif

