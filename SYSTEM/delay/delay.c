#include "delay.h"
#include "sys.h"

#if SYSTEM_SUPPORT_OS 

#include "FreeRTOS.h"
#include "task.h"

#endif

static u8  fac_us=0;  //us延时倍乘数
static u16 fac_ms=0;  //ms延时倍乘数,在ucos下,代表每个节拍的ms数

 extern void xPortSysTickHandler(void);//OS的systick中断服务函数
 
//systick中断服务函数，使用OS时用到
void SysTick_Handler(void)
{
  if( xTaskIncrementTick() != taskSCHEDULER_NOT_STARTED )//如果OS已经开始调度任务了
   {
         xPortSysTickHandler();    
   }
}
//初始化延时函数
void delay_init(void)
{
	uint32_t  reload;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);  
	fac_us=SystemCoreClock/1000000;				            
	reload=SystemCoreClock/1000000;				              
	reload*=1000000/configTICK_RATE_HZ;			          
												                            
	fac_ms=1000/configTICK_RATE_HZ;			            	  

	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;   	      
	SysTick->LOAD=reload; 					                 		
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;         	 
}	

//us级延时
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;					    	 
	ticks=nus*fac_us; 						//��Ҫ�Ľ�����
	told=SysTick->VAL;        		//�ս���ʱ����������ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	 //SysTick��24λ�ݼ���ʱ��
			else tcnt+=reload-tnow+told;	   
			told=tnow;
			if(tcnt>=ticks)break;			//��ʱ��������������˳�
		}  
	};										    
} 
//ms级延时
void delay_ms(u32 nms)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)
	{		
		if(nms>=fac_ms)						                            
		{ 
   			vTaskDelay(nms/fac_ms);	 		                     
		}
		nms%=fac_ms;						                     
	}
	delay_us((u32)(nms*1000));				
}
//延时nms
void delay_xms(u32 nms)
{
	u32 i;
	for(i=0;i<nms;i++) 
	delay_us(1000);
}


