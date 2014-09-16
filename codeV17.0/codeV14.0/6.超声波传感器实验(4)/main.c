//ͷ�ļ�
#include <stdio.h>
#include <ioCC2530.h>

#define uint   unsigned  int
#define uchar  unsigned  char

#define key1  P0_1
#define key2  P0_4

volatile uint counter=0;                   //ͳ���������
volatile uint clickcounter=0;              //ͳ���������
volatile uint flashcounter=0;              //ͳ���������
volatile uint myrate=60;                   //Ĭ�ϴ������������1��һ��
volatile uchar LEDFlag1=0;                 //��־�Ƿ�Ҫ��˸
volatile uchar LEDFlag2=0;                 //��־�Ƿ�Ҫ��˸

//��ʱ�Ӻ���
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

//32M�����ʼ��
void xtal_init(void)
{
    CLKCONCMD &= ~0x40;              //����
    while(!(SLEEPSTA & 0x40));      //�ȴ������ȶ�
    CLKCONCMD &= ~0x47;             //TICHSPD128��Ƶ��CLKSPD����Ƶ
    SLEEPCMD |= 0x04; 		 //�رղ��õ�RC����
}

//LED�Ƴ�ʼ��
void led_init(void)
{
  P1SEL  = 0x00;          //P1Ϊ��ͨ I/O ��
  P1DIR |= 0x0D;          //P1.0 P1.1 P1.2 P1.3 ���

  P2SEL  = 0x00;
  P2DIR |= 0x01;
  
  led1 = 0;
  led2 = 0;
  led3 = 0;
  led4 = 0;
  
  P0SEL = 0x00;           //������ʼ��
  P0DIR = 0x00;
}


/*UART0ͨ�ų�ʼ��
-------------------------------------------------------*/
void Uart0Init(unsigned char StopBits,unsigned char Parity)
{
   P0SEL |=  0x0C;                  //��ʼ��UART0�˿�
   PERCFG&= ~0x01;                  //ѡ��UART0Ϊ��ѡλ��һ
   U0CSR = 0xC0;                    //����ΪUARTģʽ,����ʹ�ܽ�����
   U0GCR = 8;
   U0BAUD = 59;                    //����UART0������Ϊ115200bps
   U0UCR |= StopBits|Parity;        //����ֹͣλ����żУ��
}


/*UART0��������
-------------------------------------------------------*/
void  Uart0Send(unsigned char data)
{
  while(U0CSR&0x01);    //�ȴ�UART����ʱ��������
  U0DBUF = data;
}

/*UART0�����ַ���
-------------------------------------------------------*/
void Uart0SendString(unsigned char *s)
{
  while(*s != 0)         //���η����ַ���s�е�ÿ���ַ�
    Uart0Send(*s++);
}


__near_func int putchar(int ch)
{
    U0DBUF = ch;
    while(UTX0IF == 0);
    UTX0IF = 0;
    return ch;
}

/*UART0��������
-------------------------------------------------------*/
unsigned char Uart0Receive(void)
{
  unsigned char data;
  while(!(U0CSR&0x04)); //��ѯ�Ƿ��յ����ݣ���������ȴ�
  data=U0DBUF;          //��ȡ���յ�������
  return data;          
}

void InitialT1test(void)
{
    //��ʼ��������1
    T1CTL = 0x05;    
    IEN1|=0X02;  //��ʱ��1�ж�ʹ��
     EA=1;   //�����ж�
}

int uWaveDistance(void);

#pragma vector = T1_VECTOR      //�жϷ����ӳ���
 __interrupt void T1_ISR(void)            
 {       
          
         
      ++counter;
      ++clickcounter;

      clickcounter%=100;
      if(counter>myrate){
          counter=0;  
          //printf("���� %u CM\r\n", uWaveDistance());    
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

//������
void main(void)    
{
      EA=0;                   //�����ж�
      xtal_init();
      led_init();  
      Uart0Init(0, 0);
      Uart0SendString("  ���������ʵ��\r\n");
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

