#include <ioCC2530.h>

#define   PIN_EN_CFG      (P1SEL &= ~0x08, P1DIR |= 0x08)
#define   PIN_ECHO_CFG    (P2SEL &= ~0x01, P2DIR &= ~0x01)

#define   WAVE_EN_PIN     P1_3
#define   WAVE_INPUT_PIN  P2_0

static char clkidx;
static unsigned int clk;
static unsigned clks[] = {32000, 16000, 8000, 4000, 2000, 1000, 500, 250};

#pragma optimize=none
static void  Delay_10us(void)
{
        char i = 10;
        i--;
        i--;
        i--;
        i--;
        i--;
        i--;
}

void uWaveInit(void)
{
    PIN_EN_CFG;
    PIN_ECHO_CFG;
    
    WAVE_EN_PIN = 0;
    
    clkidx = (CLKCONCMD>>3)&0x07;
    
    clk = (clks[clkidx]/128);
}

static void uWaveStart(void)
{
  WAVE_EN_PIN = 1;
  Delay_10us();
  Delay_10us();
  WAVE_EN_PIN = 0;
}
#pragma optimize=none
int uWaveDistance(void)
{
  unsigned int i = 0;
  float cnt = 0;
  int d;
  
  
  T1CNTL = 0;
  uWaveStart();
  while ((0 == WAVE_INPUT_PIN) && ++i);
  if (i == 0) return -1;
  T1CTL = 0x0D; //128div
  i = 0;
  while (WAVE_INPUT_PIN && ++i);
  T1CTL = 0x00;
  if (i == 0) return -1;
  
  cnt = (T1CNTH<<8) | (T1CNTL);
  d = (cnt) / clk * 17;
  
  return d;
}

