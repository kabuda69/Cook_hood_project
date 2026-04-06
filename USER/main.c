#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#include "delay.h"
#include "lcd.h"
#include "dht11.h"     
#include "app_task.h"   // RTOS任务头文件

int main(void)
{
    System_Init();
    delay_init(); 
	  
	
	
	  SPI1_Init();
    LCD_Init();
    
    
    StartTask_Create();
    vTaskStartScheduler();
	
    while(1)
    {
        
    }
}


