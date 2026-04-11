/*
 * 油烟机控制系统主程序
 * 基于STM32F103C8T6 + FreeRTOS
 * 
 * 功能说明：
 * 1. 待机模式：风机停止，持续计算风速
 * 2. 手动模式：手动调速，PID控制
 * 3. 自动模式：根据传感器自动调节风速
 * 4. 防回流模式：气体浓度超阈值时启动风机
 * 5. 固件更新：通过usart+dma接收固件，接收完成后跳转到app区，boot+app双区架构
 * 6. UI显示功能
 * 7.编码器测速功能：通过1ms中断检测编码器获取的计数变化量来计算电机转速
 * 8.直流有刷电机驱动功能：通过PWM互补输出驱动H桥
 * 9.CRC32检验功能：通过python脚本预处理app.bin，添加CRC32检验码，用于固件更新
 * 按键功能：
 * - 按键1(PB1)：短按切换模式
 * - 按键2(PB12)：短按切换档位，长按开关风机
 */

#include "main.h"


static void Hardware_Init(void)
{
     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
      delay_init();
      
       // PWM频率=72MHz/(psc+1)/(arr+1)，这里设置为1kHz，死区时间10个时钟周期
      TIM1_dead_pwm_init(999,71,0,10);
    motor_init();

      TIM2_encode_init(0xFFFF, 0);
	
     Beep_Init(&bep,GPIOB,GPIO_Pin_15);	

      Key_Init();

      MQ2_Init();

      PID_Init(&g_speedPID, 14.0f, 1.65f, 0.0f, 1000.0f, 0.0f);

      WindSpeed_Init();

      LCD_Init();

      uart_init(115200);	

#if ifopen
	MYDMA_Config(DMA1_Channel5,(u32)&USART1->DR,(u32)receive_buff,buff_size);
#endif  

    Show_Str(0, 0, BLUE, WHITE, " Init Complete! ", 16, 0);
    delay_ms(500);

}
int main(void)
{
    /* 硬件初始化 */
    Hardware_Init();
    
    /* 系统状态初始化 */
    System_Init();
    
    /* 创建开始任务 */
    StartTask_Create();
    
    /* 启动FreeRTOS调度器 */
    vTaskStartScheduler();
	
    while(1)
    {
        
    }
}


