#include "sys.h"

//进入低功耗模式
void WFI_SET(void)
{
   __ASM volatile("wfi");
}
//关闭全局中断
void INTX_DISABLE(void)
{
  __ASM volatile("cpsid i");
}
//开启全局中断
void INTX_ENABLE(void)
{
  __ASM volatile("cpsie i");
}
//设置主栈指针
//addr：栈顶地址
__asm void MSR_MSP(u32 addr) 
{
	// 将参数 addr 写入 MSP堆栈指针
    MSR MSP, r0 		
	// 通过 LR 返回（保持状态）
    BX r14
}

