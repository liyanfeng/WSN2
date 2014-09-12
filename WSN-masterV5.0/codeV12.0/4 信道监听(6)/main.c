
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

#define NODE_TYPE             0         //0:接收节点，！0：发送节点

static basicRfCfg_t basicRfConfig;


void rfChannelScan(void)
{
    uint8 pTxData[] = {'H', 'e', 'l', 'l', 'o', ' ', 'c', 'c', '2', '5', '3', '0', '\r', '\n'};
    uint8 pRxData[32];
    int i;
    uint8 channel;
    
    // Keep Receiver off when not needed to save power
    basicRfReceiveOn();
    
    for (channel=11; channel<=26; channel++) {
        printf("scan channel %d ... ", channel);
        halRfSetChannel(channel);
        basicRfSendPacket(RECV_ADDR, pTxData, sizeof pTxData);     
        for (i=0; i<1000; i++) {
          if (basicRfPacketIsReady()) {
            basicRfReceive(pRxData, 32, NULL);
              break;
          }
          halMcuWaitMs(1);
        }
        if (i >= 1000) {
          printf("Not Use\r\n");
        } else {
          printf("In Use\r\n");
        }
    }
}

void rfRecvData(void)
{
  uint8 pRxData[128];
  int rlen;
  
   basicRfReceiveOn();
   printf("recv msg test\n\r");
    // Main loop
    while (TRUE) {
        while(!basicRfPacketIsReady());
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);
        printf("recv msg:%s\r\n",pRxData); 
        if(rlen > 0) {
          
            ret=basicRfSendPacket(basicRfReceiveAddress(), pRxData, rlen);
            if(ret=SUCESS)
            {
                printf("send data ok\r\n");
            }
            else
            {
                printf("send data error\r\n");
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
