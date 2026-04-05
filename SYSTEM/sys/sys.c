#include "sys.h"

//THUMB指令,打开低功耗模式
void WFI_SET(void)
{
   __ASM volatile("wfi");
}
//关闭所有中断
void INTX_DISABLE(void)
{
  __ASM volatile("cpsid i");
}
//开启所有中断
void INTX_ENABLE(void)
{
  __ASM volatile("cpsie i");
}
//设置主栈指针
//addr栈地址
__asm void MSR_MSP(u32 addr) 
{
	// 将addr的值加载到r0寄存器中
    MSR MSP, r0 		
	// ͨ用bx指令跳转到栈顶（LR寄存器的值为0），开始执行程序
    BX r14
}

