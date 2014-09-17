//cd wxl 2009 串口0发数据
#include <ioCC2530.h>
#include <string.h>

#define uint unsigned int
#define uchar unsigned char

char recvBuf[256];
int recvCnt = 0; 

//定义控制灯的端口
#define led1 P1_0
#define led2 P1_1

//函数声明
void Delay(uint);
void initUARTtest(void);
void Uart_Send_String(char *Data);

void Uart_Send_char (char ch);
int Uart_Recv_char (void);


char Txdata[30]=" ChenDu WuXianLong TongXun ";


void xtal_init(void)
{
    CLKCONCMD &= ~0x40;              //晶振
    while(!(SLEEPSTA & 0x40));      //等待晶振稳定
    CLKCONCMD &= ~0x47;             //TICHSPD128分频，CLKSPD不分频
    SLEEPCMD |= 0x04; 		 //关闭不用的RC振荡器
}

/****************************************************************
*函数功能 ：延时						
*入口参数 ：定性延时									
*返 回 值 ：无												
*说    明 ：													
****************************************************************/
void Delay(uint n)
{
	uint i;
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
	for(i=0;i<n;i++);
}

/****************************************************************
*函数功能 ：初始化串口1										
*入口参数 ：无												
*返 回 值 ：无							
*说    明 ：57600-8-n-1						
****************************************************************/
void initUARTtest(void)
{
    PERCFG = 0x00;				//位置1 P0口
    P0SEL = 0x3c;				//P0用作串口
    P2DIR &= ~0XC0;                             //P0优先作为串口0

    U0CSR |= 0x80;				//UART方式
    U0CSR |= 0X40;				//允许接收
    
    U0GCR |= 8;				//baud_e
    U0BAUD |= 59;				//波特率设为115200
    UTX0IF = 0;
}

/****************************************************************
*函数功能 ：串口发送字符串函数					
*入口参数 : data:数据									
*			len :数据长度							
*返 回 值 ：无											
*说    明 ：				
****************************************************************/
void Uart_Send_String(char *Data)
{

  
  while (*Data != '\0')
  {
    Uart_Send_char(*Data++);
  }
}

void Uart_Send_char (char ch)
{
     U0DBUF = ch;
    while(UTX0IF == 0);
    UTX0IF = 0;
}

int Uart_Recv_char (void)
{
    int ch;
    
    while (URX0IF == 0);
    ch = U0DBUF;
    URX0IF = 0;
    return ch;
}

/****************************************************************
*函数功能 ：主函数								
*入口参数 ：无							
*返 回 值 ：无						
*说    明 ：无							
****************************************************************/
void main(void)
{	 
	int ch;
        //P1 out
	P1DIR = 0x03; 		 		//把led1和led2设置为输出
	led1 = 0;
	led2 = 1;				//关LED

        xtal_init();
        
	initUARTtest();
	Uart_Send_String("Please Input string end with '@'\r\n");	        //wu xian long tong xun
 
	while(1)
	{
          //  UartTX_Send_String(T); //串口发送数据
          ch = Uart_Recv_char();
          if (ch == '@' || recvCnt >= 256) {
            recvBuf[recvCnt] = 0;
            Uart_Send_String(recvBuf);
            Uart_Send_String("\r\n");
            recvCnt = 0;
          } else {
            recvBuf[recvCnt++] = ch;
          }
	}
}
