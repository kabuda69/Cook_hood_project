#ifndef __APP_TASK_H
#define __APP_TASK_H


#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "dma.h"

typedef struct {
    uint8_t temperature;
    uint8_t humidity;
    float gasConcentration;
} SystemState_t;


extern SystemState_t g_systemState;
//时序参数定义（ms）
#define AUTO_MODE_STARTUP_TIME      60000   /* 自动模式启动等待时间：60秒 */
#define COOKING_EVENT_TIMEOUT       60000   /* Cooking Event超时时间：60秒 */
#define COOKING_EVENT_DELAY_OFF     10000   /* Cooking Event结束后延时关闭：10秒 */
//任务优先级定义
#define TASK_START_PRIORITY         1       /* 开始任务优先级 */
#define TASK_KEY_PRIORITY           4       /* 按键扫描任务优先级 */
#define TASK_SENSOR_PRIORITY        3       /* 传感器采集任务优先级 */
#define TASK_WIND_SPEED_PRIORITY    3       /* 风速计算任务优先级 */
#define TASK_MOTOR_PRIORITY         5       /* 电机控制任务优先级 */
#define TASK_UI_PRIORITY            1       /* UI显示任务优先级 */
#define TASK_ANTI_BACKFLOW_PRIORITY 2       /* 防回流任务优先级 */
#define TASK_IAP_PRIORITY           7       /* IAP任务优先级 */
#define TASK_SPEED_CALC_PRIORITY    6       /* 速度计算任务优先级（最高，保证及时响应中断） */
//任务栈大小定义
#define TASK_START_STK_SIZE         64     /* 开始任务栈大小 */
#define TASK_KEY_STK_SIZE           64     /* 按键扫描任务栈大小 */
#define TASK_SENSOR_STK_SIZE        128     /* 传感器采集任务栈大小 */
#define TASK_WIND_SPEED_STK_SIZE    64     /* 风速计算任务栈大小 */
#define TASK_MOTOR_STK_SIZE         256     /* 电机控制任务栈大小 */
#define TASK_UI_STK_SIZE            256     /* UI显示任务栈大小 */
#define TASK_ANTI_BACKFLOW_STK_SIZE 64     /* 防回流任务栈大小 */
#define TASK_SPEED_CALC_STK_SIZE    128     /* 速度计算任务栈大小 */
#define TASK_IAP_STK_SIZE           256     /* IAP任务栈大小 */






void System_Init(void);
void StartTask_Create(void);
void StartTask(void *pvParameters);
void SensorTask(void *pvParameters);
void UIDisplayTask(void *pvParameters);


#endif


