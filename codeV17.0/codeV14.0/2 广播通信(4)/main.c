
#include <iocc2530.h>
#include "hal_mcu.h"
#include "hal_assert.h"
#include "hal_board.h"

#include "hal_rf.h"
#include "basic_rf.h"
#include <stdio.h>


#define RF_CHANNEL            22      // 2.4 GHz RF channel


#define PAN_ID                0x2007
#define SEND_ADDR             0x2530
#define RECV_ADDR             0x2520

#define NODE_TYPE             1         //0:接收节点，！0：发送节点

static basicRfCfg_t basicRfConfig;

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

#if NODE_TYPE

volatile unsigned int counter=0;                   //统计溢出次数
volatile unsigned int ledflashcounter=0;                   //统计溢出次数
volatile unsigned int myrate=60;                   //默认传感器数据输出1秒一次

void InitialT1test(void)
{
    //初始化计数器1
    T1CTL = 0x05;    
    IEN1|=0X02;  //定时器1中断使能
     EA=1;   //开总中断
}

char pTsxData[30] ={'l','i','y','a','n','f','e','n','g','v','5','8','7','a','b',0};

unsigned char ret;

void dht11_update(void);

//定时器1
#pragma vector = T1_VECTOR      //中断服务子程序
 __interrupt void T1_ISR(void)            
 {       
      ++counter;
      if(counter>myrate){   //改变myrate的值改变频率
          counter=0;  
          dht11_update(); //获取传感器数据
          sprintf(pTsxData,"shidu:%utmp%u℃\r\n", gdat1, gdat2);
          ret = basicRfSendPacket(0xffff, pTsxData, sizeof pTsxData);  //   无线发送数据
          ledflashcounter=8;
          hal_led_on(1);//led1闪烁一下
      }
      //控制led1的闪烁时间
      if(ledflashcounter>0)
    {
          --ledflashcounter;
          if(ledflashcounter==0)
          {
               hal_led_off(1);
          }
      }
      T1IF=0;       //请中断标志位
 }


void rfSendData(void)
{
  char pTxData[20]={0};
    unsigned int recvCnt=0;
    unsigned int ch=0,i=0;
    unsigned int tmp=0;
    // Keep Receiver off when not needed to save power
    
    
    // Main loop
    while (TRUE) {
        recvCnt=0;
        Uart_Send_String("input num/+/- must end with @\r\n");
        tmp=0;
        
        //输入字符串，把字符串写到ptxdata中   1@   5@    +@   -@
        while(TRUE)
        {
            ch = Uart_Recv_char();
            if (ch == '@' || recvCnt >= 256) {
                  pTxData[recvCnt] = 0;
                  Uart_Send_String(pTxData);
                  Uart_Send_String("\r\n");
                 // recvCnt = 0;
                  printf("get string finish...\r\n");
                  break;
            } else {
                  pTxData[recvCnt++] = ch;
            }
        }
        
        
        
        //recvCnt= sizeof pTsxData;
        printf("input num:%s,length:%d",pTxData,recvCnt);
        if(pTxData[0]=='+')
        {
              myrate-=2;
              printf("myrate:%d,f:%d\r\n",myrate,tmp);
              continue;
        }
        if(pTxData[0]=='-')
        {
              myrate+=2;//myrate  相当于周期   60为1秒
              printf("myrate:%d,f:%d\r\n",myrate,tmp);
              continue;     
        }
        
        
        //把字符串解析成为数字--》tmp
        for(i=0;i<recvCnt;i++)
        {
            if(pTxData[i]>='0'&&pTxData[i]<='9')
            {
                tmp*=10;
                tmp+=(pTxData[i]-'0');
            }
            else
            {
                printf("input num error\r\n");
                i=256;
            }
        }
        if(tmp!=0)
        {
            myrate=60*tmp;
            counter=0;
        }
        printf("myrate:%d,f:%d\r\n",myrate,tmp);
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
        while(!basicRfPacketIsReady());//等待数据
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);//接收数据
        if(rlen > 0) {
          printf("recv msg ok...\r\n");
          pRxData[rlen] = 0;
            
          printf((char *)pRxData);//吧传感器数据显示
            
        }
    }
}

void dht11_io_init(void);

void main(void)
{
    halMcuInit();//cpu初始化

    hal_led_init();
    
    hal_uart_init();//串口初始化
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
    basicRfConfig.myAddr = SEND_ADDR;
#else
    basicRfConfig.myAddr = RECV_ADDR; 
#endif
    
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
#if NODE_TYPE
    dht11_io_init();//温湿度传感器初始化
    InitialT1test();//T1定时器初始化
    basicRfReceiveOff();//关闭zigbee数据接收
    rfSendData(); //发送数据主函数
#else
    printf("接收数据\r\n");
    rfRecvData();   //接受数据主函数
#endif
}
