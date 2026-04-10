#ifndef __CRC32_H__
#define __CRC32_H__
#include "sys.h"



u32 CRC32_Calculate(u8 *data, u32 length);//CRC32计算函数
u8 CRC32_verrifyFirmware(u8 *data,u32 totalLength);//CRC32校验函数，返回1表示校验通过，0表示校验失败











#endif


