
#include <iocc2530.h>
#include "hal_mcu.h"
#include "hal_assert.h"
#include "hal_board.h"

#include "hal_rf.h"
#include "basic_rf.h"
#include <stdio.h>


#define RF_CHANNEL            13      // 2.4 GHz RF channel


#define PAN_ID                0x2007
#define SEND_ADDR             0x2530
#define RECV_ADDR             0x2520

#define NODE_TYPE             0          //0:接收节点，！0：发送节点

static basicRfCfg_t basicRfConfig;


void rfChannelScan(void)
{
    uint8 pTxData[] = {'H', 'e', 'l', 'l', 'o', ' ', 'c', 'c', '2', '5', '3', '0', '\r', '\n'};
    uint8 pRxData[32];
    int i,rlen=0;
    uint8 channel;
    uint8 ret;
    // Keep Receiver off when not needed to save power
    basicRfReceiveOn();
    while(1)
    {
        ret = basicRfSendPacket(RECV_ADDR, pTxData, sizeof pTxData);     
        if (basicRfPacketIsReady()) {
              rlen=basicRfReceive(pRxData, 32, NULL);
              if(rlen>0)
              {
                  pRxData[rlen]=0;
                  printf("get return data:%s\r\n",pRxData);
              }
              halMcuWaitMs(1);
        }
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
  uint8 pRxData[32];
  int rlen;
  uint8 ret;
  printf("recv msg test..\r\n");
   basicRfReceiveOn();
    while (TRUE) {
        while(!basicRfPacketIsReady());
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);
        
        if(rlen > 0) {
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
