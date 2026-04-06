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
#include "GUI.h"
#include "spi.h"
#include "lcd.h"


SystemState_t g_systemState;
SemaphoreHandle_t g_dataMutex = NULL;

static TaskHandle_t xStartTaskHandle = NULL;
static TaskHandle_t xSensorTaskHandle = NULL;
static TaskHandle_t xUIDisplayTaskHandle = NULL;

// 系统初始化
void System_Init(void)
{
    g_systemState.temperature = 0;
    g_systemState.humidity = 0;
    g_dataMutex = xSemaphoreCreateMutex();
}

// 开始任务创建
void StartTask_Create(void)
{
    xTaskCreate(StartTask, "StartTask", 512, NULL, 1, &xStartTaskHandle);
}

// 开始任务
void StartTask(void *pvParameters)
{
    taskENTER_CRITICAL();

    // 传感器任务
    xTaskCreate(SensorTask, "SensorTask", 512, NULL, 2, &xSensorTaskHandle);
    
    // LCD显示任务
    xTaskCreate(UIDisplayTask, "UIDisplayTask", 512, NULL, 3, &xUIDisplayTaskHandle);

    taskEXIT_CRITICAL();
    vTaskDelete(xStartTaskHandle);
}

// 传感器采集任务
void SensorTask(void *pvParameters)
{
    u8 temp, humi;
    while(1)
    {
        if(DHT_Read_Data(&temp, &humi, GPIOC, GPIO_Pin_14, &dht))
        {
            if(xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
            {
                g_systemState.temperature = temp;
                g_systemState.humidity = humi;
                xSemaphoreGive(g_dataMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// LCD显示任务
void UIDisplayTask(void *pvParameters)
{
	 char dispBuf[32];
    
    /* 清屏 */
    LCD_Clear(WHITE); 
    
    while (1)
    {
        // 读取温湿度（线程安全）
        uint8_t temp, humi;
        if(xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
        {
            temp = g_systemState.temperature;
            humi = g_systemState.humidity;
            xSemaphoreGive(g_dataMutex);
        }

        // ====================== 只显示温湿度 ======================
        sprintf(dispBuf, "Temp: %d C", temp);
        Show_Str(0, 20, BLUE, WHITE, (u8*)dispBuf, 16, 0);

        sprintf(dispBuf, "Humi: %d %%", humi);
        Show_Str(0, 50, BLUE, WHITE, (u8*)dispBuf, 16, 0);

        // 200ms 刷新
       delay_ms(200);
    }
}
	
	
	












