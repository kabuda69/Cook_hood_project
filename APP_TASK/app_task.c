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
#include "pid.h"
#include "wind_speed.h"
#include "iap.h"
#include "crc32.h"

//全局变量定义
SystemState_t g_systemState;                   // 系统状态结构体变量
SemaphoreHandle_t g_dataMutex = NULL;          // 数据互斥锁句柄
SemaphoreHandle_t g_speedCalcSemaphore = NULL;// 风速计算信号量句柄
SemaphoreHandle_t g_iapSemaphore = NULL;      // IAP任务信号量句柄

// 任务句柄定义
static TaskHandle_t xStartTaskHandle = NULL;          // 开始任务句柄
static TaskHandle_t xKeyScanTaskHandle = NULL;        // 按键扫描任务句柄
static TaskHandle_t xSensorTaskHandle = NULL;         // 传感器采集任务句柄
static TaskHandle_t xUIDisplayTaskHandle = NULL;      // LCD显示任务句柄
static TaskHandle_t xMotorControlTaskHandle = NULL;   // 电机控制任务句柄
static TaskHandle_t xWindSpeedTaskHandle = NULL;      //风速计算任务句柄
static TaskHandle_t xMotorControlTaskHandle = NULL;   //电机控制任务句柄
static TaskHandle_t xSpeedCalcTaskHandle = NULL;      //速度计算任务句柄
static TaskHandle_t xAntiBackflowTaskHandle = NULL;   //防回流任务句柄

#if ifopen
	static TaskHandle_t xIAPTaskHandle = NULL;/* IAP任务句柄 */
#endif

//PID控制器
extern PID_TypeDef g_speedPID;


//自动模式状态机状态,初始状态为启动阶段
typedef enum {
    AUTO_STATE_STARTUP,             //启动阶段（等待cooking event） 
    AUTO_STATE_COOKING,             //Cooking Event激活 
    AUTO_STATE_DELAY_OFF            //延时关闭阶段 
} AutoModeState_t;

//当前自动模式状态
static AutoModeState_t g_autoModeState = AUTO_STATE_STARTUP;

#if ifopen 
  //接收缓冲区，放在特定地址以便IAP任务使用
  u8 receive_buff[buff_size]  __attribute__ ((at(0X20004000)));
#endif
//模式名称字符串数组
static const char* ModeNames[] = {
    "Standby",
    "Manual ",
    "Auto   ",
    "Anti-BF"
};

//风速档位名称字符串数组
static const char* SpeedLevelNames[] = {
    "LOW ",
    "HIGH"
};

//自动模式状态名称字符串数组
static const char* AutoStateNames[] = {
    "Startup ",
    "Cooking ",
    "DelayOff"
};


