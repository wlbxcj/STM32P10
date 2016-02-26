/// @file   UARTCOM.c
/// This is for module @ref UART_COM
/// @author Travis Tu
/// @date   2006-Sep-1st

#include "TaxBox.h"
#include "UARTCom.h"
#include "KF701DH.h"
/*************************************UART DRIVER******************************/

#define RX_BUFFER_SIZE   512

static volatile unsigned char  uart_rx_buffer[2][RX_BUFFER_SIZE];
static volatile unsigned int   uart_rx_in[1]={0},uart_rx_out[1]={0};
static int uartOpenFlag[1]={0};

static unsigned char buff_flow_flag[1]={0};//0-no flow 1-flow


unsigned char  PortOpen(unsigned char port,unsigned long baud_rate,unsigned char data_bits,unsigned char parity,unsigned char stop_bits)
{
  
        GPIO_InitTypeDef GPIO_InitStructure;
        USART_InitTypeDef USART_InitStructure;

        if(port>MAX_PORT_NUM) return ERR_PORT;
        //Configure the UART
        USART_InitStructure.USART_BaudRate = baud_rate;
        if(data_bits!=8)
          USART_InitStructure.USART_WordLength = USART_WordLength_9b;
        else
          USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        
        if(parity=='O'||parity=='o')
          USART_InitStructure.USART_Parity = USART_Parity_Odd ;
        else if(parity=='E'||parity=='e')
          USART_InitStructure.USART_Parity = USART_Parity_Even ;
        else
          USART_InitStructure.USART_Parity = USART_Parity_No ;
        
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
        USART_InitStructure.USART_Clock = USART_Clock_Disable;
        USART_InitStructure.USART_CPOL = USART_CPOL_Low;
        USART_InitStructure.USART_CPHA = USART_CPHA_2Edge;
        USART_InitStructure.USART_LastBit = USART_LastBit_Disable;
	switch(port)
	{
            case COM3:
              //GPIO configuration
              
          
              /* Configure USARTx_Tx as alternate function push-pull */
              GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
              GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
              GPIO_Init(GPIOB, &GPIO_InitStructure);
          
              /* Configure USARTx_Rx as input floating */
              GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
              GPIO_Init(GPIOB, &GPIO_InitStructure);
              /* Configure the USART3 */
              USART_Init(USART1, &USART_InitStructure);
              /* Disable the USART Transmoit interrupt*/
              USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
              /* Enable the USART Receive interrupt: this interrupt is generated when the USART3 receive data register is not empty */
              USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
              /* Enable USART3 */
              USART_Cmd(USART1, ENABLE); 

              
	    break;
          
            case COM2:
              //GPIO configuration
              
          
              /* Configure USARTx_Tx as alternate function push-pull */
              GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
              GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
              GPIO_Init(GPIOA, &GPIO_InitStructure);
          
              /* Configure USARTx_Rx as input floating */
              GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
              GPIO_Init(GPIOA, &GPIO_InitStructure);
              /* Configure the USART3 */
              USART_Init(USART2, &USART_InitStructure);
              /* Disable the USART Transmoit interrupt*/
              USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
              /* Enable the USART Receive interrupt: this interrupt is generated when the USART3 receive data register is not empty */
              USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
              /* Enable USART3 */
              USART_Cmd(USART2, ENABLE); 

	    break;
            
            case COM1:
              //GPIO configuration
              
          
              /* Configure USARTx_Tx as alternate function push-pull */
              GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
              GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
              GPIO_Init(GPIOA, &GPIO_InitStructure);
          
              /* Configure USARTx_Rx as input floating */
              GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
              GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
              GPIO_Init(GPIOA, &GPIO_InitStructure);
              /* Configure the USART3 */
              USART_Init(USART1, &USART_InitStructure);
              /* Disable the USART Transmoit interrupt*/
              USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
              /* Enable the USART Receive interrupt: this interrupt is generated when the USART3 receive data register is not empty */
              USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
              /* Enable USART3 */
              USART_Cmd(USART1, ENABLE); 

	    break;
	    default:
	    break;
	}
        uartOpenFlag[port]=1;
        return UART_OK;
}


	
/*******************************************************************************
 * 复位指定的串口, 清控接收/发送缓冲
 * 参数：
 *     port     :  串口索引号
 */
