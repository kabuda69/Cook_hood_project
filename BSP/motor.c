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
  io_set(GPIOA,GPIO_Pin_8,GPIO_Mode_AF_PP);           // PA8 = TIM1_CH1（主通道）
	io_set(GPIOB,GPIO_Pin_13,GPIO_Mode_AF_PP);          // PB13 = TIM1_CH1N（互补通道）
  io_set(GPIOA,GPIO_Pin_2,GPIO_Mode_Out_PP);          // PA2 = 使能控制信号（推挽输出）
	
	// 初始化TIM1时基单元
  TIM_TimeBaseStructure.TIM_Period = arr;                       // 自动重载值
	TIM_TimeBaseStructure.TIM_Prescaler = psc;                    // 预分频
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV4;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); 
	
	//通道输出配置
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = ccr;                          // 设置初始占空比
  TIM_OC1Init(TIM1,&TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable); 
	
	//配置BDTR寄存器，设置死区时间
	TIM_BDTRStructInit(&TIM_BDTRInitStructure);	
	TIM_BDTRInitStructure.TIM_DeadTime = dtg;                               // 设置死区时间
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;             // OSSR位设置为1
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;            // OSSI位设置为0
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;                    //刹车关闭
	TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_Low;        //低电平刹车
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable; //使能AOE
	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);   
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);  	// 使能MOE位
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
	io_set_bit(GPIOA,GPIO_Pin_2);
}

//电机方向控制
void motor_dir(direction para)
{
	taskENTER_CRITICAL();
	//禁用所有通道输出
	TIM_CCxCmd(TIM1,TIM_Channel_1,TIM_CCx_Disable);
	TIM_CCxNCmd(TIM1,TIM_Channel_1,TIM_CCxN_Disable);
	//死区延时
	delay_us(20);
  // 方向切换
	if (para == stright)//正传
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

// 电机速度调节 
void motor_speed(u16 ccr)
{
	//限制速度，防止烧管
	if(ccr <= 1000)
	{
		//设置占空比
		TIM_SetCompare1(TIM1,ccr);
	}
}

//电机控制
void motor_pwm_set(float para)
{
	int val = (int)para;
	
	if (val >= 0)
	{
		motor_dir(stright);
		motor_speed(val);
	}
	else
	{
		motor_dir(invert);
		motor_speed(-val);
	}
}


//*************TIM2编码器功能初始化 ****************
void TIM2_encode_init(u16 arr, u16 psc)
{
	 TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
   TIM_ICInitTypeDef  TIM_ICInitStructure;
   NVIC_InitTypeDef  NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	io_set(GPIOA, GPIO_Pin_0, GPIO_Mode_IPD);            // PA0 = TIM2_CH1（主通道）
	io_set(GPIOA, GPIO_Pin_1, GPIO_Mode_IPD);            // PA1 = TIM2_CH1N（互补通道）
	// 初始化TIM2时基单元
	TIM_TimeBaseStructure.TIM_Period = arr; 				          
  TIM_TimeBaseStructure.TIM_Prescaler = psc;       		       
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	   //时钟不分割
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	//配置编码器，此处选择4倍频，TI1(A相)和TI2(B相)进行计数 
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;          // 不分频，每一个边沿触发一次捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//对应寄存器CCIS[1:0]
	TIM_ICInit(TIM2,&TIM_ICInitStructure);	
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;          // 不分频，每一个边沿触发一次捕获
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;//对应寄存器CCIS[1:0]
	TIM_ICInit(TIM2,&TIM_ICInitStructure);	
	
	// 配置编码器接口，选择TI1和TI2同时计数 
	TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
	
	//配置TIM2中断，因为计数器为65536，可能会溢出所以配置中断 
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  		// 抢占优先级6
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  			    // 子优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 				
	NVIC_Init(&NVIC_InitStructure);  					
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE );                //使能指定中断
	TIM_Cmd(TIM2, ENABLE);  
}

//TIM4初始化
//用于计算转速
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5; // 抢占优先级5
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  	    // 子优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure); 

  TIM_Cmd(TIM4, ENABLE);   	
}

//获取编码器的计数值
int get_encoder_value(void)
{
	u32 buffer;
	buffer=TIM_GetCounter(TIM2)+(overflow*65536);
	return buffer;
}

//获取计数方向
u8 TIM_GetDirection(TIM_TypeDef* TIMx)
{
	// 定时器的计数方向变为只读
	return(TIMx->CR1 & TIM_CR1_DIR) ? 1 : 0;
}

//获取电机转速
//ms = 50，即采样精度为50ms
float get_speed(int encode_value,u16 ms)
{
	  u8 i = 0, j = 0;
    float temp = 0.0;
	  static float speed=0;									
    static uint8_t sp_count = 0, k = 0;
    static float speed_arr[10] = {0.0};                     // 存储速度进行滤波数组 
	  static int old_value=0,now_value=0;

	 if (sp_count == ms)                                     // 每隔 ms 毫秒，计算一次速度 
    {
		now_value = encode_value;							                 // 记录当前编码器值
		
        // 计算转速，30为减速比，4倍频，11线
		
		    /* 1000/ms指在这个ms时间内获得了xx脉冲变化值，用xx变化值/ms得到1ms的脉冲变化值
		   * 再乘以1000得到1s的变化值 */
        speed_arr[k++] = (float)((now_value - old_value) * ((1000 / ms) * 60.0) / 30 / (11*4)); 
		    old_value = now_value;								// 保存当前计数值
		
        /* 累计10次速度值，利用冒泡排序，后面做中值滤波 */
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
            
            for (i = 2; i < 8; i++)                         //去掉最高和最低的数据 
            {
                temp += speed_arr[i];                       //将中间数值累加 
            }
            
            temp = (float)(temp / 6);                       //求速度平均值
            
            /* 一阶低通滤波
             * 公式为：Y(n)= qX(n) + (1-q)Y(n-1)
             * 其中X(n)为本次采样值，Y(n-1)为上次滤波输出值，Y(n)为本次滤波输出值，q为滤波系数
             * q值越小，上一次输出对本次输出的影响越大，输出越平稳，但是对速度变化的响应也就越慢
             */
            speed = (float)( ((float)0.48 * temp) + (speed * (float)0.52) );
            k = 0;
        }
        sp_count = 0;
    }
    sp_count ++;
	  return speed;
}
