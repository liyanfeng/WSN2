
#include <iocc2530.h>
#include "hal_mcu.h"
#include "hal_assert.h"
#include "hal_board.h"

#include "hal_rf.h"
#include "basic_rf.h"
#include <stdio.h>
#include <stdlib.h>

#define LED_ON  0
#define LED_OFF 1

//��ʱ�Ӻ���
#define led1 P1_0
#define led2 P1_2
#define led3 P1_3
#define led4 P2_0

#define RF_CHANNEL            25      // 2.4 GHz RF channel


#define PAN_ID                0x2007
#define SEND_ADDR             0x2530
#define RECV_ADDR             0x2520

#define NODE_TYPE             1         //0:���սڵ㣬��0�����ͽڵ�

static basicRfCfg_t basicRfConfig;
#if NODE_TYPE
#pragma optimize=none      
void DelaymS (unsigned int dly)                   
{
      unsigned int i;
      for ( ; dly>0; dly--)
      {
           for (i=0; i<1032; i++);        
      }
}
uint8 gdat1=0,gdat2=0;
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

 char pTxData[128] ={'l','i','y','a','n','f','e','n','g','v','5','8','7','a','b',0};
    uint8 ret;
 
//void dht11_update(char* buf);    
    
#pragma vector = T1_VECTOR      //�жϷ����ӳ���
 __interrupt void T1_ISR(void)            
 {  
   
    ++sensorcounter;
    if(sensorcounter>60)
    {
        sensorcounter=0;
        dht11_update();
        sprintf(pTxData,"shidu:%utmp%u��\r\n", gdat1, gdat2);
        printf("sensor data.....");
        printf(pTxData);
        printf("\r\n");
        ret = basicRfSendPacket(0xffff, pTxData, sizeof pTxData);       

        if (ret == SUCCESS) {
            printf("send msg ok\r\n");
         } else {
            printf("send msg error\r\n");
         }
    }
    if(sleepFlag==0)
    {
          ++counter;
          if(counter>300){
              counter=0;  
              sleepFlag=1;
              printf("����˯�� ....\r\n");
          }
    }     
    
    T1IF=0;       
 }
#endif

#if NODE_TYPE 
void rfSendData(void)
{
  
  //  uint8 recvCnt=0;
  //  uint8 ch=0;
    // Keep Receiver off when not needed to save power
    basicRfReceiveOff();
    
    // Main loop
    while (TRUE) {
      
      if(sleepFlag)
        {
              led1 = LED_OFF;
              printf("˯��....\r\n");
              setSleepTimer(5);      //����˯�߶�ʱ���Ķ�ʱ���Ϊ1s
              PowerMode(1);          //���õ�ԴģʽΪPM1
              Delay(10);
              
              led1 = LED_ON;         //1���PM1 ---> PM0
              printf("\r\n����....\r\n");
        }
      //recvCnt=0;
      //Uart_Send_String("input String must end with @\r\n");
       /*while(TRUE)
	{
            ch = Uart_Recv_char();
            if (ch == '@' || recvCnt >= 256) {
                pTxData[recvCnt] = 0;
                Uart_Send_String(pTxData);
                Uart_Send_String("\r\n");
                recvCnt = 0;
                printf("get string finish...\r\n");
                break;
            } else {
                pTxData[recvCnt++] = ch;
            }
	}*/
       
       
    }
}

#endif

void rfRecvData(void)
{
  uint8 pRxData[128];
  int rlen;
  
   basicRfReceiveOn();
   printf("recv msg start...\r\n");
    // Main loop
    while (TRUE) {
        while(!basicRfPacketIsReady());
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);
        if(rlen > 0) {
          printf("recv msg ok...\r\n");
          pRxData[rlen] = 0;
            
          printf((char *)pRxData);
          printf("\r\n");  
        }
    }
}
void dht11_io_init(void);
void main(void)
{
    halMcuInit();

    hal_led_init();
    
    hal_uart_init();
    printf("��֪��������������.....\r\n");
    if (FAILED == halRfInit()) {
        HAL_ASSERT(FALSE);
    }

    // Config basicRF
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
#ifdef SECURITY_CCM
    basicRfConfig.securityKey = key;
#endif

    
    // Initialize BasicRF
#if NODE_TYPE
    led_init(); 
    sleepTimer_init();                //��˯�߶�ʱ���ж�
    InitialT1test();
    EA=1;                             //�����ж�
    
    sleepFlag=0;
    //uWaveInit();
    
    
    basicRfConfig.myAddr = SEND_ADDR;
#else
    basicRfConfig.myAddr = RECV_ADDR; 
#endif
    
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
#if NODE_TYPE
    dht11_io_init();
  rfSendData();
#else
  printf("��������\r\n");
  rfRecvData();   
#endif
}



/*˯�߶�ʱ���жϷ������
-------------------------------------------------------*/
#if NODE_TYPE

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

#endif