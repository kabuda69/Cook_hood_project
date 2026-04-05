#ifndef __APP_TASK_H
#define __APP_TASK_H


#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "dma.h"

void Create_DHT11_Task(void);


void System_Init(void);



#endif


