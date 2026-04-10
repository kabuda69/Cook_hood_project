#include "stmflash.h"
#include "delay.h"
#include "usart.h"




//读出半字数据函数(16位数据)
//参数faddr是要读取的地址(此地址必须为2的倍数!!)
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(u16 *)faddr;
}

//读出flash函数
//ReadAddr:要读取的起始地址(此地址必须为2的倍数!!)
//pBuffer:要读取的数据的指针
//NumToRead:半字（16位数据）的数量
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

#if STM32_FLASH_WREN //如果使能了写入功能
//不检查写入该地址的数据函数
//WriteAddr:要写入的地址(此地址必须为2的倍数!!)
//pBuffer:要写入的数据的指针
//NumToWrite:半字（16位数据）的数量
void STMFLASH_Write_Nocheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
    u16 i;
    for(i=0;i<NumToWrite;i++)
    {
        FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
        WriteAddr+=2;//地址增加2
    }  
}

//根据Flash大小定义扇区大小
#if STM32_FLASH_SIZE<256
	#define STM_SECTOR_SIZE 1024 //字节
#else 
	#define STM_SECTOR_SIZE	2048
#endif	
//flash数组，大小为一个扇区的大小一半
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];

//写入flash函数
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
    u32 secpos;//扇区地址
    u16 secoff; //扇区内偏移地址(16位字计算)
    u16 secremain;//扇区剩余空间(16位字计算)
    u16 i;//临时变量
    u32 offaddr;//去掉Flash基地址后的偏移地址

    if(WriteAddr<STM32_FLASH_BASE||(WriteAddr)>=(STM32_FLASH_BASE+STM32_FLASH_SIZE*1024))return;//如果地址不在Flash范围内，返回错误
    FLASH_Unlock();                     //解锁
    offaddr=WriteAddr-STM32_FLASH_BASE; //偏移地址
    secpos=offaddr/STM_SECTOR_SIZE;     //扇区地址  0~127
    secoff=(offaddr%STM_SECTOR_SIZE)/2; //扇区内偏移地址(2字节计算)
    secremain=STM_SECTOR_SIZE/2-secoff; //扇区剩余空间(2字节计算)    


    if(NumToWrite<=secremain)secremain=NumToWrite;//不大于剩余空间
    while(1)
    {
        STMFLASH_Read(STM32_FLASH_BASE+secpos*STM_SECTOR_SIZE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
        for(i=0;i<secremain;i++)//校验数据
        {
            if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//需要擦除
        }
        if(i<secremain)//需要擦除
        {
            FLASH_ErasePage(STM32_FLASH_BASE+secpos*STM_SECTOR_SIZE);//擦除这个扇区
            for(i=0;i<secremain;i++)//复制
            {
                STMFLASH_BUF[i+secoff]=pBuffer[i];//复制数据到缓冲区
            }
            STMFLASH_Write_Nocheck(STM32_FLASH_BASE+secpos*STM_SECTOR_SIZE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区
        }else STMFLASH_Write_Nocheck(WriteAddr,pBuffer,secremain);//直接写入扇区剩余区间（注意这个函数只能写入2的倍数个字节！）

        if(NumToWrite==secremain)break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff=0;//偏移位置为0

            pBuffer+=secremain;  //指针偏移
            WriteAddr+=secremain*2; //地址偏移

            NumToWrite-=secremain; //字节数递减
            if(NumToWrite>STM_SECTOR_SIZE/2)secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
            else secremain=NumToWrite; //下一个扇区可以写完了
        }
    }
    FLASH_Lock();                     //锁
}
#endif




//测试写入flash函数
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字 
}

























