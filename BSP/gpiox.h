#ifndef __GPIOX_H
#define __GPIOX_H

#include "sys.h"

typedef GPIO_TypeDef*   gpioled;

typedef struct{
		gpioled port;
		uint16_t pin;
}led_d;

void io_set(gpioled port,u16 pin,GPIOMode_TypeDef mode);
void io_set_bit(gpioled port,u16 pin);
void io_reset_bit(gpioled port,u16 pin);


#endif
