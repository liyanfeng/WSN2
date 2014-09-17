
#include <iocc2530.h>
#include "hal_mcu.h"
#include "hal_assert.h"
#include "hal_board.h"

#include "hal_rf.h"
#include "basic_rf.h"
#include <stdio.h>


#define RF_CHANNEL            25      // 2.4 GHz RF channel


#define PAN_ID                0x2007
#define SEND_ADDR             0x2530
#define RECV_ADDR             0x2520

#define NODE_TYPE             0         //0:接收节点，！0：发送节点

static basicRfCfg_t basicRfConfig;


void rfSendData(void)
{
  uint8 pTxData[20] ={'l','i','y','a','n','f','e','n','g','v','5','8','7','a','b',0};
    uint8 ret;
    uint8 recvCnt=0;
    uint8 ch=0;
    // Keep Receiver off when not needed to save power
    basicRfReceiveOff();
    
    // Main loop
    while (TRUE) {
      recvCnt=0;
      
       ret = basicRfSendPacket(0xffff, pTxData, sizeof pTxData); //广播地址      
       if (ret == SUCCESS) {
          printf("send msg ok\r\n");
          hal_led_on(1);
          halMcuWaitMs(100);
          hal_led_off(1);
          halMcuWaitMs(900); 
       } else {
          printf("send msg error\r\n");
          hal_led_on(1);
          halMcuWaitMs(1000);
          hal_led_off(1);
       }
       
    }
}

void rfRecvData(void)
{
  uint8 pRxData[128];
  int rlen;
  
   basicRfReceiveOn();
   printf("recv msg start...\r\n");
    // Main loop
    while (TRUE) {
        while(!basicRfPacketIsReady());  //判断缓冲区里是否有数据
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);
        if(rlen > 0) {
          printf("recv msg ok...\r\n");
          pRxData[rlen] = 0; //字符串结束符
            
          printf((char *)pRxData); //
            
        }
    }
}

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
    basicRfConfig.myAddr = SEND_ADDR;
#else
    basicRfConfig.myAddr = RECV_ADDR; 
#endif
    
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
#if NODE_TYPE
  rfSendData();
#else
  printf("接收数据\r\n");
  rfRecvData();   
#endif
}
