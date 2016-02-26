#if 0
#include  "base.h"
#include  "comm.h"
#include  "vosapi.h"
#endif

#include "KF701DH.h"
#include "..\..\inc\FunctionList.h"

#include  "base.h"
#include  "comm.h"
#include  "vosapi.h"


#define RX_BUFFER_SIZE   512

#define UART2	         1
#define UART1	         0

#define MAX_PORT_NUM	 UART2



static volatile unsigned char  uart_rx_buffer[2][RX_BUFFER_SIZE];
static volatile unsigned int   uart_rx_in[2]={0},uart_rx_out[2]={0};
static int uartOpenFlag[2]={0};

static unsigned char buff_flow_flag[2]={0};//0-no flow 1-flow

void s_ComInit(void)
{
#if 0
    memset(gCom_PortUsed,0x00,sizeof(gCom_PortUsed));
    memset(gCom_InRead,0x00,sizeof(gCom_InRead));
    memset(gCom_InWrite,0x00,sizeof(gCom_InWrite));
    memset(gCom_OutRead,0x00,sizeof(gCom_OutRead));
    memset(gCom_OutWrite,0x00,sizeof(gCom_OutWrite)); 
#endif  
  
}

int Lib_ComOpen(uchar port, char *ComPara)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    char data_bits=0,stop_bits=0,parity=0,tmps[100];
    ushort i,j;
    long  baudrate;  
    const long baud_tab[]={600,1200,2400,4800,9600,14400,19200,28800,38400,
        57600,115200,230400};
  
    if(port>MAX_PORT_NUM) 
      return COM_INVALID_PORTNO;
  
      for(i=j=0;i<strlen(ComPara);i++)
    {
        if(ComPara[i]==',')break;
        if(ComPara[i]!=0x20)tmps[j++]=ComPara[i];
        if(j>=7)  return COM_INVALID_PARA;//too long baud rate string
    }
    if(i>=strlen(ComPara))      return COM_INVALID_PARA;//not found ',' following baud rate
    i++;
    tmps[j]=0;
    baudrate=atol(tmps);
    for(j=0;j<sizeof(baud_tab)/sizeof(long);j++)
    {
        if(baudrate==baud_tab[j]) break;
    }
    if(j>=sizeof(baud_tab)/sizeof(long))  return COM_INVALID_PARA;//invalid baud rate

    for(;i<strlen(ComPara);i++)
    {
        if(ComPara[i]!=0x20){data_bits=ComPara[i];break;}
    }
    if(i>=strlen(ComPara))               return COM_INVALID_PARA;//not found data bits
    if(data_bits<'5' || data_bits>'8')   return COM_INVALID_PARA; //invalid data bits
    i++;
    for(;i<strlen(ComPara);i++)
    {
        if(ComPara[i]!=0x20)break;
    }
    if(i>=strlen(ComPara))       return COM_INVALID_PARA;//not found ',' following data bits

    if(ComPara[i]!=',')          return COM_INVALID_PARA;  //not ',' following data bits
    i++;
    for(;i<strlen(ComPara);i++)
    {
        if(ComPara[i]!=0x20){parity=ComPara[i];break;}
    }
    if(i>=strlen(ComPara))       return COM_INVALID_PARA;//not found parity

    if(parity=='e')      parity='E';
    else if(parity=='n') parity='N';
    else if(parity=='o') parity='O';
    else if(parity!='N'&&parity!='E'&&parity!='O')
        return COM_INVALID_PARA;//invalid parity
    i++;

    for(;i<strlen(ComPara);i++)
    {
        if(ComPara[i]!=0x20)break;
    }
    if(i>=strlen(ComPara))
        return COM_INVALID_PARA;//not found ',' following parity
    if(ComPara[i]!=',')
        return COM_INVALID_PARA;//not ',' following parity
    i++;
    for(;i<strlen(ComPara);i++)
    {
        if(ComPara[i]!=0x20){stop_bits=ComPara[i];break;}
    }
    if(i>=strlen(ComPara))
        return COM_INVALID_PARA;//not found stop bits
    if(stop_bits!='1' && stop_bits!='2')
        return COM_INVALID_PARA;//invalid stop bits

        //Configure the UART
    USART_InitStructure.USART_BaudRate = baudrate;
    if(data_bits!='8')
      USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    else
      USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    
    if(stop_bits=='1')    
      USART_InitStructure.USART_StopBits = USART_StopBits_1;
    else
      USART_InitStructure.USART_StopBits = USART_StopBits_2;
      
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

          
      case UART2:
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
        
        //手写板 上电
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_11);
        
	break;
            
        case UART1:
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
        return 0;    
}




