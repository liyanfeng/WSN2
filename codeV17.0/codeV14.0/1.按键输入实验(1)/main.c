//ͷ�ļ�
#include <stdio.h>
#include <ioCC2530.h>

#define uint   unsigned  int
#define uchar  unsigned  char

#define led1 P1_0
#define led2 P1_2


#define key1  P0_1
#define key2  P0_4

volatile uint counter=0;                //ͳ���������
volatile uint clickcounter=0;                //ͳ���������
volatile uchar LEDFlag1=0;                //��־�Ƿ�Ҫ��˸
volatile uchar LEDFlag2=0;                //��־�Ƿ�Ҫ��˸
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


//32M�����ʼ��
void xtal_init(void)
{
    CLKCONCMD &= ~0x40;              //����
    while(!(SLEEPSTA & 0x40));      //�ȴ������ȶ�
    CLKCONCMD &= ~0x47;             //TICHSPD128��Ƶ��CLKSPD����Ƶ
    SLEEPCMD |= 0x04; 		 //�رղ��õ�RC����
}

//LED�Ƴ�ʼ��
void io_init(void)
{
  P1SEL  = 0x00;          //led��ʼ��
  P1DIR  = 0x05;
  led1 = 1;
  led2 = 1;
  
  P0SEL = 0x00;           //������ʼ��
  P0DIR = 0x00;
}


void InitialT1test(void)
{
    //��ʼ��������1
    T1CTL = 0x05;    
    IEN1|=0X02;  //��ʱ��1�ж�ʹ��
     EA=1;   //�����ж�
}

#pragma vector = T1_VECTOR      //�жϷ����ӳ���
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

//������
void main(void)    
{
   
    EA=0;//�����ж�
     
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


