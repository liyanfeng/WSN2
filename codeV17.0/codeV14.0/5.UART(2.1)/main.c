//cd wxl 2009 ����0������
#include <ioCC2530.h>
#include <string.h>

#define uint unsigned int
#define uchar unsigned char

char recvBuf[256];
int recvCnt = 0; 

//������ƵƵĶ˿�
#define led1 P1_0
#define led2 P1_1

//��������
void Delay(uint);
void initUARTtest(void);
void Uart_Send_String(char *Data);

void Uart_Send_char (char ch);
int Uart_Recv_char (void);


char Txdata[30]=" ChenDu WuXianLong TongXun ";


void xtal_init(void)
{
    CLKCONCMD &= ~0x40;              //����
    while(!(SLEEPSTA & 0x40));      //�ȴ������ȶ�
    CLKCONCMD &= ~0x47;             //TICHSPD128��Ƶ��CLKSPD����Ƶ
    SLEEPCMD |= 0x04; 		 //�رղ��õ�RC����
}

/****************************************************************
*�������� ����ʱ						
*��ڲ��� ��������ʱ									
*�� �� ֵ ����												
*˵    �� ��													
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
*�������� ����ʼ������1										
*��ڲ��� ����												
*�� �� ֵ ����							
*˵    �� ��57600-8-n-1						
****************************************************************/
void initUARTtest(void)
{
    PERCFG = 0x00;				//λ��1 P0��
    P0SEL = 0x3c;				//P0��������
    P2DIR &= ~0XC0;                             //P0������Ϊ����0

    U0CSR |= 0x80;				//UART��ʽ
    U0CSR |= 0X40;				//�������
    
    U0GCR |= 8;				//baud_e
    U0BAUD |= 59;				//��������Ϊ115200
    UTX0IF = 0;
}

/****************************************************************
*�������� �����ڷ����ַ�������					
*��ڲ��� : data:����									
*			len :���ݳ���							
*�� �� ֵ ����											
*˵    �� ��				
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
*�������� ��������								
*��ڲ��� ����							
*�� �� ֵ ����						
*˵    �� ����							
****************************************************************/
void main(void)
{	 
	int ch;
        //P1 out
	P1DIR = 0x03; 		 		//��led1��led2����Ϊ���
	led1 = 0;
	led2 = 1;				//��LED

        xtal_init();
        
	initUARTtest();
	Uart_Send_String("Please Input string end with '@'\r\n");	        //wu xian long tong xun
 
	while(1)
	{
          //  UartTX_Send_String(T); //���ڷ�������
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
