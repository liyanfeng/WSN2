//头文件
#include <stdio.h>
#include <ioCC2530.h>

#define uint   unsigned  int
#define uchar  unsigned  char

#define led1 P1_0
#define led2 P1_2


#define key1  P0_1
#define key2  P0_4

volatile uint counter=0;                //统计溢出次数
volatile uint clickcounter=0;                //统计溢出次数
volatile uchar LEDFlag1=0;                //标志是否要闪烁
volatile uchar LEDFlag2=0;                //标志是否要闪烁
volatile uchar ledstate=0;

void Delay(unsigned n)   
{
  unsigned tt;
  for(tt = 0;tt<n;tt++);
  for(tt = 0;tt<n;tt++);
  for(tt = 0;tt<n;tt++);
  for(tt = 0;tt<n;tt++);
  for(tt = 0;tt<n;tt++);
    for(tt = 0;tt<n;tt++);
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
void io_init(void)
{
  P1SEL  = 0x00;          //led初始化
  P1DIR  = 0x05;
  led1 = 1;
  led2 = 1;
  
  P0SEL = 0x00;           //按键初始化
  P0DIR = 0x00;
}


void InitialT1test(void)
{
    //初始化计数器1
    T1CTL = 0x05;    
    IEN1|=0X02;  //定时器1中断使能
     EA=1;   //开总中断
}

#pragma vector = T1_VECTOR      //中断服务子程序
 __interrupt void T1_ISR(void)            
 {       
          
         
      ++counter;
      ++clickcounter;
      clickcounter%=100;
      if(counter>10){
          counter=0;  
          
          if(LEDFlag1)
          {
              led1 =  !led1;
          }
          else{
              led1 =  1;
          }
          if(LEDFlag2)
          {
              led2 =  !led2;
          }
          else
          {
              led2 = 1;
          }

      }
      T1IF=0;       
 }

void stateMechine(int state)
{
      
      switch(state)
      {
          case 0:
            LEDFlag1=0;
            LEDFlag2=0;
            break;
          case 1:
            LEDFlag1=1;
            LEDFlag2=0;
            break;
          case 2:
            LEDFlag1=0;
            LEDFlag2=1;
            break;
          case 3:
            LEDFlag1=1;
            LEDFlag2=1;
            break;
            
      }
      led1=1;
      led2=1;
      counter=0;
}

//主函数
void main(void)    
{
   
    EA=0;//禁用中断
     
    xtal_init();
    
    io_init();
    InitialT1test();
    
     while(1) {
       if (key1==0) {
         if(clickcounter<10)
         {
              continue;
         }
         ++ledstate;
         ledstate%=4;
         stateMechine(ledstate);
         clickcounter=0;
       } 
       if (key2==0) {
         if(clickcounter<10)
         {
              continue;
         }
         ledstate+=4;
         --ledstate;
         ledstate%=4;
         stateMechine(ledstate);
         clickcounter=0;
       } 
      // Delay(10);
    }
}


