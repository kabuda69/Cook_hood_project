#include "motor.h"
#include "gpiox.h"
#include "delay.h"
#include "FreeRTOS.h" 
#include "task.h" 

volatile int overflow = 0;
volatile float speed = 0.0f;

//**************直流有刷电机驱动*******************
//u16 arr - 自动重装载值;    u16 psc - 预分频值;
//u16 ccr - 比较值(占空比);  u16 dtg - 死区时间;
void TIM1_dead_pwm_init(u16 arr,u16 psc,u16 ccr,u16 dtg)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  io_set(GPIOA,GPIO_Pin_8,GPIO_Mode_AF_PP);           // PA8 = TIM1_CH1
	io_set(GPIOB,GPIO_Pin_13,GPIO_Mode_AF_PP);          // PB13 = TIM1_CH1N
  io_set(GPIOA,GPIO_Pin_2,GPIO_Mode_Out_PP);          // PA2 = 
	
	// 初始化TIM1时基单元
  TIM_TimeBaseStructure.TIM_Period = arr;                       
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                    
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 
	
	//初始化TIM1 OC1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ccr;                          // 设置初始占空比
    TIM_OC1Init(TIM1,&TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); 
	
	//配置BDTR寄存器
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);	
	TIM_BDTRInitStructure.TIM_DeadTime = dtg;                               // 设置死区时间，单位为TIM1的时钟周期，范围0-255
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;             // 当MOE=0时，OC1N输出为高电平，OC1输出为低电平
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;            //当MOE=0时，OC1N输出为高电平，OC1输出为低电平
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                    //不使能刹车功能
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;        //刹车输入极性，低电平有效
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; //使能自动输出，当刹车事件发生时，MOE位会被自动清零，关闭PWM输出；当刹车事件结束时，MOE位会被自动置位，恢复PWM输出
	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);   
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  	// 使能MOE
	TIM_Cmd(TIM1, ENABLE); 
	
}

//电机停止
void motor_stop(void)
{
	TIM_CCxCmd(TIM1,TIM_Channel_1,TIM_CCx_Disable);
	TIM_CCxNCmd(TIM1,TIM_Channel_1,TIM_CCxN_Disable);
	io_reset_bit(GPIOA,GPIO_Pin_2);
}

//电机启动 
void motor_start(void)
{
	io_set_bit(GPIOA,GPIO_Pin_2);//PA2置高，启动电机
}

//电机方向控制
void motor_dir(direction para)
{
	taskENTER_CRITICAL();
	//关闭PWM输出
	TIM_CCxCmd(TIM1,TIM_Channel_1,TIM_CCx_Disable);
	TIM_CCxNCmd(TIM1,TIM_Channel_1,TIM_CCxN_Disable);
	//延时20us，确保PWM输出完全关闭，避免死区时间过短导致的电机损坏
	delay_us(20);
  // 根据方向参数设置PWM输出
	if (para == stright)//正转
	{
	TIM_CCxNCmd(TIM1,TIM_Channel_1,TIM_CCxN_Enable);
	}
	else if(para == invert)//反转
	{
		TIM_CCxCmd(TIM1,TIM_Channel_1,TIM_CCx_Enable);
	}
	taskEXIT_CRITICAL();
}

//电机初始化
void motor_init(void)
{
	motor_dir(stright);
	motor_stop();
	motor_start();                                                                                
}

// 设置电机转速，ccr为比较值，占空比，范围0-1000
void motor_speed(u16 ccr)
{
	//限制ccr的范围，避免过高的占空比导致电机损坏
	if(ccr <= 1000)
	{
		//设置TIM1_CH1的比较值，调整占空比
		TIM_SetCompare1(TIM1,ccr);
	}
}

// 设置电机转速和方向，para为转速参数，正数表示正转，负数表示反转，绝对值表示转速大小
void motor_pwm_set(float para)
{
	int val = (int)para;//将转速参数转换为整数，便于比较和设置占空比
	
	if (val >= 0)
	{
		motor_dir(stright);//设置正转
		motor_speed(val);//设置转速
	}
	else
	{
		motor_dir(invert);//设置反转
		motor_speed(-val);//设置转速
	}
}


