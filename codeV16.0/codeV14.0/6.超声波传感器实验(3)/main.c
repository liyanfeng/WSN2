//ͷ�ļ�

#include <stdio.h>

#include <ioCC2530.h>

#define LED_ON  0
#define LED_OFF 1

//��ʱ�Ӻ���
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
}

/*˯�߶�ʱ���жϳ�ʼ��
-------------------------------------------------------*/
void sleepTimer_init(void)
{
  STIF=0;   //˯�߶�ʱ���жϱ�־��0
   
  STIE=1;   //��˯�߶�ʱ���ж�
    
  //EA=1;     //�����ж�...................................
}

/*����˯�߶�ʱ���Ķ�ʱ���
-------------------------------------------------------*/
void setSleepTimer(unsigned int sec)
{
  unsigned long sleepTimer = 0;
  
  sleepTimer |= ST0;                      //ȡ��Ŀǰ��˯�߶�ʱ���ļ���ֵ
  sleepTimer |= (unsigned long)ST1 << 8;
  sleepTimer |= (unsigned long)ST2 << 16;
  
  sleepTimer += ((unsigned long)sec * (unsigned long)32768);   //��������Ҫ�Ķ�ʱʱ��
  
  ST2 = (unsigned char)(sleepTimer >> 16);   //����˯�߶�ʱ���ıȽ�ֵ
  ST1 = (unsigned char)(sleepTimer >> 8); 
  ST0 = (unsigned char)sleepTimer;
}

/*ѡ���Դģʽ
-------------------------------------------------------*/
void PowerMode(unsigned char mode)
{
  if(mode<4)
  {
    SLEEPCMD &= 0xfc;       //��SLEEP.MODE��0
    SLEEPCMD |= mode;       //ѡ���Դģʽ
    PCON |= 0x01;        //���ô˵�Դģʽ
  }
}


/*��ʱ����
-------------------------------------------------------*/
void Delay(unsigned int n)
{
  unsigned int i,j;
  for(i=0;i<n;i++)
    for(j=0;j<1000;j++);
}

/*UART0ͨ�ų�ʼ��
-------------------------------------------------------*/
void Uart0Init(unsigned char StopBits,unsigned char Parity)
{
   P0SEL |=  0x0C;                  //��ʼ��UART0�˿�
   PERCFG&= ~0x01;                  //ѡ��UART0Ϊ��ѡλ��һ
   U0CSR = 0xC0;                    //����ΪUARTģʽ,����ʹ�ܽ�����
   U0GCR = 11;
   U0BAUD = 216;                    //����UART0������Ϊ115200bps
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

void InitialT1test(void);               //��ʼ����������

void InitialT1test(void)
{
    //��ʼ��������1
    T1CTL = 0x05;    
    IEN1|=0X02;  //��ʱ��1�ж�ʹ��
     //EA=1;   //�����ж�   .....................
}

unsigned int counter=0;                //ͳ���������
unsigned int sensorcounter=0;                //ͳ���������
unsigned char sleepFlag=0;

 //void uWaveInit(void);
 int tmp;
 //int uWaveDistance(void);

 
#pragma vector = T1_VECTOR      //�жϷ����ӳ���
 __interrupt void T1_ISR(void)            
 {  
          if(sleepFlag==0)
          {
                ++counter;
                if(counter>150){
                    counter=0;  
                    sleepFlag=1;
                    printf("����˯�� ....\r\n");
                }
          }     
          ++sensorcounter;
          if(sensorcounter>60)
          {
              sensorcounter=0;
              //tmp=uWaveDistance();
              //printf("return:%d\r\n",tmp);
              //printf("���� %u CM\r\n", uWaveDistance());    
              //printf("���� xx CM\r\n");    
              dht11_update();   
          }
          T1IF=0;       
 }



void main(void)
{
    EA=0;                             //�����ж�
    
    xtal_init();         
    led_init(); 
    Uart0Init(0, 0);    
    Uart0SendString("sleep and wake test\r\n");
    sleepTimer_init();                //��˯�߶�ʱ���ж�
    InitialT1test();
    EA=1;                             //�����ж�
    
    sleepFlag=0;
    //uWaveInit();
    dht11_io_init();
    while(1)
    {
        if(sleepFlag)
        {
              setSleepTimer(3);      //����˯�߶�ʱ���Ķ�ʱ���Ϊ1s
              led1 = LED_OFF;
              printf("˯��....\r\n");
              PowerMode(1);          //���õ�ԴģʽΪPM1
              Delay(10);
              
              led1 = LED_ON;         //1���PM1 ---> PM0
              printf("\r\n����....\r\n");
        }
        //DelaymS(1000);
        //printf("���� %u CM\r\n", uWaveDistance());    
        //printf("���� xx CM\r\n");    
    }
}

/*˯�߶�ʱ���жϷ������
-------------------------------------------------------*/
#pragma vector= ST_VECTOR
__interrupt void sleepTimer_IRQ(void)
{
   
    EA=0;     //���ж�
    STIF=0;   //˯�߶�ʱ���жϱ�־��0
    sleepFlag=0;
    counter=0;  
    led2=!led2;
    //printf("��������....\r\n");
    EA=1;     //���ж�
}