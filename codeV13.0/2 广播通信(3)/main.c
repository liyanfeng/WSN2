
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

//延时子函数
#define led1 P1_0
#define led2 P1_2
#define led3 P1_3
#define led4 P2_0

#define RF_CHANNEL            25      // 2.4 GHz RF channel


#define PAN_ID                0x2007
#define SEND_ADDR             0x2530
#define RECV_ADDR             0x2520

#define NODE_TYPE             1         //0:接收节点，！0：发送节点

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

 char pTxData[128] ={'l','i','y','a','n','f','e','n','g','v','5','8','7','a','b',0};
    uint8 ret;
 
//void dht11_update(char* buf);    
    
#pragma vector = T1_VECTOR      //中断服务子程序
 __interrupt void T1_ISR(void)            
 {  
   
    ++sensorcounter;
    if(sensorcounter>60)
    {
        sensorcounter=0;
        dht11_update();
        sprintf(pTxData,"shidu:%utmp%u℃\r\n", gdat1, gdat2);
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
              printf("即将睡眠 ....\r\n");
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
              printf("睡眠....\r\n");
              setSleepTimer(5);      //设置睡眠定时器的定时间隔为1s
              PowerMode(1);          //设置电源模式为PM1
              Delay(10);
              
              led1 = LED_ON;         //1秒后PM1 ---> PM0
              printf("\r\n唤醒....\r\n");
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
    printf("你知道串口是正常的.....\r\n");
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
    sleepTimer_init();                //开睡眠定时器中断
    InitialT1test();
    EA=1;                             //启用中断
    
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
  printf("接收数据\r\n");
  rfRecvData();   
#endif
}



/*睡眠定时器中断服务程序
-------------------------------------------------------*/
#if NODE_TYPE

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

#endif