unsigned char PortReset(unsigned char port)
{
    if(port>MAX_PORT_NUM) return ERR_PORT;
    
    switch(port)
    {
        case COM3:
          USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
          USART_ClearITPendingBit(USART3, USART_IT_RXNE);
          uart_rx_out[port]=0;
          uart_rx_in[port]=0;	
          buff_flow_flag[port]=0;
          USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
        break;
      case COM2:
          USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
          USART_ClearITPendingBit(USART2, USART_IT_RXNE);
          uart_rx_out[port]=0;
          uart_rx_in[port]=0;	
          buff_flow_flag[port]=0;
          USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
        break;

      case COM1:
          USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
          USART_ClearITPendingBit(USART1, USART_IT_RXNE);
          uart_rx_out[port]=0;
          uart_rx_in[port]=0;	
          buff_flow_flag[port]=0;
          USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
        break;
        default:
        break;
    }
    
    return UART_OK;
}

/*******************************************************************************
 * 从指定的串口在指定的时间内，接收串口1字节的数据
 * 参数：
 *     port     :  串口索引号
 *     buffer   :  接收1个数据字节
 *     time_out :  指定时间，以10毫秒计算
 * 返回：
 *     0 - 成功
 *     1 - 失败
 */

unsigned char PortRecv(unsigned char port,  unsigned char* bytes, unsigned int time_out)
{
	unsigned long cnt;

	if(port>MAX_PORT_NUM) return ERR_PORT;//skx 090223
	
	if(!uartOpenFlag[port]) return ERR_NO_OPEN;
	
	if(time_out == 0){ 
            if(uart_rx_in[port] == uart_rx_out[port]) 
            {
              if( !buff_flow_flag[port] )
                return ERR_RX_TIMEOUT;
            }
            *bytes = uart_rx_buffer[port][uart_rx_out[port]++];
            if(uart_rx_out[port] == RX_BUFFER_SIZE)
            {
              buff_flow_flag[port]=0;//clear flow flag
              uart_rx_out[port] = 0;
            }
            return 0;
	}
	else{
		TimerSet(3, time_out);
		while(uart_rx_in[port] == uart_rx_out[port] && ( !buff_flow_flag[port] ) ){
			if( !TimerCheck(3) ) return ERR_RX_TIMEOUT;
		}
		*bytes = uart_rx_buffer[port][uart_rx_out[port]++];
		if(uart_rx_out[port] == RX_BUFFER_SIZE) 
                {
                  buff_flow_flag[port]=0;//clear flow flag
                  uart_rx_out[port] = 0;
                }
                  
		return 0;
	}
}

/*******************************************************************************
 * 从指定串口发送指定个数的字节数据
 * 参数：
 *     port     :  串口索引号
 *     buffer   :  接收缓冲
 *     nums     :  指定数据字节数量
 * 返回：
 *     0 - 成功
 *     1 - 失败
 */
extern unsigned char S_PortSend(unsigned char port, unsigned char bytes);
unsigned char PortSends(unsigned char port, unsigned char* buffer, unsigned short nums)
{
    int  i = nums;
    
    if(port>MAX_PORT_NUM) return ERR_PORT;//skx 090223

    if(!uartOpenFlag[port]) return ERR_NO_OPEN;

    for(i=0;i<nums;i++)
            (void)S_PortSend(port, buffer[i]);
    
    return 0;
}


/*******************************************************************************
 * 从指定串口发送一个字节数据
 * 参数：
 *     port     :  串口索引号
 *     byte     :  待发送的数据字节
 * 返回：
 *     0 - 成功
 *     1 - 失败
 */
unsigned char S_PortSend(unsigned char port, unsigned char bytes)
{
	//if(port>MAX_PORT_NUM) return ERR_PORT;

	//if(!uartOpenFlag[port]) return ERR_NO_OPEN;

	switch(port)
	{
            case COM3:
              USART_SendData(USART3, bytes);
              break;
            case COM1:
              USART_SendData(USART1, bytes);
              break;
            case COM2:
              USART_SendData(USART2, bytes);
            break;
            default:
            break;
	}
	
	return 0;
}

/*******************************************************************************
 * 关闭打开的串口
 * 参数：
 *     port     :  串口索引号
 * 返回：
 *     0 - 成功
 *     1 - 失败
 */
