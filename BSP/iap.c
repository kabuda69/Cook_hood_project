#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h"

iapfun jump2app;//函数指针,指向应用程序入口地址
u16 iapbuf[512];//512个字节的缓冲区

//写APP固件到Flash
//参数:
//appxaddr:要写入的起始地址(此地址必须为2的倍数!!)
//appbuf:要写入的数据的指针
//appsize:半字（16位数据）的数量
//返回值:
//无
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
  u16 t;
  u16 i=0;
  u16 temp;
  u32 fwaddr=appxaddr;//要写入的地址
  u8 *dfu=(u8 *)appbuf;//要写入的数据指针
 for (t=0;t<appsize;t+=2)//每次取2个字节
 {
    temp=(u16)dfu[1]<<8;//高字节,先左移8位
    temp+=(u16)dfu[0];//低字节,直接相加
    dfu+=2;//指针偏移2个字节
    iapbuf[i++]=temp;//放入缓冲区
    if (i==512)//缓冲区满
    {
        i=0;//清空缓冲区
        STMFLASH_Write(fwaddr,iapbuf,512);//写入一个扇区
        fwaddr+=1024;//地址偏移一个扇区
    }
 }
 if(i)//缓冲区不为空
 {
    STMFLASH_Write(fwaddr,iapbuf,i);//不足512个字节,写入缓冲区
 };
}

//跳转到APP程序
//appxaddr:要跳转的起始地址(此地址必须为2的倍数!!)
void iap_load_appbin(u32 appxaddr)
{
    __disable_irq();//关闭全局中断，防止跳转过程中被中断打断
    //关闭外设
    DMA_Cmd(DMA1_Channel5, DISABLE);//关闭DMA1通道5
    USART_Cmd(USART1, DISABLE);//关闭串口,否则无法使用DMA
    //清除中断标志位
    DMA_ClearITPendingBit(DMA1_IT_TC5);//清除DMA1通道5的传输完成中断标志
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);//清除USART1的接收中断标志


    //判断地址是否合法
    if (((*(vu32*)appxaddr) & 0x2FFE0000) == 0x20000000)//判断地址是否为应用程序入口地址
    {
       
        jump2app = (iapfun)*(vu32*)(appxaddr + 4);//函数指针指向复位地址的下4字节
        MSR_MSP(*(vu32*)appxaddr);//设置主堆栈指针为复位地址的值
        jump2app();//跳转到应用程序
    }
}




