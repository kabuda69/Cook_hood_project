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
#include "gui.h"
#include "spi.h"
#include "lcd.h"
#include "beep.h"
#include "key.h"  


//全局变量定义
SystemState_t g_systemState;                // 系统状态结构体变量
SemaphoreHandle_t g_dataMutex = NULL;       // 数据互斥锁句柄


// 任务句柄定义
static TaskHandle_t xStartTaskHandle = NULL;          // 开始任务句柄
static TaskHandle_t xKeyScanTaskHandle = NULL;        // 按键扫描任务句柄
static TaskHandle_t xSensorTaskHandle = NULL;         // 传感器采集任务句柄
static TaskHandle_t xUIDisplayTaskHandle = NULL;      // LCD显示任务句柄
static TaskHandle_t xMotorControlTaskHandle = NULL;   // 电机控制任务句柄





// 系统初始化
void System_Init(void)
{
    g_systemState.temperature = 0;// 温度初始化为0度
    g_systemState.humidity = 0;// 湿度初始化为0%
    g_dataMutex = xSemaphoreCreateMutex();

    Key_Init();;

    TIM1_dead_pwm_init(1000, 71, 0, 10); // 10kHz PWM，死区时间10
    motor_init();
}

// 开始任务创建
void StartTask_Create(void)
{
    // 创建开始任务，优先级最低，等待系统初始化完成后创建其他任务
    xTaskCreate(StartTask, "StartTask", TASK_START_STK_SIZE, NULL, 
                TASK_START_PRIORITY, &xStartTaskHandle);
}

// 开始任务
void StartTask(void *pvParameters)
{
    taskENTER_CRITICAL();
   // 创建按键扫描任务，优先级为4

    
    // 创建传感器采集任务，优先级为3
    xTaskCreate(SensorTask, "Sensor", TASK_SENSOR_STK_SIZE, NULL,
                TASK_SENSOR_PRIORITY, &xSensorTaskHandle);
       
    // 创建LCD显示任务，优先级为1
    xTaskCreate(UIDisplayTask, "UI", TASK_UI_STK_SIZE, NULL,
                TASK_UI_PRIORITY, &xUIDisplayTaskHandle);

    // 创建电机控制任务，优先级为5


    TIM4_init(5-1, 14400-1);


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
// 按键扫描任务
void KeyScanTask(void *pvParameters)
{
    KeyEvent_t key1Event, key2Event;
    
    while (1)
    {
        /* 扫描按键 */
        Key_Scan();
        
   
        /* 获取按键2事件（档位切换/风机开关） */
        key2Event = Key2_GetEvent();
        if (key2Event == KEY_EVENT_SHORT_PRESS)
        {
            //System_SwitchSpeedLevel();
            Buzzer_Beep(100,&bep);           /* 短按提示音 */
            Key2_ClearEvent();
        }
        else if (key2Event == KEY_EVENT_LONG_PRESS)
        {
           //System_ToggleMotor();         /*长按时关闭或者打开电机，无论电机在何种模式下*/
            
        }
        else if (key2Event == KEY_EVENT_LONG_PRESSING)
        {
            Beep_on(&bep);                /* 长按持续鸣叫 */
        }
        else if (key2Event == KEY_EVENT_RELEASE)
        {
            Beep_off(&bep);               /* 释放时停止鸣叫 */
            Key2_ClearEvent();
        }
        
        delay_ms(10);   				 /* 10ms扫描周期 */
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
	
	
	












