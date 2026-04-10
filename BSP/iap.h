#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  

//定义IAP地址基地址
typedef  void (*iapfun)(void);	
//第一个应用地址基地址
#define FLASH_APP1_ADDR		0x0800F000  

void iap_load_appbin(u32 appxaddr);////跳转到APP程序
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize);//写入APP程序



#endif