int Lib_ComClose(uchar port)
{
  if(port>MAX_PORT_NUM) 
    return COM_INVALID_PORTNO;//skx 090223
  if(!uartOpenFlag[port]) return 0;
  switch(port)
  {
     case UART1:
       //Disable the USART Transmoit interrupt
       
       USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
       //Disable the USART Receive interrupt

       USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
#if 0       //affect keyboard?
       
       //Disable USART3
       USART_Cmd(USART1, DISABLE);
#endif       
       break;
     case UART2:
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

void isr_com1(void)
{
    int i;
    
    if (USART_GetITStatus(USART1, USART_IT_RXNE)!= RESET)
    {
      /* Clear the USART3 Receive interrupt */
      USART_ClearITPendingBit(USART1, USART_IT_RXNE);
      
      //uart_rx_buffer[COM3][uart_rx_in[COM3]++] =(u8)(USART3->DR & (u16)0x0FF);
      
      uart_rx_buffer[UART1][uart_rx_in[UART1]++] = (unsigned char)USART_ReceiveData(USART1);//yxh
        
      if(uart_rx_in[UART1]>=RX_BUFFER_SIZE) 
      {
        buff_flow_flag[UART1]=1;//set flow flag
        uart_rx_in[UART1]=0;
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
    
    //13/10/10
    if(USART_GetITStatus(USART1, USART_IT_FE) != RESET)
    {
        /* Clear the USART1 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_FE);
        USART_ReceiveData(USART1);

    }    
    if(USART_GetITStatus(USART1, USART_IT_PE) != RESET)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        {
        }
        /* Clear the USART1 Parity error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_PE);
        USART_ReceiveData(USART1);
    }

    /* If a Overrun error is signaled by the card */
    //if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)//13/10/10
    {
        /* Clear the USART1 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
        USART_ReceiveData(USART1);
    }

    /* If a Noise error is signaled by the card */
    if(USART_GetITStatus(USART1, USART_IT_NE) != RESET)
    {
        /* Clear the USART1 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_NE);
        USART_ReceiveData(USART1);
    }
    
    /* If a error is signaled by the card */
    if(USART_GetITStatus(USART1, USART_IT_ERR) != RESET)
    {
        /* Clear the USART1 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_ERR);
        USART_ReceiveData(USART1);
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
      
      uart_rx_buffer[UART2][uart_rx_in[UART2]++] = (unsigned char)USART_ReceiveData(USART2);//yxh
        
      if(uart_rx_in[UART2]>=RX_BUFFER_SIZE) 
      {
        buff_flow_flag[UART2]=1;//set flow flag
        uart_rx_in[UART2]=0;
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
		//13/10/10
    if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)//13/10/10
    {
        /* Clear the USART2 Frame error pending bit */
        USART_ClearITPendingBit(USART2, USART_IT_ORE);
        USART_ReceiveData(USART2);
    }	
}

unsigned char kbhit(void)
{
    if(uart_rx_in[UART1]==uart_rx_out[UART1] && ( !buff_flow_flag[UART1] ) )
      return 0xff;
    else
      return 0;
}

#if 0
//unsigned char s_getkey(void)
unsigned char getkey(int port)
{
    unsigned char ch=0;
    while(!ch)
       PortRecv(port,&ch,0);
    return ch;
}
#endif


unsigned char getkey(void)
{
    if(Lib_KbCheck()==0)
	  return Lib_KbGetCh();
    else
	  return 0;
}

void debug_printf(u8 col,u8 row,u8 mode,char *str,...)
{
#ifdef UARTPRINF_DEBUG  
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
#endif      
}

void trace_debug_printf(char *str,...)
{
#ifdef UARTPRINF_DEBUG  
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


      //UART_DataSend(USART1,"\r\n",2);
  //    UART_DataSend(USART1,"\r",1); //13/05/29

      
#endif	
}


/****************************************************************************
  函数名     :
  描述       :  从串口发送单字节数据函数
  输入参数   :  1、uchar port：串口号
                2、uchar send_byte：要发送的数据
  输出参数   :  无
  返回值     :  0：成功，-6503：串口号错误，-6502：串口未打开，-6505：发送超时
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Lib_ComSendByte(uchar port,uchar send_byte)
{
    if(port>MAX_PORT_NUM)      
      return COM_INVALID_PORTNO;
    if(!uartOpenFlag[port]) 
      return COM_NOT_OPEN;
  
  switch(port)
  {
    case UART1:
      USART_SendData(USART1, send_byte);
      break;
    case UART2:
      USART_SendData(USART2, send_byte);
      break;
    default:
      return COM_INVALID_PORTNO;
      break;
  }
	
  return 0;  
}


int Lib_ComCheckSend(uchar port) 
{
 switch(port)
  {
    case UART1:
      if(USART_GetFlagStatus(USART1, USART_FLAG_TC))
        return 0;//finished
      else
        return 1;
      break;
    case UART2:
      if(USART_GetFlagStatus(USART2, USART_FLAG_TC))
        return 0;//finished
      else
        return 1;
      break;
    default:
      break;
  }
  return 0;
  
}

/****************************************************************************
  函数名     :
  描述       :  从串口接收单字节数据函数
  输入参数   :  1、uchar port：串口号
                2、uchar *recv_byte：接收到的数据
                3、int waitms：接收超时时间
  输出参数   :  无
  返回值     :  0：成功，-6503：串口号错误，-6502：串口未打开，-6504：接收超时
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Lib_ComRecvByte(uchar port,uchar *recv_byte,int waitms)
{
  unsigned long cnt;

  if(port>MAX_PORT_NUM) return COM_INVALID_PORTNO;//skx 090223
	
  if(!uartOpenFlag[port]) return COM_NOT_OPEN;

  if(recv_byte==NULL)
    return COM_TX_MEMOVER;
  
  if(  (waitms<0))
    return COM_INVALID_PARA;
  
  if(waitms == 0)
  { 
    if(uart_rx_in[port] == uart_rx_out[port]) 
    {
      if( !buff_flow_flag[port] )
         return COM_RX_TIMEOUT;
    }
    *recv_byte = uart_rx_buffer[port][uart_rx_out[port]++];
    if(uart_rx_out[port] == RX_BUFFER_SIZE)
    {
       buff_flow_flag[port]=0;//clear flow flag
       uart_rx_out[port] = 0;
    }
    return 0;
  }
  else
  {
    //TimerSet(3, waitms);
    TimerSet(2, waitms/10);//10ms
    while(uart_rx_in[port] == uart_rx_out[port] && ( !buff_flow_flag[port] ) )
    {
	//if( !TimerCheck(3) ) 
	if( !TimerCheck(2) ) 
          return COM_RX_TIMEOUT;
    }
    *recv_byte = uart_rx_buffer[port][uart_rx_out[port]++];
    if(uart_rx_out[port] == RX_BUFFER_SIZE) 
    {
        buff_flow_flag[port]=0;//clear flow flag
        uart_rx_out[port] = 0;
    }
                  
    return 0;
  }  
}

int Lib_ComRecv(uchar port,uchar *recv_data,int max_len,int *recv_len,int waitms)
{
  unsigned long cnt;
  int recvlen; 

  if(port>MAX_PORT_NUM) 
    return COM_INVALID_PORTNO;
	
  if(!uartOpenFlag[port]) 
    return COM_NOT_OPEN;
  
  if(recv_data==NULL)
    return COM_TX_MEMOVER;
  if( (max_len<=0) || (waitms<0))
    return COM_INVALID_PARA;
  
  recvlen = 0;

  if(waitms == 0)
  { 
    if(uart_rx_in[port] == uart_rx_out[port]) 
    {
      if( !buff_flow_flag[port] )
         return COM_RX_TIMEOUT;
    }
    *recv_data = uart_rx_buffer[port][uart_rx_out[port]++];
    if(uart_rx_out[port] == RX_BUFFER_SIZE)
    {
       buff_flow_flag[port]=0;//clear flow flag
       uart_rx_out[port] = 0;
    }
    return 0;
  }
  else
  {
    TimerSet(2, waitms/10);//10ms 3->2
    while(1)
    {
      if(uart_rx_in[port] == uart_rx_out[port] && ( !buff_flow_flag[port] ) )
      {
	if( !TimerCheck(2) ) //3->2
        {
          *recv_len = recvlen;
          return COM_RX_TIMEOUT;
        }
        continue;
      }
      recv_data[recvlen++] = uart_rx_buffer[port][uart_rx_out[port]++];
      if(uart_rx_out[port] == RX_BUFFER_SIZE) 
      {
        buff_flow_flag[port]=0;//clear flow flag
        uart_rx_out[port] = 0;
      }
      if(recvlen>=max_len)
      {
        *recv_len = recvlen;
        return 0;
      }
      
    }
    
    
                  
    return 0;
  }  
  
  
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
            case UART1:
              USART_SendData(USART1, bytes);
              break;
            case UART2:
              USART_SendData(USART2, bytes);
              break;
            default:
              break;
	}
	
	return 0;
}


int Lib_ComSend(uchar port,BYTE *send_data,int send_len)
{
    int i;
    
    if(port>MAX_PORT_NUM)      
      return COM_INVALID_PORTNO;
    if(!uartOpenFlag[port]) 
      return COM_NOT_OPEN;
#if 0
    //--check if it will be overflowed
    if(send_len>=RX_BUFFER_SIZE)
    {
        errno=403;//overflowed for too long input
        return 4;
    }
#endif   

    //--check if it will be overflowed
    if(send_len>=(9*1024))
    {
        //errno=403;//overflowed for too long input
        return COM_TX_MEMOVER;
    }
    
    for(i=0;i<send_len;i++)
    {
      S_PortSend(port, send_data[i]);
      
    }
    return 0;
}

int Lib_ComReset(uchar port)
{
    if(port>MAX_PORT_NUM) 
      return COM_INVALID_PORTNO;
    if(!uartOpenFlag[port]) 
      return COM_NOT_OPEN;
    
    switch(port)
    {
      case UART2:
          USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
          USART_ClearITPendingBit(USART2, USART_IT_RXNE);
          uart_rx_out[port]=0;
          uart_rx_in[port]=0;	
          buff_flow_flag[port]=0;
          USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
          break;

      case UART1:
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
    
    return 0;  
  
}

int TestCom(void)
{
     uchar buff[100];
     int iret;

     s_UartPrint(COM_DEBUG,"Test Com...\r\n");
     //Lib_LcdCls();
     //s_ComInit();
     Lib_ComOpen(UART1,"115200,8,N,1");
     Lib_ComReset(UART1);
     Lib_ComOpen(UART2,"115200,8,N,1");
     Lib_ComReset(UART2);
     memset(buff,0,sizeof(buff)); 
     while(1)
     {
          iret=Lib_ComRecvByte(UART1,&buff[0],10);
          if(iret==0)
          {
               Lib_ComSendByte(UART1,buff[0]);
               //Lib_DelayMs(500);
               //i++;
               //if(i>20) break;
               if(buff[0]=='e' || buff[0]=='E') break;
          }
          iret=Lib_ComRecvByte(UART2,&buff[0],10);
          if(iret==0)
          {
               Lib_ComSendByte(UART2,buff[0]);
               //Lib_DelayMs(500);
               //i++;
               //if(i>20) break;
               if(buff[0]=='e' || buff[0]=='E') break;
          }

     }
    return 0;
}


unsigned char get_lrc_for_sign_panel(unsigned char *buf)
{
#define FRAME_BYTE(x, y)			(((unsigned char *) (x))[(y)])
#define FRAME_DATA_LENGTH_SIGN_PANEL(x)		(FRAME_BYTE((x), 3) << 8 | FRAME_BYTE((x), 2))
#define FRAME_LENGTH_SIGN_PANEL(x)		(FRAME_DATA_LENGTH_SIGN_PANEL((x)) + 5)
  
	int i;
	unsigned char lrc = 0;

	for (i = 0; i < FRAME_LENGTH_SIGN_PANEL(buf) - 2; i++)
		lrc ^= buf[i + 1];

	return lrc;
}

int send_frame_to_sign_panel(unsigned char cmd, unsigned char *data, int data_len)
{
#define MAX_RECV_BUFSIZE 8096
#define STX 0x02
	unsigned char buf[MAX_RECV_BUFSIZE];

	data_len = (data != NULL) ? (data_len % MAX_RECV_BUFSIZE) : 0;

	buf[0] = STX;
	buf[1] = cmd;
	buf[2] = data_len % 256;
	buf[3] = data_len / 256;

	if (data_len > 0)
		memcpy(&buf[4], data, data_len);

	buf[data_len + 4] = get_lrc_for_sign_panel(buf);

	//return send_to_sign_panel(buf, data_len + 5, 0);
        return Lib_ComSend(1, buf, data_len + 5);
}


int Sign_panel_handshake(unsigned char *frame)
{
#define CMD_SIGN_PANEL_HANDSHAKE 0xA0
  
	return send_frame_to_sign_panel(CMD_SIGN_PANEL_HANDSHAKE, NULL, 0);
}