// 系统初始化
void System_Init(void)
{
    g_systemState.currentMode = MODE_STANDBY;//默认待机模式
    g_systemState.speedLevel = SPEED_LOW;//默认低档
    g_systemState.motorRunning = 0;//电机初始状态为停止
    g_systemState.temperature = 0;//初始化温度为0
    g_systemState.humidity = 0;//初始化湿度为0
    g_systemState.gasConcentration = 0.0f;//初始化气体浓度为0
    g_systemState.windSpeedPWM = 0.0f;//初始化风速PWM为0
    g_systemState.actualRPM = 0.0f;//初始化实际转速为0
    g_systemState.targetRPM = 0;//初始化目标转速为0
    g_systemState.cookingEventActive = 0;//默认无Cooking Event
    g_systemState.antiBackflowActive = 0;//默认防回流未激活
    g_systemState.gasThreshold = GAS_THRESHOLD_NORMAL;//默认气体浓度阈值为正常水平
    g_systemState.autoModeCounter = 0;//自动模式计数器
    g_systemState.cookingEventCounter = 0;//Cooking Event计数器

    // 创建数据互斥锁
    g_dataMutex = xSemaphoreCreateMutex();
    // 创建风速计算二值信号量
    g_speedCalcSemaphore = xSemaphoreCreateBinary();

#if ifopen
    // 创建IAP任务二值信号量
     g_iapSemaphore = xSemaphoreCreateBinary();
#endif
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
  
    //创建按键扫描任务
    xTaskCreate(KeyScanTask, "KeyScan", TASK_KEY_STK_SIZE, NULL,
                TASK_KEY_PRIORITY, &xKeyScanTaskHandle);
    
    //创建传感器采集任务
    xTaskCreate(SensorTask, "Sensor", TASK_SENSOR_STK_SIZE, NULL,
                TASK_SENSOR_PRIORITY, &xSensorTaskHandle);
    
    //创建风速计算任务
    xTaskCreate(WindSpeedTask, "WindSpeed", TASK_WIND_SPEED_STK_SIZE, NULL,
                TASK_WIND_SPEED_PRIORITY, &xWindSpeedTaskHandle);
    
    //创建电机控制任务
    xTaskCreate(MotorControlTask, "Motor", TASK_MOTOR_STK_SIZE, NULL,
                TASK_MOTOR_PRIORITY, &xMotorControlTaskHandle);
    
    //创建UI显示任务
    xTaskCreate(UIDisplayTask, "UI", TASK_UI_STK_SIZE, NULL,
                TASK_UI_PRIORITY, &xUIDisplayTaskHandle);
    
    //创建防回流任务 
    xTaskCreate(AntiBackflowTask, "AntiBF", TASK_ANTI_BACKFLOW_STK_SIZE, NULL,
                TASK_ANTI_BACKFLOW_PRIORITY, &xAntiBackflowTaskHandle);
    
    //创建速度计算任务
    xTaskCreate(SpeedCalcTask, "SpeedCalc", TASK_SPEED_CALC_STK_SIZE, NULL,
                TASK_SPEED_CALC_PRIORITY, &xSpeedCalcTaskHandle);
#if ifopen  
    //创建IAP任务
    xTaskCreate(iap_task, "IAP", TASK_IAP_STK_SIZE, NULL,
                TASK_IAP_PRIORITY, &xIAPTaskHandle);
#endif   
   //初始化TIM4定时器
    TIM4_init(5-1, 14400-1);

    taskEXIT_CRITICAL();

    vTaskDelete(xStartTaskHandle);
}



// 按键扫描任务
void KeyScanTask(void *pvParameters)
{
    KeyEvent_t key1Event, key2Event;
    
    while (1)
    {
        // 按键扫描
        Key_Scan();
        
                //获取按键1事件（模式切换）
        key1Event = Key1_GetEvent();
        if (key1Event == KEY_EVENT_SHORT_PRESS)
        {
            System_SwitchMode();
            Buzzer_Beep(100,&bep);           // 短按提示音 
            Key1_ClearEvent();
        }
        else if (key1Event == KEY_EVENT_LONG_PRESSING)
        {
            Beep_on(&bep);                // 长按持续鸣叫
        }
        else if (key1Event == KEY_EVENT_RELEASE)
        {
            Beep_off(&bep);               // 释放时停止鸣叫
            Key1_ClearEvent();
        }
        
   
        //获取按键2事件（档位切换/风机开关）
        key2Event = Key2_GetEvent();
        if (key2Event == KEY_EVENT_SHORT_PRESS)
        {
            //System_SwitchSpeedLevel();
            Buzzer_Beep(100,&bep);           // 短按提示音
            Key2_ClearEvent();
        }
        else if (key2Event == KEY_EVENT_LONG_PRESS)
        {
           //System_ToggleMotor();         // 长按时关闭或者打开电机，无论电机在何种模式下
            
        }
        else if (key2Event == KEY_EVENT_LONG_PRESSING)
        {
            Beep_on(&bep);                // 长按持续鸣叫
        }
        else if (key2Event == KEY_EVENT_RELEASE)
        {
            Beep_off(&bep);               // 释放时停止鸣叫
            Key2_ClearEvent();
        }
        
        delay_ms(10);   				 // 10ms扫描周期
    }
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
	
	
	












