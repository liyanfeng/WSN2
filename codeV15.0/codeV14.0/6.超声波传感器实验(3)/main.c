//头文件

#include <stdio.h>

#include <ioCC2530.h>

#define LED_ON  0
#define LED_OFF 1

//延时子函数
#define led1 P1_0
#define led2 P1_2
#define led3 P1_3
#define led4 P2_0



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
}

/*睡眠定时器中断初始化
-------------------------------------------------------*/
void sleepTimer_init(void)
{
  STIF=0;   //睡眠定时器中断标志清0
   
  STIE=1;   //开睡眠定时器中断
    
  //EA=1;     //开总中断...................................
}

/*设置睡眠定时器的定时间隔
-------------------------------------------------------*/
void setSleepTimer(unsigned int sec)
{
  unsigned long sleepTimer = 0;
  
  sleepTimer |= ST0;                      //取得目前的睡眠定时器的计数值
  sleepTimer |= (unsigned long)ST1 << 8;
  sleepTimer |= (unsigned long)ST2 << 16;
  
  sleepTimer += ((unsigned long)sec * (unsigned long)32768);   //加上所需要的定时时长
  
  ST2 = (unsigned char)(sleepTimer >> 16);   //设置睡眠定时器的比较值
  ST1 = (unsigned char)(sleepTimer >> 8); 
  ST0 = (unsigned char)sleepTimer;
}

/*选择电源模式
-------------------------------------------------------*/
void PowerMode(unsigned char mode)
{
  if(mode<4)
  {
    SLEEPCMD &= 0xfc;       //将SLEEP.MODE清0
    SLEEPCMD |= mode;       //选择电源模式
    PCON |= 0x01;        //启用此电源模式
  }
}


/*延时函数
-------------------------------------------------------*/
void Delay(unsigned int n)
{
  unsigned int i,j;
  for(i=0;i<n;i++)
    for(j=0;j<1000;j++);
}

/*UART0通信初始化
-------------------------------------------------------*/
void Uart0Init(unsigned char StopBits,unsigned char Parity)
{
   P0SEL |=  0x0C;                  //初始化UART0端口
   PERCFG&= ~0x01;                  //选择UART0为可选位置一
   U0CSR = 0xC0;                    //设置为UART模式,而且使能接受器
   U0GCR = 11;
   U0BAUD = 216;                    //设置UART0波特率为115200bps
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

void InitialT1test(void);               //初始化函数声明

void InitialT1test(void)
{
    //初始化计数器1
    T1CTL = 0x05;    
    IEN1|=0X02;  //定时器1中断使能
     //EA=1;   //开总中断   .....................
}

unsigned int counter=0;                //统计溢出次数
unsigned int sensorcounter=0;                //统计溢出次数
unsigned char sleepFlag=0;

 //void uWaveInit(void);
 int tmp;
 //int uWaveDistance(void);

 
#pragma vector = T1_VECTOR      //中断服务子程序
 __interrupt void T1_ISR(void)            
 {  
          if(sleepFlag==0)
          {
                ++counter;
                if(counter>150){
                    counter=0;  
                    sleepFlag=1;
                    printf("即将睡眠 ....\r\n");
                }
          }     
          ++sensorcounter;
          if(sensorcounter>60)
          {
              sensorcounter=0;
              //tmp=uWaveDistance();
              //printf("return:%d\r\n",tmp);
              //printf("距离 %u CM\r\n", uWaveDistance());    
              //printf("距离 xx CM\r\n");    
              dht11_update();   
          }
          T1IF=0;       
 }



void main(void)
{
    EA=0;                             //禁用中断
    
    xtal_init();         
    led_init(); 
    Uart0Init(0, 0);    
    Uart0SendString("sleep and wake test\r\n");
    sleepTimer_init();                //开睡眠定时器中断
    InitialT1test();
    EA=1;                             //启用中断
    
    sleepFlag=0;
    //uWaveInit();
    dht11_io_init();
    while(1)
    {
        if(sleepFlag)
        {
              setSleepTimer(3);      //设置睡眠定时器的定时间隔为1s
              led1 = LED_OFF;
              printf("睡眠....\r\n");
              PowerMode(1);          //设置电源模式为PM1
              Delay(10);
              
              led1 = LED_ON;         //1秒后PM1 ---> PM0
              printf("\r\n唤醒....\r\n");
        }
        //DelaymS(1000);
        //printf("距离 %u CM\r\n", uWaveDistance());    
        //printf("距离 xx CM\r\n");    
    }
}

/*睡眠定时器中断服务程序
-------------------------------------------------------*/
#pragma vector= ST_VECTOR
__interrupt void sleepTimer_IRQ(void)
{
   
    EA=0;     //关中断
    STIF=0;   //睡眠定时器中断标志清0
    sleepFlag=0;
    counter=0;  
    led2=!led2;
    //printf("即将唤醒....\r\n");
    EA=1;     //开中断
}