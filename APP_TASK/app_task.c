#include "app_task.h"
#include "delay.h"
#include "gpiox.h"
#include "usart.h"	
#include "motor.h"
#include "dma.h"
#include <stdio.h>
#include <string.h>
#include "mq2.h"
#include "dht11.h"



// 任务优先级、栈大小、任务句柄定义
#define DHT11_TASK_PRIO    2               // 任务优先级（根据系统调整）
#define DHT11_TASK_STACK   256             // 任务栈大小（字节/字，依RTOS而定）
TaskHandle_t Dht11Task_Handle = NULL;      // 任务句柄

extern led_d dht;// DHT11 的led灯外部定义

// DHT11测试任务函数（无限循环读取+串口打印）
static void DHT11_Task_Func(void *pvParameters)
{
    u8 temp = 0, humi = 0;
    u8 ret = 0;

    printf("DHT11 GOOD\r\n");

    while(1)
    {
        // ✅ 完全调用你自己的 DHT_Read_Data 函数，参数完全匹配
        ret = DHT_Read_Data(&temp, &humi, GPIOC, GPIO_Pin_14, &dht);

        if(ret == 1)  // 你的驱动：成功返回1
        {
            printf("TEMP：%d℃  HUMI：%d%%RH\r\n", temp, humi);
        }
        else
        {
            printf("DHT11 bad\r\n");
        }

        // RTOS 延时，不影响时序
        vTaskDelay(2000);
    }
}

void Create_DHT11_Task(void)
{
    xTaskCreate(
        DHT11_Task_Func,
        "DHT11_Task",
        DHT11_TASK_STACK,
        NULL,
        DHT11_TASK_PRIO,
        &Dht11Task_Handle
    );
}












