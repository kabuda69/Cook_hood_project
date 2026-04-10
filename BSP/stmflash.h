#ifndef __STMFLASH_H__
#define __STMFLASH_H__
#include "sys.h"  

#define STM32_FLASH_SIZE 64	 //所选STM32的Flash大小，单位为KB，这里为64KB
#define STM32_FLASH_WREN 1   //使能Flash写入功能，1为使能，0为禁止写入

//Flash基地址
#define STM32_FLASH_BASE 0x08000000 




u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void STMFLASH_Write_Nocheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite); //不检查写入函数
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);     //从指定地址开始读取指定长度的数据

void Test_Write(u32 WriteAddr,u16 WriteData);  //测试函数，向指定地址写入一个半字数据







#endif











