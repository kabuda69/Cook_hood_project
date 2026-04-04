#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

u8 dma_buffer[100];
// 电机任务句柄
TaskHandle_t motor_task_handle = NULL;

// 电机任务函数（真正运行电机的地方）
void motor_task(void *pvParameters)
{
    u16 i;

    // 初始化电机硬件
    TIM1_dead_pwm_init(999, 71, 0, 100);
    motor_init();
    motor_start();
    motor_dir(stright);

    while (1)
    {
        // 加速
        for (i = 0; i <= 500; i += 50)
        {
            motor_speed(i);
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
            vTaskDelay(200);
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
            vTaskDelay(200);
        }

        // 减速
        for (i = 500; i >= 0; i -= 50)
        {
            motor_speed(i);
            GPIO_SetBits(GPIOC, GPIO_Pin_13);
            vTaskDelay(200);
            GPIO_ResetBits(GPIOC, GPIO_Pin_13);
            vTaskDelay(200);
        }

        vTaskDelay(1000);
    }
}


int main(void)
{
   // 1. 系统时钟初始化
    SystemInit();
    delay_init();

    // 2. LED 初始化
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);

    // ==================== FreeRTOS 任务创建 ====================
    // 创建电机任务
    xTaskCreate(
        motor_task,       // 任务函数
        "motor_task",     // 任务名
        512,              // 堆栈大小
        NULL,             // 参数
        2,                // 优先级
        &motor_task_handle // 任务句柄
    );

    // 启动调度器 —— 真正开始跑系统
    vTaskStartScheduler();

    // 调度器启动失败才会跑到这里
    while (1)
    {
    }
}