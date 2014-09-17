
#include <iocc2530.h>
#include "hal_mcu.h"
#include "hal_assert.h"
#include "hal_board.h"

#include "hal_rf.h"
#include "basic_rf.h"
#include <stdio.h>


#define RF_CHANNEL            23      // 2.4 GHz RF channel


#define PAN_ID                0x2007
#define SEND_ADDR             0x2530
#define RECV_ADDR             0x2520

#define NODE_TYPE             0          //0:接收节点，！0：发送节点

static basicRfCfg_t basicRfConfig;

void rfChannelScan(void)
{
    uint8 pTxData[64];
    uint8 pRxData[64];
    int rlen=0;
    unsigned long i=0;
    uint8 channel;
    uint8 ret;
    char ch;
    int recvCnt;
    // Keep Receiver off when not needed to save power
    basicRfReceiveOn();
    printf("发送端\r\n");
    while(1)
    {
        recvCnt=0;
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
        
        ret = basicRfSendPacket(RECV_ADDR, pTxData, sizeof pTxData);     
        ///for(i=0;i<2000;i++)
        //{
          i=0;
         while(!basicRfPacketIsReady())
         {
             i++;
             if(i>=12000)
             {
                  break;
             }
         }
         if(i<12000)
         {
             rlen=basicRfReceive(pRxData, 64, NULL); //pRxData=接受到的数据
             if(rlen>0)
             {
                 pRxData[rlen]=0;
                 printf("get return data:%s  i=%d\r\n",pRxData,i);
          
             }
             halMcuWaitMs(1);     
         }
         else
         {
              printf("get return msg error\r\n");
         }
       
        if (ret == SUCCESS) 
        {
            printf("send msg ok\r\n");
            hal_led_on(1);
            halMcuWaitMs(100);
            hal_led_off(1);
            halMcuWaitMs(900); 
       } 
        else 
        {
            printf("send msg error\r\n");
            hal_led_on(1);
            halMcuWaitMs(1000);
            hal_led_off(1);
       }
       
    }
}

void rfRecvData(void)
{
  uint8 pRxData[65];
  int rlen;
  uint8 ret;
  printf("recv msg test..\r\n");
   basicRfReceiveOn();
   printf("接收端\r\n");
    while (TRUE) {
        while(!basicRfPacketIsReady());
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);
        
        if(rlen > 0) {
            pRxData[rlen]='@';
            rlen++;
            pRxData[rlen]=0;
            printf("recv msg ok data:%s\r\n",pRxData);
            ret=basicRfSendPacket(basicRfReceiveAddress(), pRxData, rlen);
            if(ret==SUCCESS)
            {
                printf("send msg ok!\r\n");
            }
            else
            {
                printf("send msg error!\r\n");
            }
        }
    }
}

void main(void)
{
    halMcuInit();

    hal_led_init();
    
    hal_uart_init();
    printf("现在你终于知道了串口是正常的了吧!\r\n");
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
  rfChannelScan();
#else
  rfRecvData();   
#endif
  while (TRUE);
}
