
#include <iocc2530.h>
#include "hal_mcu.h"
#include "hal_assert.h"
#include "hal_board.h"

#include "hal_rf.h"
#include "basic_rf.h"
#include <stdio.h>
#include <stdlib.h>

unsigned char gdat1=0,gdat2=0;

#define RF_CHANNEL            13      // 2.4 GHz RF channel


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

void rfChannelScan(void)
{
    uint8 pTxData[64] = {'H', 'e', 'l', 'l', 'o', ' ', 'c', 'c', '2', '5', '3', '0', '\r', '\n'};
    uint8 pRxData[64];
    int i;
    int tmp=0;
    int rlen=0;
    uint8 channel;
    
    basicRfReceiveOn();
   
    while(1)
    {
        for (channel=11; channel<=26; channel++) {
            printf("scan channel %d ... \r\n", channel);
            halRfSetChannel(channel);
            basicRfSendPacket(RECV_ADDR, pTxData, sizeof pTxData);     
            for (i=0; i<1000; i++) {
                if (basicRfPacketIsReady()) {
                    rlen=basicRfReceive(pRxData, 32, NULL);
                    pRxData[rlen]=0;
                    printf("recvive data %s,  channel:%d\r\n",pRxData,channel);
                    break;
                }
                halMcuWaitMs(1);
            }
            if (i < 1000){
                printf("In Use\r\n");
                channel=28;
            }
        }  
    }
    
}
uint16 basicRfReceiveAddress();
void rfRecvData(void)
{
    uint8 pRxData[64],ret;
    int rlen;
    int channel=0;
    basicRfReceiveOn();
    printf("recv msg test\n\r");
    // Main loop
    //srand((int)time(0));
    while (TRUE) {
      //   11-26
      //   0-1
      //   
        channel=(int)rand()/(RAND_MAX+1.0)*15+11;
        printf("channel:%d\r\n",channel);
        //halRfSetChannel(channel);
        basicRfConfig.channel = channel;
        printf("channel:%d\r\n",channel);
        
        if(basicRfInit(&basicRfConfig)==FAILED) {
          HAL_ASSERT(FALSE);
        }
        basicRfReceiveOn();
        while(!basicRfPacketIsReady());
        rlen = basicRfReceive(pRxData, sizeof pRxData, NULL);
        pRxData[rlen]=0;
        printf("recv msg:%s  channel:%d\r\n",pRxData,channel); 
        if(rlen > 0) {
            //sprintf(pRxData,"i am recvive data ok!\r\n");
            gdat1=0;
            gdat2=0;
            dht11_update();
            sprintf(pRxData,"shidu:%utmp%u℃\r\n", gdat1, gdat2);
            ret=basicRfSendPacket(basicRfReceiveAddress(), pRxData, rlen);
            if(ret==SUCCESS)
            {
              printf("send data ok channel:%d\r\n",channel);
            }
            else
            {
              printf("send data error channel:%d\r\n",channel);
            }
        }
    }
}

void dht11_io_init(void);

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
  dht11_io_init();
  rfRecvData();   
#endif
  while (TRUE);
}
