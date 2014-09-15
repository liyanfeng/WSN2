//头文件
#include <stdio.h>
#include <ioCC2530.h>

#define uint   unsigned  int
#define uchar  unsigned  char

#define key1  P0_1
#define key2  P0_4

volatile uint counter=0;                   //统计溢出次数
volatile uint clickcounter=0;              //统计溢出次数
volatile uint flashcounter=0;              //统计溢出次数
volatile uint myrate=60;                   //默认传感器数据输出1秒一次
volatile uchar LEDFlag1=0;                 //标志是否要闪烁
volatile uchar LEDFlag2=0;                 //标志是否要闪烁

//延时子函数
#define led1 P1_0
#define led2 P1_2
#define led3 P1_3
#define led4 P2_0

uint8 gdat1,gdat2;

#pragma optimize=none      
void DelaymS (unsigned int dly)                   
{
	unsigned int i;
	for ( ; dly>0; dly--)
	{
		for (i=0; i<1032; i++);        
	}
}

//32M晶振初始化
void xtal_init(void)
{
    CLKCONCMD &= ~0x40;              //晶振
    while(!(SLEEPSTA & 0x40));      //等待晶振稳定
    CLKCONCMD &= ~0x47;             //TICHSPD128分频，CLKSPD不分频
    SLEEPCMD |= 0x04; 		 //关闭不用的RC振荡器
}

//LED灯初始化
void led_init(void)
{
  P1SEL  = 0x00;          //P1为普通 I/O 口
  P1DIR |= 0x0D;          //P1.0 P1.1 P1.2 P1.3 输出

  P2SEL  = 0x00;
  P2DIR |= 0x01;
  
  led1 = 0;
  led2 = 0;
  led3 = 0;
  led4 = 0;
  
  P0SEL = 0x00;           //按键初始化
  P0DIR = 0x00;
}


/*UART0通信初始化
-------------------------------------------------------*/
void Uart0Init(unsigned char StopBits,unsigned char Parity)
{
   P0SEL |=  0x0C;                  //初始化UART0端口
   PERCFG&= ~0x01;                  //选择UART0为可选位置一
   U0CSR = 0xC0;                    //设置为UART模式,而且使能接受器
   U0GCR = 8;
   U0BAUD = 59;                    //设置UART0波特率为115200bps
   U0UCR |= StopBits|Parity;        //设置停止位与奇偶校验
}


/*UART0发送数据
-------------------------------------------------------*/
void  Uart0Send(unsigned char data)
{
  while(U0CSR&0x01);    //等待UART空闲时发送数据
  U0DBUF = data;
}

/*UART0发送字符串
-------------------------------------------------------*/
void Uart0SendString(unsigned char *s)
{
  while(*s != 0)         //依次发送字符串s中的每个字符
    Uart0Send(*s++);
}


__near_func int putchar(int ch)
{
    U0DBUF = ch;
    while(UTX0IF == 0);
    UTX0IF = 0;
    return ch;
}

/*UART0接受数据
-------------------------------------------------------*/
unsigned char Uart0Receive(void)
{
  unsigned char data;
  while(!(U0CSR&0x04)); //查询是否收到数据，否则继续等待
  data=U0DBUF;          //提取接收到的数据
  return data;          
}

void InitialT1test(void)
{
    //初始化计数器1
    T1CTL = 0x05;    
    IEN1|=0X02;  //定时器1中断使能
     EA=1;   //开总中断
}

int uWaveDistance(void);

#pragma vector = T1_VECTOR      //中断服务子程序
 __interrupt void T1_ISR(void)            
 {       
          
         
      ++counter;
      ++clickcounter;

      clickcounter%=100;
      if(counter>myrate){
          counter=0;  
          //printf("距离 %u CM\r\n", uWaveDistance());    
          dht11_update();   
      }
      
      if(LEDFlag1>0||LEDFlag2>0)    
      {
        
           ++flashcounter;
           if(flashcounter>10)
           {
               flashcounter=0;
               if(LEDFlag1)
               {
                   printf("led1 falsh\r\n");
                   led1=!led1;
                   LEDFlag1=0;
               }
               if(LEDFlag2)
               {
                   printf("led2 falsh\r\n");
                   led2=!led2;
                   LEDFlag2=0;
               }
           }
           
      }
     
      T1IF=0;       
 }

void uWaveInit(void);

//主函数
void main(void)    
{
      EA=0;                   //禁用中断
      xtal_init();
      led_init();  
      Uart0Init(0, 0);
      Uart0SendString("  超声波测距实验\r\n");
     // uWaveInit();
      dht11_io_init();
      InitialT1test();
       while(1) {
            if (key1==0) {
                 if(clickcounter<10)
                 {
                      continue;
                 }
                 myrate+=2;
                 printf("now rate :%f/s\r\n",60.0/myrate);
                 led1=!led1;
                 LEDFlag1=1;
                 clickcounter=0;
           } 
           if (key2==0) {
                 if(clickcounter<10)
                 {
                      continue;
                 }
                 if(myrate>10)
                 {
                      myrate-=2;
                      printf("now rate :%f/s\r\n",60.0/myrate);
                      led2=!led2;
                      LEDFlag2=1;
                      flashcounter=0;
                 }
                 else
                 {
                      printf("rate is too low\r\n");
                      led1=!led1;
                      LEDFlag1=1;
                      led2=!led2;
                      LEDFlag2=1;
                      flashcounter=0;
                 }
                 clickcounter=0;
           } 
      }
}

