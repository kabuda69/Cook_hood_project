#ifndef _SPI_H_
#define _SPI_H_

#include "sys.h"



u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte);
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet);
void SPI1_Init(void);

#endif



