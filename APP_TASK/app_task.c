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


// 按键扫描任务  周期10ms
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
            System_SwitchSpeedLevel();
            Buzzer_Beep(100,&bep);           // 短按提示音
            Key2_ClearEvent();
        }
        else if (key2Event == KEY_EVENT_LONG_PRESS)
        {
           System_ToggleMotor();         // 长按时关闭或者打开电机，无论电机在何种模式下
            
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

//切换工作模式
void System_SwitchMode(void)
{
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
    {
        /* 模式循环切换：待机 -> 手动 -> 自动 -> 防回流 -> 待机 */
        switch (g_systemState.currentMode)
        {
            case MODE_STANDBY:
                g_systemState.currentMode = MODE_MANUAL;
                break;
            case MODE_MANUAL:
                g_systemState.currentMode = MODE_AUTO;
                g_autoModeState = AUTO_STATE_STARTUP;
                break;
            case MODE_AUTO:
                g_systemState.currentMode = MODE_ANTI_BACKFLOW;
                break;
            case MODE_ANTI_BACKFLOW:
                g_systemState.currentMode = MODE_STANDBY;
                break;
        }
        
        /* 切换模式时停止电机（除自动模式外） */
        if (g_systemState.currentMode != MODE_AUTO)
        {
            g_systemState.motorRunning = 0;
            motor_stop();
        }
        
        xSemaphoreGive(g_dataMutex);
    }
}

//切换风速档位
void System_SwitchSpeedLevel(void)
{
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
    {
        /* 档位循环切换：LOW -> HIGH -> LOW */
        switch (g_systemState.speedLevel)
        {
            case SPEED_LOW:
                g_systemState.speedLevel = SPEED_HIGH;
                break;
            case SPEED_HIGH:
                g_systemState.speedLevel = SPEED_LOW;
                break;
        }
        xSemaphoreGive(g_dataMutex);
    }
}

//切换电机开关
void System_ToggleMotor(void)
{
    if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
    {
        if (g_systemState.motorRunning)
        {
            /* 关闭电机：强制切换到待机模式，确保MotorControlTask不会重新拉起电机 */
            motor_stop();
            g_systemState.motorRunning = 0;
            g_systemState.currentMode = MODE_STANDBY;
            Show_Str(0, 0, BLUE, WHITE, "Motor Stopped", 16, 0);
        }
        else
        {
            /* 开启电机：强制切换到手动模式，确保MotorControlTask持续驱动电机 */
            motor_start();
            g_systemState.motorRunning = 1;
            g_systemState.currentMode = MODE_MANUAL;
            Show_Str(0, 0, BLUE, WHITE, "Motor Started", 16, 0);
        }
        
        xSemaphoreGive(g_dataMutex);
    }
}

//获取系统状态
SystemState_t* System_GetState(void)
{
    return &g_systemState;
}



// 传感器采集任务     周期500ms
void SensorTask(void *pvParameters)
{
    u8 temp,humi;
    float gasValue;
    u8 ret;

    while (1)
    {
        //读取DHT11数据 
        ret = DHT_Read_Data(&temp,&humi,GPIOC, GPIO_Pin_14, &dht);
        if (ret == 1)//如果校验成功
        {
            /*如果此处获取2个传感器数据使用同一把锁，会导致占用资源时间过长，
            更容易带来优先级反转问题；因此此处采用两把锁*/
            if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
            {
                g_systemState.temperature = temp;
                g_systemState.humidity = humi;
                xSemaphoreGive(g_dataMutex);
            }
        }
        
        //读取MQ2数据 
        {
            gasValue = MQ2_GetGasConcentration();
            if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
            {
                g_systemState.gasConcentration = gasValue;
                xSemaphoreGive(g_dataMutex);
            }
        }
#if SENSOR_DEBUG    //传感器数据调试，右键点击该宏--goto definition--将0改成1即可使用      
        printf("Temp: %d, Humi: %d, Gas: %.2f\r\n", temp, humi, gasValue);
#endif
        delay_ms(500);  //500ms采集周期
    }
}


// 风速计算任务      周期100ms
void WindSpeedTask(void *pvParameters)
{
    u8 temp, humidity;
    float gas;
    
    //如果此处获取传感器和更新系统状态使用同一把锁，会导致占用资源时间过长，更容易带来优先级反转问题
    while (1)
    {
        // 获取传感器数据
        if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
        {
            temp = g_systemState.temperature;
            humidity = g_systemState.humidity;
            gas = (float)g_systemState.gasConcentration;
            xSemaphoreGive(g_dataMutex);
        }
        
        // 更新风速计算
        WindSpeed_Update(temp, humidity, gas);
        
        // 更新系统状态
        if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
        {
            g_systemState.windSpeedPWM = WindSpeed_GetPWM();
            g_systemState.cookingEventActive = WindSpeed_IsCookingEvent();
            xSemaphoreGive(g_dataMutex);
        }
        
        delay_ms(100);  //100ms计算周期
    }
}


//电机控制任务 - 周期50ms
void MotorControlTask(void *pvParameters)
{
    float pidOutput;
    
    while (1)
    {
        u16 pwmCompare = 0;

        // 获取实际转速
        if (xSemaphoreTake(g_dataMutex, portMAX_DELAY) == pdTRUE)
        {
            g_systemState.actualRPM = speed;
            xSemaphoreGive(g_dataMutex);
        }
        
        switch (g_systemState.currentMode)
        {
            case MODE_STANDBY:
                // 待机模式：电机停止，但仍计算风速
                motor_stop();
                g_systemState.motorRunning = 0;
                g_systemState.autoModeCounter = 0;
                g_systemState.cookingEventCounter = 0;
                g_autoModeState = AUTO_STATE_STARTUP;
                break;
                
            case MODE_MANUAL:
                // 手动模式：PID控制电机转速
                if(g_systemState.motorRunning == 0)
                {
                    motor_start();
                    g_systemState.motorRunning = 1;
                }

                if (g_systemState.motorRunning)
                {
                    //1.设置目标转速;
                    g_systemState.targetRPM = WindSpeed_GetTargetRPM(g_systemState.speedLevel);
                    //2.计算PID输出;
                    PID_SetTarget(&g_speedPID, (float)g_systemState.targetRPM);
                    //3.设置电机PWM；
                    pidOutput = PID_Calculate(&g_speedPID, speed);
                    motor_pwm_set(pidOutput);
                }
                break;
                
            case MODE_AUTO:
                // 自动模式状态机
                switch (g_autoModeState)
                {
                    case AUTO_STATE_STARTUP:
                        // 启动阶段：以最小转速运行，等待Cooking Event
                        if (!g_systemState.motorRunning)
                        {
                            g_systemState.motorRunning = 1;
                            motor_start();
                        }

                        // 使用最小转速代表自动模式开启
                        {
                            motor_pwm_set(PWM_MIN*10);
                        }
                        
                        g_systemState.autoModeCounter += 50;
                        
                        if (g_systemState.cookingEventActive)
                        {
                            // 检测到Cooking Event
                            g_autoModeState = AUTO_STATE_COOKING;
                        }
                        else if (g_systemState.autoModeCounter >= AUTO_MODE_STARTUP_TIME)
                        {
                            // 60秒内无Cooking Event，关闭自动模式
                            g_systemState.currentMode = MODE_STANDBY;
                            motor_stop();
                            g_systemState.motorRunning = 0;
                        }
                        break;
                        
                    case AUTO_STATE_COOKING:
                        // Cooking Event激活：根据传感器自动调节
                        {
                            //根据风速算法设置速度，MAXCCR为最大占空比
                            pwmCompare = WindSpeed_GetPWMCompare(MAXCCR);
                            motor_pwm_set(pwmCompare);
                        }
                        
                        g_systemState.cookingEventCounter += 50;
                        
                        if (!g_systemState.cookingEventActive)
                        {
                            // Cooking Event结束
                            g_autoModeState = AUTO_STATE_DELAY_OFF;
                            g_systemState.cookingEventCounter = 0;
                        }
                        else if (g_systemState.cookingEventCounter >= COOKING_EVENT_TIMEOUT)
                        {
                            // Cooking Event持续超过60秒，关闭自动模式
                            g_systemState.currentMode = MODE_STANDBY;
                            motor_stop();
                            g_systemState.motorRunning = 0;
                        }
                        break;
                        
                    case AUTO_STATE_DELAY_OFF:
                        // 延时关闭阶段
                        {
                            //依据风速输出转速，MAXCCR为最大占空比
                            pwmCompare = WindSpeed_GetPWMCompare(MAXCCR);
                            motor_pwm_set(pwmCompare);
                        }
                        
                        g_systemState.cookingEventCounter += 50;
                        
                        if (g_systemState.cookingEventActive)
                        {
                            // 又检测到Cooking Event
                            g_autoModeState = AUTO_STATE_COOKING;
                            g_systemState.cookingEventCounter = 0;
                        }
                        else if (g_systemState.cookingEventCounter >= COOKING_EVENT_DELAY_OFF)
                        {
                            // 10秒延时结束，关闭自动模式
                            g_systemState.currentMode = MODE_STANDBY;
                            motor_stop();
                            g_systemState.motorRunning = 0;
                        }
                        break;
                }
                break;
                
            case MODE_ANTI_BACKFLOW:
                // 防回流模式：由防回流任务控制
                if (g_systemState.antiBackflowActive && g_systemState.motorRunning)
                {
                    // 使用用户设定的档位转速
                    g_systemState.targetRPM = WindSpeed_GetTargetRPM(g_systemState.speedLevel);
                    PID_SetTarget(&g_speedPID, (float)g_systemState.targetRPM);
                    pidOutput = PID_Calculate(&g_speedPID, speed);
                    motor_pwm_set(pidOutput);
                }
                break;
        }
        
        delay_ms(50);   /* 50ms控制周期 */
    }
}



// LCD显示任务    周期200ms
void UIDisplayTask(void *pvParameters)
{
	 char dispBuf[32];
    
    /* 清屏 */
    LCD_Clear(WHITE); 
    
    while (1)
    {
        /*显示当前模式 */
        sprintf(dispBuf, "Mode:%s", ModeNames[g_systemState.currentMode]);
        Show_Str(0, 20, BLUE, WHITE, (u8*)dispBuf, 16, 0);
        
        // 显示档位
        sprintf(dispBuf, "Level:%s", SpeedLevelNames[g_systemState.speedLevel]);
        Show_Str(0, 40, BLUE, WHITE, (u8*)dispBuf, 16, 0);
 
        // 显示风速PWM
        sprintf(dispBuf, "WIND:%.1f%%  ", g_systemState.windSpeedPWM);
        Show_Str(0, 60, BLUE, WHITE, (u8*)dispBuf, 16, 0);
        
        // 显示实际转速
        sprintf(dispBuf, "RPM:%.0f    ", g_systemState.actualRPM);
        Show_Str(0, 80, BLUE, WHITE, (u8*)dispBuf, 16, 0);
        
        // 显示自动模式状态
        sprintf(dispBuf, "Auto:%s", AutoStateNames[g_autoModeState]);
        Show_Str(0, 100, BLUE, WHITE, (u8*)dispBuf, 16, 0);
        
        // 显示自动模式启动计时
        sprintf(dispBuf, "AMCnt:%ds  ", g_systemState.autoModeCounter / 1000);
        Show_Str(0, 120, BLUE, WHITE, (u8*)dispBuf, 16, 0);
        
        // 显示Cooking Event计时
        sprintf(dispBuf, "CECnt:%ds  ", g_systemState.cookingEventCounter / 1000);
        Show_Str(0, 140, BLUE, WHITE, (u8*)dispBuf, 16, 0);

        // 200ms 刷新
       delay_ms(200);
    }
}
	

//防回流任务    周期100ms
void AntiBackflowTask(void *pvParameters)
{
    u8 isDetected = 0; // 检测到气体浓度超过阈值的标志

    while (1)
    {
        // 判断是否是防回流模式
        if (g_systemState.currentMode == MODE_ANTI_BACKFLOW)
        {
            isDetected = (g_systemState.gasConcentration >= g_systemState.gasThreshold) ? 1 : 0;

            if(isDetected)
            {
                //防止在极端情况下会重启风机
                //避免气体浓度反复跳变，导致风机不停开关。
                if(g_systemState.gasConcentration < GAS_THRESHOLD_HIGH)
                {
                        // 气体超过NORMAL阈值，启动风机
                        g_systemState.antiBackflowActive = 1;
                        g_systemState.motorRunning = 1;
                        motor_start();
                        
                        // 立即切换到HIGH阈值，防止重复触发
                        g_systemState.gasThreshold = GAS_THRESHOLD_HIGH;
                }
            }
            else if(g_systemState.gasConcentration < GAS_THRESHOLD_NORMAL)
            {
                // 气体浓度降到NORMAL阈值以下，停止风机
                g_systemState.antiBackflowActive = 0;
                g_systemState.motorRunning = 0;
                motor_stop();
                g_systemState.gasThreshold = GAS_THRESHOLD_NORMAL;
            }
            
        }
        else
        {
            // 非防回流模式，重置所有状态
            g_systemState.antiBackflowActive = 0;
            g_systemState.gasThreshold = GAS_THRESHOLD_NORMAL;
        }
        
        delay_ms(100);  /* 100ms检测周期 */
    }
}

//速度计算任务 - 周期50ms
void SpeedCalcTask(void *pvParameters)
{
    int encoderCount;
    
    while (1)
    {
        // 等待TIM4中断发送的信号量
        if (xSemaphoreTake(g_speedCalcSemaphore, portMAX_DELAY) == pdTRUE)
        {
            // 获取编码器计数值
            encoderCount = get_encoder_value();
            
            // 计算电机转速（50ms计算一次）
            speed = get_speed(encoderCount, 50);
        }
    }
}

//TIM4中断服务程序 - 用于速度计算定时器
void TIM4_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        
        // 发送信号量通知SpeedCalcTask执行速度计算
        xSemaphoreGiveFromISR(g_speedCalcSemaphore, &xHigherPriorityTaskWoken);
        
        // 如果需要，触发上下文切换
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

//TIM2中断服务程序 - 用于编码器计数溢出处理
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        // 根据计数方向更新溢出计数器
        if (TIM_GetDirection(TIM2))
        {
            overflow--;     /* 递减计数 */
        }
        else
        {
            overflow++;     /* 递增计数 */
        }
    }
}

