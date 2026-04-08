#include "dht11.h"
#include "gpiox.h"


u8  BUF[5]={0x00,0x00,0x00,0x00,0x00};    //存储读取的温湿度信息
u32 sum=0;         						  //校验值


// DHT11 启动函数，通过拉低引脚至少18ms来启动DHT11传感器
void DHT11_Start(led_d *io)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // 时钟选择，GPIO初始化
    chushi(io);
    GPIO_InitStructure.GPIO_Pin = io->pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(io->port, &GPIO_InitStructure);
    GPIO_ResetBits(io->port, io->pin); // 拉低引脚
    delay_xms(20); // 等待至少18ms   
    GPIO_SetBits(io->port, io->pin); // 拉高引脚
    delay_us(30); // 等待20-40us
    GPIO_ResetBits(io->port, io->pin); // 拉低引脚
}
// DHT11 读取函数，配置引脚为输入模式，并读取引脚状态
void DHT11_Read(led_d *io)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    chushi(io);
    GPIO_InitStructure.GPIO_Pin = io->pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 输入模式，浮空
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(io->port, &GPIO_InitStructure);
    
}

// DHT11 读取一个字节数据函数，读取8位数据并返回
//temp是一个临时变量，用于存储当前读取的位，ReadDat用于存储最终的字节数据
//readpin(io)函数用于读取引脚状态，返回0或1，根据DHT11的协议，'0'代码高电平时间26~28us，'1'代码高电平时间70us，通过延时30us来判断当前位是0还是1
u8 DHT_Read_Byte(led_d *io)
{
   u8 t=0;
   u8 i;
   u8 temp;
   u8 ReadDat;
  // DHT11 发送40位数据，分为5个字节，每个字节8位
 	for(i=0;i<8;i++)
	{
        // 等待引脚拉低，表示数据开始传输,如果引脚一直没有拉低，t会增加，防止程序卡死
		while(readpin(io)==0&&t<100)  
		{		
			delay_us(1);
			t++;  //防止卡死
		}
		t=0;
		//由于‘0’代码高电平时间26~28us，1代码高电平时间70us，延时30us，可判断高低电平，数字0读取到的是低电平，高电平则反之
		delay_us(30);
		temp=0;
        // 读取引脚状态，如果是高电平则temp=1，否则temp=0
		if(readpin(io)==1) temp=1;		
		// 等待引脚拉高，准备读取下一个数据位,如果引脚一直没有拉高，t会增加，防止程序卡死	
		while(readpin(io)==1&&t<100)
		{		
			delay_us(1);
			t++;
		}
		t=0;
		ReadDat<<=1; //左移1位，DHT11数据由高到低发送
		ReadDat|=temp;//将当前位的值temp（0或1）与ReadDat进行按位或运算，存储到ReadDat中
	}	
	return ReadDat;
}

// DHT11 读取一个字节数据函数，读取8位数据并返回
//0,失败，1成功。temp表温度，humi表湿度，port和pin是引脚信息，io是DHT11的led结构体
u8 DHT_Read_Data(u8 *temp,u8 *humi,gpioled port,u16 pin,led_d *io)
{
    u8 i;
	u8 t = 0;
	io->port=port;
	io->pin=pin;
	DHT11_Start(io);
	DHT11_Read(io);
	delay_us(20);
	//延时20us，低电平80us，还剩60us，检查是否是低电平以确定是否有响应信号
	if(readpin(io)==0)  //如果读取到低电平，证明DHT11响应
	{
		while(readpin(io)==0&&t<100)//接收响应信号低电平剩余60us，等待变高电平
		{
			delay_us(1);
			t++;			
		}
		t=0;//超过100us自动向下运行，以免卡死
		while(readpin(io)==1&&t<100)//接收响应信号高电平80us，等待变低电平
			{
				delay_us(1);
				t++;			
			}
		t=0;
		for(i=0;i<5;i++)  //接收40位数据
			{
				BUF[i]=DHT_Read_Byte(io);//读出1个字节
			}
		delay_us(50);//结束信号
	}
    // BUF[0]：湿度整数值 BUF[1]：湿度小数值 BUF[2]：温度整数值 BUF[3]：温度小数值 BUF[4]：校验位，都为8位数据
	sum=BUF[0]+BUF[1]+BUF[2]+BUF[3];		
	if(BUF[4]==(u8)sum)  //校验  
	{
		*humi=BUF[0];
		*temp=BUF[2];
		return 1;    
	}
	else
		return 0;   
}

//时钟选择，GPIO初始化
//根据传入的io结构体中的port成员选择对应的GPIO时钟，并使能该时钟
void chushi(led_d *io)
{
		if(io->port==GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 
		else if(io->port==GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		else if(io->port==GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
		else if(io->port==GPIOD) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
		else if(io->port==GPIOE) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
		else if(io->port==GPIOF) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
		else if(io->port==GPIOG) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

}


// 读取引脚状态
// 返回值：0 或 1，表示引脚的电平状态，0 表示低电平，1 表示高电平
u8 readpin(led_d *io)
{
	return GPIO_ReadInputDataBit(io->port,io->pin);
}