unsigned char PortClose(unsigned char port)
{

	if(port>MAX_PORT_NUM) return ERR_PORT;//skx 090223

	if(!uartOpenFlag[port]) return ERR_NO_OPEN;

	switch(port)
	{
            case COM3:
              //Disable the USART Transmoit interrupt
              USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
              //Disable the USART Receive interrupt
              USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
              //Disable USART3
              USART_Cmd(USART3, DISABLE);
            break;
            case COM1:
              //Disable the USART Transmoit interrupt
              USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
              //Disable the USART Receive interrupt
              USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
              //Disable USART3
              USART_Cmd(USART1, DISABLE);
            break;
            case COM2:
              //Disable the USART Transmoit interrupt
              USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
              //Disable the USART Receive interrupt
              USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
              //Disable USART3
              USART_Cmd(USART2, DISABLE);
            break;

            
          default:
            break;
	}
	
	uartOpenFlag[port]=0;//skx 做上关闭标志
        buff_flow_flag[port]=0;//reset flow flag
	return 0;
}

void isr_com3(void)
{
    int i;
    
    if (USART_GetITStatus(USART3, USART_IT_RXNE)!= RESET)
    {
      /* Clear the USART3 Receive interrupt */
      USART_ClearITPendingBit(USART3, USART_IT_RXNE);
      
      //uart_rx_buffer[COM3][uart_rx_in[COM3]++] =(u8)(USART3->DR & (u16)0x0FF);
      
      uart_rx_buffer[COM3][uart_rx_in[COM3]++] = (unsigned char)USART_ReceiveData(USART3);//yxh
        
      if(uart_rx_in[COM3]>=RX_BUFFER_SIZE) 
      {
        buff_flow_flag[COM3]=1;//set flow flag
        uart_rx_in[COM3]=0;
      }
      /*
      if(uart_rx_in[COM3]==uart_rx_out[COM3]){
              uart_rx_out[COM3]++;
              if(uart_rx_out[COM3]==RX_BUFFER_SIZE) uart_rx_out[COM3]=0;	
      }
      */
    }
    
    if (USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
    {
      //Clear the USART3 transmit interrupt
      USART_ClearITPendingBit(USART3, USART_IT_TXE);
    }
	
}

void isr_com1(void)
{
    int i;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE)!= RESET)
    {
      /* Clear the USART3 Receive interrupt */
      USART_ClearITPendingBit(USART1, USART_IT_RXNE);
      
      //uart_rx_buffer[COM3][uart_rx_in[COM3]++] =(u8)(USART3->DR & (u16)0x0FF);
      
      uart_rx_buffer[COM1][uart_rx_in[COM1]++] = (unsigned char)USART_ReceiveData(USART1);//yxh
        
      if(uart_rx_in[COM1]>=RX_BUFFER_SIZE) 
      {
        buff_flow_flag[COM1]=1;//set flow flag
        uart_rx_in[COM1]=0;
      }
      /*
      if(uart_rx_in[COM3]==uart_rx_out[COM3]){
              uart_rx_out[COM3]++;
              if(uart_rx_out[COM3]==RX_BUFFER_SIZE) uart_rx_out[COM3]=0;	
      }
      */
    }
    
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
      //Clear the USART3 transmit interrupt
      USART_ClearITPendingBit(USART1, USART_IT_TXE);
    }
	
}

void isr_com2(void)
{
    int i;
    
    if (USART_GetITStatus(USART2, USART_IT_RXNE)!= RESET)
    {
      /* Clear the USART3 Receive interrupt */
      USART_ClearITPendingBit(USART2, USART_IT_RXNE);
      
      //uart_rx_buffer[COM3][uart_rx_in[COM3]++] =(u8)(USART3->DR & (u16)0x0FF);
      
      uart_rx_buffer[COM2][uart_rx_in[COM2]++] = (unsigned char)USART_ReceiveData(USART2);//yxh
        
      if(uart_rx_in[COM2]>=RX_BUFFER_SIZE) 
      {
        buff_flow_flag[COM2]=1;//set flow flag
        uart_rx_in[COM2]=0;
      }
      /*
      if(uart_rx_in[COM3]==uart_rx_out[COM3]){
              uart_rx_out[COM3]++;
              if(uart_rx_out[COM3]==RX_BUFFER_SIZE) uart_rx_out[COM3]=0;	
      }
      */
    }
    
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
      //Clear the USART3 transmit interrupt
      USART_ClearITPendingBit(USART2, USART_IT_TXE);
    }
	
}

