#include "key.h"


//定义两个按键结构体变量
static Key_t g_key1;
static Key_t g_key2;

//按键1读取引脚电平函数
static u8 Key1_ReadPin(void)
{
    return (KEY1_READ() == KEY_PRESSED_LEVEL) ? 1 : 0;
}
//按键2读取引脚电平函数
static u8 Key2_ReadPin(void)
{
    return (KEY2_READ() == KEY_PRESSED_LEVEL) ? 1 : 0;
}  

//初始化按键
void Key_Init(void)
{
    //初始化按键1，PB1引脚为上拉输入
    io_set(KEY1_GPIO_PORT,KEY1_GPIO_PIN,GPIO_Mode_IPU);
     //初始化按键2，PB12引脚为上拉输入
    io_set(KEY2_GPIO_PORT,KEY2_GPIO_PIN,GPIO_Mode_IPU);
    //初始化按键1结构体变量
    g_key1.state = KEY_STATE_IDLE;//状态为空闲
    g_key1.pressStartTick = 0;    //按键按下开始时间
    g_key1.readPin = Key1_ReadPin;//读取引脚电平函数指针
    g_key1.event = KEY_EVENT_NONE;//当前事件为无事件
    g_key1.longPressTriggered = 0;//长按事件未触发
    //初始化按键2结构体变量
    g_key2.state = KEY_STATE_IDLE;//状态为空闲
    g_key2.pressStartTick = 0;    //按键按下开始时间
    g_key2.readPin = Key2_ReadPin;//读取引脚电平函数指针
    g_key2.event = KEY_EVENT_NONE;//当前事件为无事件
    g_key2.longPressTriggered = 0;//长按事件未触发
}

//按键状态机单个处理函数
static void Key_StateMachine(Key_t *key)
{
    //读取按键引脚电平,如果按键按下返回1，否则返回0
    u8 keyPressed = key->readPin();
    //记录rtos时钟节拍数，每1ms记录一次，此时时钟节拍数加1
    TickType_t currentTick = xTaskGetTickCount();
    //按键按下持续时间
    TickType_t elapsedTime;

    switch (key->state)
    {

        //空闲状态：检测按键按下
       case KEY_STATE_IDLE:
           if (keyPressed)
           {
            key->state = KEY_STATE_DEBOUNCE;
            key->pressStartTick = currentTick;
            }
        break;

         //消抖状态：检测按键按下持续时间是否超过消抖时间
        case KEY_STATE_DEBOUNCE:
        //检测按键按下持续时间是否超过按键消抖时间
        elapsedTime = currentTick - key->pressStartTick;
            if (elapsedTime >= pdMS_TO_TICKS(KEY_DEBOUNCE_TIME_MS))
            {
                if (keyPressed)
                {
                //消抖完毕，确认按键按下
                  key->state = KEY_STATE_PRESSED;  
                  //重新记录时间，用于长短判断
                  key->pressStartTick = currentTick;
                }  else
                {
                //按键未持续按下，视为抖动，返回空闲状态
                  key->state = KEY_STATE_IDLE;
                }
            }
        break;

         //按下状态：判断按键是短按还是长按
        case KEY_STATE_PRESSED:
        //检测按键是否持续按下，如果持续按下则进入长按状态，否则进入等待释放状态
            if (keyPressed)
            {
                //检测按键按下持续时间是否超过按键消抖时间
                elapsedTime = currentTick - key->pressStartTick;
                if (elapsedTime > pdMS_TO_TICKS(KEY_LONG_PRESS_TIME_MS))
                {
                    //达到长按时间
                    key->state = KEY_STATE_LONG_PRESS;
                    key->event = KEY_EVENT_LONG_PRESS;
                    key->longPressTriggered = 1;	//标记一次长按
                }
            }
            else
            {
                // 不为长按续按下，则释放按键 
                if (!key->longPressTriggered)
                {
                    //未触发长按，则为短按 事件
                    key->event = KEY_EVENT_SHORT_PRESS;
                }
                key->state = KEY_STATE_IDLE;
                key->longPressTriggered = 0;//重置长按标记
            }
        break;


        //长按状态：持续监测，产生长按持续事件，直到按键释放
        case KEY_STATE_LONG_PRESS:
        //检测按键是否释放，如果未释放则进入长按持续中事件，否则进入释放按键事件
           if (keyPressed)
            {
                // 长按持续中，每个周期都产生持续事件
                key->event = KEY_EVENT_LONG_PRESSING;
            }
            else
            {
                //释放按键
                key->event = KEY_EVENT_RELEASE;
                key->state = KEY_STATE_IDLE;
                key->longPressTriggered = 0;
            }
        break;

            //其他状态：返回空闲状态
           default:
            key->state = KEY_STATE_IDLE;
            break;

    }

}

//按键扫描函数（需要周期性调用）
void Key_Scan(void)
{
    Key_StateMachine(&g_key1);
    Key_StateMachine(&g_key2);
}

//获取按键1当前事件
KeyEvent_t Key1_GetEvent(void)
{
    return g_key1.event;
}

//获取按键2当前事件
KeyEvent_t Key2_GetEvent(void)
{
    return g_key2.event;
}

//清除按键1当前事件
void Key1_ClearEvent(void)
{
    g_key1.event = KEY_EVENT_NONE;
}

//清除按键2当前事件
void Key2_ClearEvent(void)
{
    g_key2.event = KEY_EVENT_NONE;
}

//检查按键1是否处于按下状态（短按或长按）
u8 Key1_IsPressed(void)
{
    return (g_key1.state == KEY_STATE_PRESSED || 
            g_key1.state == KEY_STATE_LONG_PRESS) ? 1 : 0;
}


//检查按键2是否处于按下状态（短按或长按）
u8 Key2_IsPressed(void)
{
    return (g_key2.state == KEY_STATE_PRESSED || 
            g_key2.state == KEY_STATE_LONG_PRESS) ? 1 : 0;
}









