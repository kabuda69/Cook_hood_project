#ifndef __KEY_H
#define __KEY_H

#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gpiox.h"

#define KEY1_GPIO_PORT      GPIOB
#define KEY1_GPIO_PIN       GPIO_Pin_1

#define KEY2_GPIO_PORT      GPIOB
#define KEY2_GPIO_PIN       GPIO_Pin_12
//按键读取宏定义
#define KEY1_READ()         GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN)
#define KEY2_READ()         GPIO_ReadInputDataBit(KEY2_GPIO_PORT, KEY2_GPIO_PIN)

//按键按下为低电平
#define KEY_PRESSED_LEVEL   0 
//时间参数定义
#define KEY_DEBOUNCE_TIME_MS    30    /* 消抖时间：30ms */
#define KEY_LONG_PRESS_TIME_MS  1000   /* 长按时间：1000ms（1秒） */

//按键事件定义 
typedef enum {
    KEY_EVENT_NONE = 0,         /* 无事件 */
    KEY_EVENT_SHORT_PRESS,      /* 短按事件 */
    KEY_EVENT_LONG_PRESS,       /* 长按事件 */
    KEY_EVENT_LONG_PRESSING,    /* 长按持续中（用于蜂鸣器持续鸣叫） */
    KEY_EVENT_RELEASE           /* 按键释放事件 */
} KeyEvent_t;

//按键状态机状态定义 
typedef enum {
    KEY_STATE_IDLE = 0,         /* 空闲状态 */
    KEY_STATE_DEBOUNCE,         /* 消抖状态 */
    KEY_STATE_PRESSED,          /* 按下确认状态 */
    KEY_STATE_LONG_PRESS,       /* 长按状态 */
    KEY_STATE_WAIT_RELEASE      /* 等待释放状态 */
} KeyState_t;

//按键控制结构体 
typedef struct {
    KeyState_t state;           /* 当前状态 */
    TickType_t pressStartTick;  /* 按下开始时间戳（用于精确计时） */
    u8 (*readPin)(void);        /* 读取引脚电平函数指针 */
    KeyEvent_t event;           /* 当前事件 */
    u8 longPressTriggered;      /* 长按事件是否已触发 */
} Key_t;


void Key_Init(void);                        
void Key_Scan(void);                        
KeyEvent_t Key1_GetEvent(void);             
KeyEvent_t Key2_GetEvent(void);            
void Key1_ClearEvent(void);                 
void Key2_ClearEvent(void);                
u8 Key1_IsPressed(void);                    
u8 Key2_IsPressed(void); 





#endif