unsigned char kbhit(void)
{
    if(uart_rx_in[COM3]==uart_rx_out[COM3] && ( !buff_flow_flag[COM3] ) )
      return 0xff;
    else if(uart_rx_in[COM1]==uart_rx_out[COM1] && ( !buff_flow_flag[COM1] ) )
      return 0xff;
    else if(uart_rx_in[COM2]==uart_rx_out[COM2] && ( !buff_flow_flag[COM2] ) )
      return 0xff;
    else
      return 0;
}


unsigned char getkey(void)
{
    unsigned char ch=0;
    while(!ch)
       PortRecv(COM3,&ch,0);
    return ch;
}



/******************************************************************************/
//add by skx for printf debug
  #include <stdarg.h>//must include this .h otherwise can not compile
  #include <stdio.h>

  #include <string.h>
void debug_printf(u8 col,u8 row,u8 mode,char *str,...)
{
      va_list       varg;
      int           retv;
      char          sbuffer[512];
      char * where;
      memset(sbuffer, 0, sizeof(sbuffer));
      where =&sbuffer[0];
      va_start( varg, str );
      retv=vsprintf(sbuffer,  str,  varg);
      va_end( varg );
      
      if(retv==0)return;
      UART_DataSend(USART1, sbuffer,retv);
      UART_DataSend(USART1,"\r\n",2);
}

void trace_debug_printf(char *str,...)
{
      va_list       varg;
      int           retv;
      char          sbuffer[512];
      char * where;
      memset(sbuffer, 0, sizeof(sbuffer));
      where =&sbuffer[0];
      va_start( varg, str );
      retv=vsprintf(sbuffer,  str,  varg);
      va_end( varg );
      
      if(retv==0)return;
      UART_DataSend(USART1, sbuffer,retv);
      UART_DataSend(USART1,"\r\n",2);
	
}

//test debug_printf
void uart_test(unsigned char cmd)
{ 
  int i,k=0;
  unsigned char arr[]="uart_test";
  //Disable the USART Receive interrupt
  USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//测试结果表明只要对方有连续数据进入则必然影响发送
  while(1)
  {
    for(i=0;i<9;i++)
    {
      debug_printf(0,0,0,"hello,world,loop:%d",k++);
      debug_printf(0,0,0,"hello,loop:%d,world,%c",k++,arr[i]);
      debug_printf(0,0,0,"hello,world,%s",arr);
    }  
    delay_ms(10);
  }
}


void uart_driver_test(unsigned char cmd)
{
  unsigned char ret,recbyte[256],timeout=0;
  ret=PortOpen(COM1,115200,8,0,1);
  int i=0;
  //ret=PortOpen(COM3,9600,8,0,1);
  if(ret)
  {
    ledgreen_on();
    while(1);
  }
 
  PortReset(COM1);
  
  while(  1 )
  {
    if( !kbhit()  && getkey()=='e') break;
    debug_printf(0,0,0,"waiting begin key:e");
    delay_ms(10);
  }
  
  debug_printf(0,0,0,"waiting for package and head byte must be a");
  
begin_recv:
  timeout=0;
  while( PortRecv(COM1,  &recbyte[0], timeout) );
  if(recbyte[0]=='a')
  {
    delay_ms(10);//@1此处必须延时使串口接受数据中断接收完毕再执行发送打印，
    //否则由于中断接收与发送并行进行将影响发送数据，而接收不受影响（为中断）
    debug_printf(0,0,0,"revc head byte ok:a");
    memset(recbyte,0,sizeof(recbyte));
    for(i=1;i<=20;i++)
    {
      if( PortRecv(COM1,  &(recbyte[i]), timeout) ) break;
    }
    for(;i>0;i--)
    {
      PortSends(COM1, &(recbyte[i-1]), 1);
    } 
    goto begin_recv;
    
  }
  else
  {
    delay_ms(10);//同@1
    debug_printf(0,0,0,"revc head byte err:%c",recbyte[0]);
    goto begin_recv;
  }
  
}

int Lib_ComCheckSend(uchar port) 
{
    if(gCom_OutRead[port]!=gCom_OutWrite[port]) 
      return 1;  //not finished
    return 0;
}

