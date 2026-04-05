#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#include "usart.h"       // 串口初始化头文件
#include "app_task.h"   // RTOS任务头文件

int main(void)
{
	  SystemInit();
    // 1. 延时初始化（必须第一个）
    delay_init();

    // 2. 串口初始化
    uart_init(115200);
    printf("系统初始化成功\r\n");

    // 3. 创建任务
    Create_DHT11_Task();

    // 4. 启动RTOS
    vTaskStartScheduler();

    while(1)
    {
        
    }
}

// 【可选】FreeRTOS空闲钩子函数（若需要）
void vApplicationIdleHook(void)
{
    // 空闲任务执行的内容（可留空）
}

// 【可选】栈溢出钩子函数（调试用）
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	 printf("RenWuYiChu:%s\r\n", pcTaskName);
    while(1);
}