// Bootloader任务,用于固件升级
#if ifopen
void iap_task(void *pvParameters)
{
	uint16_t receivedLength;// 保存接收到的数据长度
	u32 firmwareLen;  // 保存实际固件长度（不含CRC）
	while(1)
	{
		xSemaphoreTake(g_iapSemaphore,portMAX_DELAY);	//等待信号量，portMAX_DELAY代表无限等
#if DEBUG
		printf("excute iap_task");//调试用
#endif
		receivedLength = GetReceivedDataLength();//计算所接收的数据量，需要将该变量放在等待信号量函数后，以确保及时更新
		if(receive_buff[0])//看是否接收到了APP代码
		{
				printf("received the size of firmware=%d\r\n",receivedLength);

				/*==================== CRC32校验 ====================*/
				/* 校验接收到的固件数据完整性 */
				/* 数据格式：[固件数据][4字节CRC32] */
				if(CRC32_VerifyFirmware(receive_buff, receivedLength) == 0)
				{
                    /* CRC校验失败，清空缓冲区并等待重传 */
                    #if DEBUG
                        printf("CRC32 verify failed!\r\n");
                    #endif
					Show_Str(0,0,BLUE,WHITE,"CRC32 Error!",16,0);
					memset(receive_buff,0,buff_size);   //清空数组
					MYDMA_Enable(DMA1_Channel5);//开启一次DMA传输，用于下次更新
					FLASH_ErasePage(FLASH_APP1_ADDR);
				}
				else
				{
                    /* CRC校验通过，继续后续流程 */
                    #if DEBUG
                        printf("CRC32 verify passed!\r\n");
                    #endif
					firmwareLen = receivedLength - 4;  /* 实际固件长度 = 总长度 - 4字节CRC */
				
					/*拷贝、跳转*/
					if(((*(vu32*)(0X20004000+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
					{	
                        LCD_Clear(WHITE);               /* 清屏 */ 
						Show_Str(0,80,BLUE,WHITE,"Firmware updating!",16,0);/*打印信息在LCD屏幕上代表正在更新*/

						FLASH_ErasePage(FLASH_APP1_ADDR);/*先擦除APP区的flash，防止有数据残留--不甘心的咸鱼*/

						iap_write_appbin(FLASH_APP1_ADDR,receive_buff,firmwareLen);//更新FLASH代码（只写入固件数据，不含CRC）

                        #if DEBUG
                            printf("firmware updated complete!\r\n");
                        #endif
                        
                        Show_Str(0,100,BLUE,WHITE," update completed",16,0);/*打印信息在LCD屏幕上代表更新完成*/

						if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
						{	
                            #if DEBUG
                                printf("firmware execution started!\r\n");
                            #endif
                            Show_Str(0,120,BLUE,WHITE," excute app",16,0);/*打印信息在LCD屏幕上代表执行APP*/
							iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
						}
						else{
                            #if DEBUG
                                printf("non-FLASH application, cannot execute!\r\n");
                            #endif
							
						}	
					}
					/*如果判断固件失败，先将接收APP数组情况，防止数据残留并重新开启DMA，用于下次更新--不甘心的咸鱼*/
					else{
						memset(receive_buff,0,buff_size);   //清空数组
						MYDMA_Enable(DMA1_Channel5);//开启一次DMA传输，用于下次更新
                        #if DEBUG
                            printf("error! firmware address judgment failed\r\n");
                        #endif
						
					}
				}
		}
	}
}

//DMA中断任务，用于接收APP程序数据
void DMA1_Channel5_IRQHandler(void) 
{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;/*释放二值信号量所需参数，初始化为pdFalse*/
		// 检查DMA传输完成中断
		if (DMA_GetITStatus(DMA1_IT_TC5))
		{
            // 清除中断标志
            DMA_ClearITPendingBit(DMA1_IT_TC5);

 #if DEBUG
            printf("CNDTR: %d\n", DMA_GetCurrDataCounter(DMA1_Channel5)); // 调试用
#endif
            // 释放信号量，通知iap_task处理数据
            xSemaphoreGiveFromISR(g_iapSemaphore, &xHigherPriorityTaskWoken);/*释放二值信号量*/	
		}
		/* 如果需要的话进行一次任务切换 */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
#endif


