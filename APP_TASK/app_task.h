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
} SystemState_t;


extern SystemState_t g_systemState;

void System_Init(void);
void StartTask_Create(void);
void StartTask(void *pvParameters);
void SensorTask(void *pvParameters);
void UIDisplayTask(void *pvParameters);


#endif