//*************TIM2编码器功能初始化 ****************
void TIM2_encode_init(u16 arr, u16 psc)
{
	 TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   TIM_ICInitTypeDef  TIM_ICInitStructure;
   NVIC_InitTypeDef  NVIC_InitStructure;
	//使能TIM2时钟和GPIO 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	io_set(GPIOA, GPIO_Pin_0, GPIO_Mode_IPD);            // PA0 = TIM2_CH1
	io_set(GPIOA, GPIO_Pin_1, GPIO_Mode_IPD);            // PA1 = TIM2_CH1N

	// 初始化TIM2参数
	TIM_TimeBaseStructure.TIM_Period = arr; 				          
    TIM_TimeBaseStructure.TIM_Prescaler = psc;       		       
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	   //时钟不分频
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	// 配置TIM2输入捕获参数，使用CH1和CH2作为编码器输入
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;          // 输入捕获不分频
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //选择输入通道，对应寄存器CCIS[1:0]
	TIM_ICInit(TIM2,&TIM_ICInitStructure);	
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;          // 不分频，每一个边沿触发一次捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//选择输入通道，对应寄存器CCIS[1:0]
	TIM_ICInit(TIM2,&TIM_ICInitStructure);	
	
	//配置TIM2编码器接口，使用CH1和CH2作为编码器输入，计数模式为TI12模式，上升沿触发
	TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	
	//配置TIM2中断，优先级为6，子优先级为0
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  			    
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				
	NVIC_Init(&NVIC_InitStructure);  					
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );                //使能TIM2更新中断，计数器溢出时触发中断
	TIM_Cmd(TIM2, ENABLE);  
}

//TIM4定时器初始化，用于定时获取编码器计数值，计算速度
//arr - 自动重装载值，决定定时器的周期；psc - 预分频值，决定定时器的计数频率
void TIM4_init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	TIM_TimeBaseStructure.TIM_Period = arr; 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); 	
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; //抢占优先级5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  	    // 子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 

  TIM_Cmd(TIM4, ENABLE);   	
}

//获取编码器计数值，返回一个整数，表示当前的编码器计数值，考虑了计数器溢出的情况
int get_encoder_value(void)
{
	u32 buffer;
	buffer=TIM_GetCounter(TIM2)+(overflow*65536);
	return buffer;
}

//获取编码器转速，返回一个浮点数，表示当前的转速，单位为RPM
u8 TIM_GetDirection(TIM_TypeDef* TIMx)
{
	// 判断计数器的方向位，如果DIR位为1，表示计数器在递减，返回1；如果DIR位为0，表示计数器在递增，返回0
	return(TIMx->CR1 & TIM_CR1_DIR) ? 1 : 0;
}

//获取编码器转速，返回一个浮点数，表示当前的转速，单位为RPM
//ms = 50，表示每50ms计算一次转速
float get_speed(int encode_value,u16 ms)
{
	  u8 i = 0, j = 0;
    float temp = 0.0;
	  static float speed=0;									
    static uint8_t sp_count = 0, k = 0;
    static float speed_arr[10] = {0.0};                     // 存储最近10次计算的转速值，用于中位数滤波 
	  static int old_value=0,now_value=0;

	 if (sp_count == ms)                                     // 每ms计算一次转速 
    {
		now_value = encode_value;							                 // 获取当前编码器计数值
		
        // 计算转速，单位为RPM，公式为：转速 = (计数值变化量 * (1000/ms) * 60) / (编码器每转的计数值 * 4)
		    // 这里假设编码器每转的计数值为30，且使用了4倍频模式，所以除以(30*4)
        speed_arr[k++] = (float)((now_value - old_value) * ((1000 / ms) * 60.0) / 30 / (11*4)); 
		    old_value = now_value;								// 更新旧的编码器计数值
		
        // 当收集了10个转速值后，进行中位数滤波和指数移动平均滤波
        if (k == 10)
        {
            for (i = 10; i >= 1; i--)                       
            {
                for (j = 0; j < (i - 1); j++) 
                {
                    if (speed_arr[j] > speed_arr[j + 1])    
                    { 
                        temp = speed_arr[j];                
                        speed_arr[j] = speed_arr[j + 1];
                        speed_arr[j + 1] = temp;
                    }
                }
            }
            
            temp = 0.0;
            
            for (i = 2; i < 8; i++)                         // 取中间6个转速值，去掉最高和最低的2个值，计算平均值，减少异常值的影响
            {
                temp += speed_arr[i];                       // 累加中间6个转速值 
            }
            
            temp = (float)(temp / 6);                       // 计算中间6个转速值的平均值，得到一个较为稳定的转速值
            
            /* 指数移动平均滤波
             * 输出Y(n)= qX(n) + (1-q)Y(n-1)
             * 其中X(n)为当前输入，Y(n-1)为上一次输出，q为滤波系数
             * q值越小，滤波效果越强，但响应速度越慢
             * 这里选择q=0.48，表示当前转速值占48%，上一次转速值占52%，可以平滑转速变化，减少噪声的影响
						 * 通过调整q值，可以在滤波效果和响应速度之间进行权衡
             */
            speed = (float)( ((float)0.48 * temp) + (speed * (float)0.52) );
            k = 0;
        }
        sp_count = 0;
    }
    sp_count ++;
	  return speed;
}
