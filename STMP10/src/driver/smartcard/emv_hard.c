/**
 * =============================================================================
 * 文档主要实现STM32F10x系列智能卡读卡器功能，与硬件紧密关联的部分。开发驱动时请
 * 参考"emv_hard.h"中的接口声明，实现其接口可快速移植协议。
 *
 * Author            : lian guang ping
 * Date              : 2012-4-24
 * Hardware platform : v10p
 * Email             : lian98@163.com
 * =============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h> 
#if 0
#include "icc.h"
#include "base.h"
#include "vosapi.h"
#endif
//#include "usip.h"
#include "KF701DH.h"

#include "..\..\inc\FunctionList.h"

#include  "base.h"
#include  "icc.h"
#include  "vosapi.h"






#if 0
#include "emv_errno.h"
#include "emv_hard.h"
#include "emv_queue.h"
#include "emv_patch.h"
#include "emv_core.h"

#include "stm32f10x_type.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_it.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"

#include "kf323_emv_hard.h"

static volatile struct emv_core  *scs_dev = NULL; /*当前设备句柄*/
volatile struct emv_queue      	  scs_queue[ 2 ]; 
static volatile int               repeat_chc = 0; /*错误重发次数*/
static volatile int               scs_stat;       /**/
#define SC_UART_WAIT_TS          ( 1 << 0 )
#define SC_UART_RECV_EV          ( 1 << 1 )
#define SC_UART_SEND_EV          ( 1 << 2 )

static volatile int               work_step;    /*卡片复位阶段标记*/ 
#define SC_UART_WAIT_EVENT       ( 1 << 2 )
#define SC_UART_CLKS_EVENT       ( 1 << 3 )
#define SC_UART_RSTL_EVENT       ( 1 << 4 )
#define SC_UART_RSTH_EVENT       ( 1 << 5 )
#define SC_UART_IGDT_EVENT       ( 1 << 6 )

static volatile unsigned int       scs_rwt;     /* 有效复位等待时间 */ 
static volatile unsigned int       scs_etu;     
static volatile int                wait_counter;/* 等待计数器 */ 
volatile unsigned int              scs_bwt;     /* 块等待时间和'TS'等待时间 */ 
volatile unsigned int              scs_cwt;     /* T=1 CWT and T=0 WWT */ 
volatile unsigned int              scs_cgt;

volatile unsigned int              scs_crg = 0; /* 虚拟控制寄存器 */
#define CONV_EN                  ( 1 << 0 )     /* 通信逻辑 */ 
#define CRPT_EN                  ( 1 << 1 )     /* 错误重发/收标记 */
#define TTSC_EN                  ( 1 << 2 )     /* TS字符是否已经收到 */         
#define PARC_B                   ( 1 << 3 )     /* 偶校验位 */
#define PARE_SB                  ( 1 << 4 )     /* 校验错误标记 */

volatile int                       twt_state;       /*超时标记*/
volatile int                       twt_wait_counter;/*超时计数器*/
#define TWT_READY  			       0
#define TWT_START  			       1
#define TWT_STOP   			       2

/*标记SAM卡共用电源和时钟信号*/
static unsigned int                sam_vcc_used_flag = 0;
static unsigned int                sam_clk_used_flag = 0;
static unsigned int                sam_slot_last_flag = 0;

/*本地全局函数区*/
static int  USART_TOV_Timer( int enable, int etu );          /*超时定时器控制*/
static void Timer5_Isr( void );
static void Usart1_Isr( void );
static void Usart2_Isr( void );
static unsigned char scs_inverse_ch( unsigned char c );
static int  emv_hard_config( struct emv_core *pdev );        /*工作参数配置*/
static int  emv_hard_power_signal( int channel, int vcc );   /*电源控制信号控制*/
static int  emv_hard_clock_enable( int channel, int enable );/*时钟控制信号控制*/
static int  emv_hard_reset_signal( int channel, int level ); /*卡片复位信号控制*/

/**
 * 清智能卡控制器1中断
 */
static void USART1_ISR_Clr()
{
	unsigned char ch;

	ch = SC_SML_UART->SR;
	ch = SC_SML_UART->DR;
	
    ch = ch;
    
	return;
}

/**
 * 清智能卡控制器2中断
 */
static void USART2_ISR_Clr()
{
	unsigned char ch;

	ch = SC_BIG_UART->SR;
	ch = SC_BIG_UART->DR;
	
    ch = ch;

	return;
}

/**
 * 智能卡波特率配置
 */
static void USART_SmartCardSetBrr( int brr )
{
	unsigned int tmpreg            = 0x00; 
	unsigned int apbclock          = 0x00;
	unsigned int integerdivider    = 0x00;
	unsigned int fractionaldivider = 0x00;

    if( NULL == scs_dev )return;
    
    if( brr > 40000 )brr -= 12;/*补偿边沿损失*/
    else brr -= 2; 
    
	if( USC == scs_dev->terminal_ch )apbclock = 72000000ul / 2;
	else apbclock = 72000000ul;
	
	/* Determine the integer part Bug: CR1_OVER8_Set (used in original code, removed now) is undocumented.*/
	integerdivider = ( 25ul * apbclock ) / ( 4 * brr );
	tmpreg = ( integerdivider / 100 ) << 4;
	fractionaldivider = integerdivider - ( 100 * ( tmpreg >> 4 ) ); //Determine the fractional part 
	tmpreg |= ( ( ( fractionaldivider * 16 ) + 50 ) / 100 ) & ( (unsigned char)0x0F );
	
	if( USC == scs_dev->terminal_ch )SC_BIG_UART->BRR = ( unsigned short )tmpreg;
	else SC_SML_UART->BRR = ( unsigned short )tmpreg;	
	
	//dbg_msg( 1, "BRR = %X\r\n", tmpreg );
	
	return;
}

/**
 * 智能卡控制器1配置
 */
static void USART1_Config( )
{
	GPIO_InitTypeDef       GPIO_InitStructure;
	USART_InitTypeDef      USART_InitStructure;
	NVIC_InitTypeDef       NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );				   

	//AFIO->MAPR &= ~( 1 << 2 );
	
	NVIC_InitStructure.NVIC_IRQChannel                  = USART1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 1;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
	NVIC_Init( &NVIC_InitStructure );	
   
	GPIO_InitStructure.GPIO_Pin   = SAM_CLK_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init( SAM_GPIO_PORT_1, &GPIO_InitStructure );

	GPIO_InitStructure.GPIO_Pin   = SAM_IO_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;
	GPIO_Init( SAM_GPIO_PORT_1, &GPIO_InitStructure );		

	USART_SetPrescaler( SC_SML_UART, 0x0A );// USART Clock 3.6MHz
	USART_SetGuardTime( SC_SML_UART, 0x12 );// USART Guard Time 12 Bit 

	USART_InitStructure.USART_Clock      = USART_Clock_Disable;
	USART_InitStructure.USART_CPOL       = USART_CPOL_Low;
	USART_InitStructure.USART_CPHA       = USART_CPHA_1Edge;
	USART_InitStructure.USART_LastBit    = USART_LastBit_Enable;
	USART_InitStructure.USART_BaudRate   = FCLK_CARD / 372;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1_5;
	USART_InitStructure.USART_Mode 	     = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity     = USART_Parity_No;	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init( SC_SML_UART, &USART_InitStructure ); 
        Set_IRQProcessFunc( USART1_IRQChannel, Usart1_Isr );
        
	USART_ITConfig( SC_SML_UART, USART_IT_IDLE, DISABLE ); 
	USART_ITConfig( SC_SML_UART, USART_IT_TXE, DISABLE ); 
	USART_ITConfig( SC_SML_UART, USART_IT_RXNE, DISABLE ); 
	USART_ITConfig( SC_SML_UART, USART_IT_TC, DISABLE ); 
	USART_ITConfig( SC_SML_UART, USART_IT_FE, DISABLE ); 
	USART_ITConfig( SC_SML_UART, USART_IT_PE, DISABLE ); 

	USART_Cmd( SC_SML_UART, ENABLE );                  // Enable SC_BIG_UART
	USART_SmartCardNACKCmd( SC_SML_UART, DISABLE ); // Enable the NACK Transmission */ 	
	USART_SmartCardCmd( SC_SML_UART, ENABLE );      // Enable the Smartcard Interface */
	
        Set_IRQProcessFunc( USART1_IRQChannel, Usart1_Isr );
        
	return;
}

/**
 * 智能卡控制器2配置
 */
static void USART2_Config( )
{
	GPIO_InitTypeDef       GPIO_InitStructure;
	USART_InitTypeDef      USART_InitStructure;
	NVIC_InitTypeDef       NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );  // enable usart2 clock				   

	//AFIO->MAPR &= ~(1 << 3); 				 // no remap
	
	NVIC_InitStructure.NVIC_IRQChannel                  = USART2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority       = 1;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
   
	GPIO_InitStructure.GPIO_Pin   = ICC_CLK_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init( ICC_GPIO_PORT_1, &GPIO_InitStructure );
  
	GPIO_InitStructure.GPIO_Pin   = ICC_IO_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;
	GPIO_Init( ICC_GPIO_PORT_1, &GPIO_InitStructure );		
	
	USART_SetPrescaler( SC_BIG_UART, 0x05 );// USART Clock 3.6MHz
	USART_SetGuardTime( SC_BIG_UART, 0x12 );// USART Guard Time 12 Bit 
  
	USART_InitStructure.USART_Clock      = USART_Clock_Disable;
	USART_InitStructure.USART_CPOL       = USART_CPOL_Low;
	USART_InitStructure.USART_CPHA       = USART_CPHA_1Edge;
	USART_InitStructure.USART_LastBit    = USART_LastBit_Enable;
	USART_InitStructure.USART_BaudRate   = FCLK_CARD / 372;
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1_5;
	USART_InitStructure.USART_Mode 	     = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity     = USART_Parity_No;	
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init( SC_BIG_UART, &USART_InitStructure );
	
	USART_ITConfig( SC_BIG_UART, USART_IT_IDLE, DISABLE ); 
	USART_ITConfig( SC_BIG_UART, USART_IT_TXE, DISABLE ); 
	USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, DISABLE ); 
	USART_ITConfig( SC_BIG_UART, USART_IT_TC, DISABLE ); 
	USART_ITConfig( SC_BIG_UART, USART_IT_FE, DISABLE ); 
	USART_ITConfig( SC_BIG_UART, USART_IT_PE, DISABLE ); 

	USART_Cmd( SC_BIG_UART, ENABLE );                  // Enable SC_BIG_UART
	USART_SmartCardNACKCmd( SC_BIG_UART, DISABLE ); // Enable the NACK Transmission */ 	
	USART_SmartCardCmd( SC_BIG_UART, ENABLE );      // Enable the Smartcard Interface */
	
        Set_IRQProcessFunc( USART2_IRQChannel, Usart2_Isr );
        
	return;
}

/**
 * 智能卡控制器1的中断服务函数
 */
void Usart1_Isr()
{
    unsigned char ch         = 0;	
	int           parity_bit = 0;
    
	if( TWT_READY == twt_state ) // to start atr-wwt counter
	{
		twt_state = TWT_START;
	}
	
	if( SC_UART_RECV_EV & scs_stat )
	{ 	    		
		if( SC_SML_UART->SR & ( 1 << 5 ) )
		{   
			wait_counter       =   scs_cwt;
			work_step          =   SC_UART_WAIT_EVENT;

			if( scs_stat & SC_UART_WAIT_TS ) // TS character 
			{    
				parity_bit = SC_SML_UART->DR;	
				if( 0x0103 == parity_bit )
				{         
					ch        =  0x3F;
					scs_crg  |=  CONV_EN;					
					SC_SML_UART->CR1 |= 1 << 10;    // parity enable
					SC_SML_UART->CR1 |= 1 << 9;    //odd parity
				}
				else if( 0x013B == parity_bit )
				{    
					ch 	     = 0x3B;
					scs_crg &= ~CONV_EN;				
					SC_SML_UART->CR1 |=  ( 1 << 10 );    // parity enable
					SC_SML_UART->CR1 &= ~( 1 << 9 );   // even parity 
				}
				else
				{
					ch = 0xff;  // TS error
				}
																
				scs_queue[ 1 ].qbuf[ scs_queue[ 1 ].ip ] = ch;
				scs_queue[ 1 ].ip++;
				scs_stat &= ~SC_UART_WAIT_TS;
				return ;
				
			}

			if( SC_SML_UART->SR & 1 )   // parity error
			{
				ch = ( unsigned char )SC_SML_UART->DR;					
				if( scs_crg & CRPT_EN )	//T=0;
				{
					if( repeat_chc != 0 )
					{
						repeat_chc--;							
						return;
					}
					else
					{
						scs_crg |= PARE_SB;
					}
				}
				else 	// T=1 or ATR 
				{	
					scs_crg |= PARE_SB;
				}
			}

			if( scs_crg & PARE_SB )	
			{
				scs_crg &= ~PARE_SB;
			
				if( 0 == scs_queue[ 1 ].sf )
				{
					scs_queue[ 1 ].sf = ICC_ERR_PARS;
					scs_queue[ 1 ].pf = scs_queue[ 1 ].ip;
				}
				return ;
			}	
			
			repeat_chc = 4;  	
			ch = ( unsigned char )SC_SML_UART->DR;		
			scs_queue[ 1 ].qbuf[ scs_queue[ 1 ].ip ] = scs_inverse_ch(ch);
			scs_queue[ 1 ].ip++;
			if( MAX_QBUF_SIZE == scs_queue[ 1 ].ip )
			{
				scs_queue[ 1 ].ip = 0; 
			}					
			
		}  		
    }
	
    
    if( SC_UART_SEND_EV & scs_stat )
    {					
        if( SC_SML_UART->SR & ( 1 << 6 ) )     //TC flag is set
        {       			
        	if( ( SC_SML_UART->SR & ( 1 << 5 ) ) && ( SC_SML_UART->SR & ( 1 << 1 ) ) ) //frame error, four time re-send
        	{    	
				USART1_ISR_Clr();
				if( scs_crg & CRPT_EN )
				{
	        		if( repeat_chc-- )
	        		{						
			            ch = scs_queue[ 1 ].qbuf[ scs_queue[ 1 ].op - 1 ];
			            SC_SML_UART->DR = scs_inverse_ch( ch );		
					}
					else
					{
						scs_queue[1].sf = ICC_ERR_PARS;
						scs_queue[1].pf = 0;
						scs_queue[1].ip = 1;
						scs_queue[1].op = 0;		
						USART_ITConfig( SC_SML_UART, USART_IT_TC,   DISABLE );
						USART_ITConfig( SC_SML_UART, USART_IT_RXNE, DISABLE );  	   
					}
				}
				
        	}
			else
			{		
				USART1_ISR_Clr();
				repeat_chc = 4;

				if( (int)( scs_queue[ 1 ].ip ) != (int)( scs_queue[ 1 ].op ) )
		        {
		            ch = scs_queue[ 1 ].qbuf[ scs_queue[ 1 ].op ];
		            scs_queue[ 1 ].op++;
					
		            if( MAX_QBUF_SIZE == scs_queue[ 1 ].op )
		            {
		                scs_queue[ 1 ].op = 0;
		            }    
					
					if( (int)( scs_queue[ 1 ].ip ) == (int)( scs_queue[ 1 ].op ) )
					{
						USART_SetGuardTime( SC_SML_UART, 5 ); // delay for re-sending  while frame error	
					}
										
		            SC_SML_UART->DR = scs_inverse_ch( ch );	
					
		        }
		        else
		        {          
		            scs_stat        =  SC_UART_RECV_EV;	 // send completed, convert to RX mode 
					wait_counter	=  scs_bwt; 
					work_step	    =  SC_UART_WAIT_EVENT;	
					
					USART1_ISR_Clr();	
					USART_ITConfig( SC_SML_UART, USART_IT_TC,   DISABLE );			
					USART_ITConfig( SC_SML_UART, USART_IT_RXNE, ENABLE );  
					
					USART_TOV_Timer( ENABLE, scs_etu );		
				}
			}						
        }
    }

    return;
}

/**
 * 智能卡控制器2的中断服务函数
 */
void Usart2_Isr()
{
    unsigned char ch         = 0;	
	int           parity_bit = 0;

	if( TWT_READY == twt_state ) // to start atr-wwt counter
	{
		twt_state = TWT_START;
	}	
	
	if( SC_UART_RECV_EV & scs_stat )
	{ 	    		
		if( SC_BIG_UART->SR & ( 1 << 5 ) ) //The software must wait for the RXNE flag to be set before clearing the PE bit
		{           	
			wait_counter = scs_cwt;
			work_step    = SC_UART_WAIT_EVENT;

			if( scs_stat & SC_UART_WAIT_TS ) // TS character 
			{    
				parity_bit = SC_BIG_UART->DR;	
				if( 0x0103 == parity_bit )
				{         
					ch        =  0x3F;
					scs_crg  |=  CONV_EN;					
					SC_BIG_UART->CR1 |= 1 << 10;    // parity enable
					SC_BIG_UART->CR1 |= 1 << 9;    //odd parity
				}
				else if( 0x013B == parity_bit )
				{    
					ch 	      = 0x3B;
					scs_crg  &= ~CONV_EN;				
					SC_BIG_UART->CR1 |= 1 << 10;    // parity enable
					SC_BIG_UART->CR1 &= ~( 1 << 9 );   // even parity 
				}
				else
				{
					ch = 0xff;  // TS error
				}
																
				scs_queue[ 0 ].qbuf[ scs_queue[ 0 ].ip ] = ch;
				scs_queue[ 0 ].ip++;
				scs_stat &= ~SC_UART_WAIT_TS;
				return ;
				
			}

			if( SC_BIG_UART->SR & 1 )   // parity error
			{
				ch = ( unsigned char )SC_BIG_UART->DR;					
				if(scs_crg & CRPT_EN)	//T=0;
				{
					if( repeat_chc != 0 )
					{
						repeat_chc--;	
						
						//dbg_msg( 1, "recv pars!\r\n" );
												
						return;
					}
					else
					{
						scs_crg |= PARE_SB;
					}
				}
				else 	// T=1 or ATR 
				{	
					scs_crg |= PARE_SB;
				}
			}

			if( scs_crg & PARE_SB )	
			{
				scs_crg &= ~PARE_SB;
				
                if( 0 == scs_queue[ 0 ].sf )
				{
					scs_queue[ 0 ].sf = ICC_ERR_PARS;
					scs_queue[ 0 ].pf = scs_queue[ 0 ].ip;
				}
				
				if( 0 == scs_dev->terminal_ptype )
				{        		
					emv_hard_power_dump( ( struct emv_core* )scs_dev ); //power down
				}
				return ;
			}	
			
			repeat_chc = 4;  	
			ch = ( unsigned char )SC_BIG_UART->DR;		
			scs_queue[ 0 ].qbuf[ scs_queue[ 0 ].ip ] = scs_inverse_ch(ch);
			scs_queue[ 0 ].ip++;

			if( MAX_QBUF_SIZE == scs_queue[ 0 ].ip )
			{
				scs_queue[ 0 ].ip = 0; 
			}					
		}  		
    }
	
    
    if( SC_UART_SEND_EV & scs_stat )
    {					
        if( SC_BIG_UART->SR & ( 1 << 6 ) )     //TC flag is set
        {       			
        	if((SC_BIG_UART->SR & ( 1 << 5 ) ) && ( SC_BIG_UART->SR & 2 )) //frame error, four time re-send
        	{    	
				USART2_ISR_Clr();
				if( scs_crg & CRPT_EN )
				{
	        		if( repeat_chc-- )
	        		{						
			            ch = scs_queue[ 0 ].qbuf[ scs_queue[ 0 ].op - 1 ];
			            SC_BIG_UART->DR = scs_inverse_ch( ch );		
					}
					else
					{
						scs_queue[0].sf = ICC_ERR_PARS;
						scs_queue[0].pf = 0;
						scs_queue[0].ip = 1;
						scs_queue[0].op = 0;		
						USART_ITConfig( SC_BIG_UART, USART_IT_TC,   DISABLE );
						USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, DISABLE );  	
						emv_hard_power_dump( ( struct emv_core* )scs_dev ); //power down	
						
						//dbg_msg( 1, "send pars!\r\n" );
						  
					}
				}
				
        	}
			else
			{		
				USART2_ISR_Clr();
				repeat_chc = 4;

				if( (int)(scs_queue[ 0 ].ip) != (int)(scs_queue[ 0 ].op) )
		        {
		            ch = scs_queue[ 0 ].qbuf[ scs_queue[ 0 ].op ];
		            scs_queue[ 0 ].op++;
					
		            if( MAX_QBUF_SIZE == scs_queue[ 0 ].op )
		            {
		                scs_queue[ 0 ].op = 0;
		            }    
					
					if( (int)(scs_queue[ 0 ].ip) == (int)(scs_queue[ 0 ].op) )
					{
						USART_SetGuardTime( SC_BIG_UART, 5 ); // delay for re-sending  while frame error	
					}
		            SC_BIG_UART->DR = scs_inverse_ch( ch );	
		        }
		        else
		        {          
		            scs_stat     =  SC_UART_RECV_EV;	 // send completed, convert to RX mode 
					wait_counter =  scs_bwt; 
					work_step	 =  SC_UART_WAIT_EVENT;		
					USART2_ISR_Clr();
					USART_ITConfig( SC_BIG_UART, USART_IT_TC,   DISABLE );			
					USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, ENABLE );  
					USART_TOV_Timer( 1, scs_etu );		
				}
			}						
        }
    }

    return;
}

/**
 * 超时定时器配置
 */
static int USART_TOV_Timer( int enable, int etu )
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    
    TIM_Cmd( TIM5, DISABLE );
    TIM_TimeBaseStructure.TIM_Period        = etu - 1;      
    TIM_TimeBaseStructure.TIM_Prescaler     = 0x13;       
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up; 
    TIM_TimeBaseInit( TIM5, &TIM_TimeBaseStructure );
    
    TIM_ITConfig( TIM5, TIM_IT_Update, ENABLE );
    TIM_ClearITPendingBit( TIM5, TIM_IT_Update );
    if( enable )
    {
    	TIM_Cmd( TIM5, ENABLE );
    }
    
    return 0;	
}

/**
 * 超时定时器中断服务函数
 */
void Timer5_Isr()
{	
	TIM5->SR = 0;
	
	switch( work_step )
    {
   		case SC_UART_CLKS_EVENT:
   		{
	        wait_counter--;
	        if( 0 == wait_counter )
	        {
	        	emv_hard_reset_signal( scs_dev->terminal_ch, 0 );
	        	wait_counter  =  scs_rwt;
	        	work_step     =  SC_UART_RSTH_EVENT;
	        	emv_hard_clock_enable( scs_dev->terminal_ch, 1 );
	       	}
	 	}
	 	break;
    
		case SC_UART_RSTL_EVENT:
		{
		    wait_counter--;
		    if( 0 == wait_counter )
		    {
		        emv_hard_reset_signal( scs_dev->terminal_ch, 0 );
		        wait_counter =   scs_rwt;
		        work_step    =   SC_UART_RSTH_EVENT;
		    }
		}
		break;
    
		case SC_UART_RSTH_EVENT:
		{
		    wait_counter--;
		    if( 0 == wait_counter )
		    {	
		    	scs_dev->queue->ip = 0;
		    	scs_dev->queue->op = 0;
		    	scs_dev->queue->sf = 0;
		    	scs_dev->queue->pf = INVALID_INDEX;
		    	wait_counter = scs_bwt;
			    work_step = SC_UART_WAIT_EVENT;	
			
			    emv_hard_reset_signal( scs_dev->terminal_ch, 1 );	
                
                if( USC == scs_dev->terminal_ch )
                {
                    USART2_ISR_Clr();			
			        USART_ITConfig( SC_BIG_UART, USART_IT_TC, DISABLE );		
			        USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, ENABLE );
                }
                else
                {
                    USART1_ISR_Clr();			
			        USART_ITConfig( SC_SML_UART, USART_IT_TC, DISABLE );		
			        USART_ITConfig( SC_SML_UART, USART_IT_RXNE, ENABLE );   
                }
			    USART_TOV_Timer( 1, scs_etu );	
		  }
		}
		break;
    	
	    case SC_UART_WAIT_EVENT:
	    {
	        wait_counter--;
	        if( 0 == wait_counter )
	        {   				
	            if( (int)( scs_dev->queue->ip ) == (int)( scs_dev->queue->op ) )/* WWT or BWT or TS */
	            {
	                scs_dev->queue->sf = ICC_ERR_BWT;					
	            }
	            else
	            {
	                scs_dev->queue->sf = ICC_ERR_CWT;
	            }
	            scs_dev->queue->pf = (int)scs_dev->queue->ip;
	            work_step = 0;	
	            
	            if( USC == scs_dev->terminal_ch )
                {		
				    USART_ITConfig( SC_BIG_UART, USART_IT_TC, DISABLE );
				    USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, DISABLE );
				}
				else
				{
				    USART_ITConfig( SC_SML_UART, USART_IT_TC, DISABLE);
				    USART_ITConfig( SC_SML_UART, USART_IT_RXNE, DISABLE);  
				}
				USART_TOV_Timer( 0, 0 ); 	
	        }
	    }
	   	break;

		case SC_UART_IGDT_EVENT: /*通用定时器*/
		{
			wait_counter--;
			if( 0 == wait_counter )
			{
				USART_TOV_Timer( 0, 0 );
				work_step = 0;
			}
		}
		break;
		
		default:
		break;
	}

	if( TWT_START == twt_state )
    {
        twt_wait_counter --;   
        if( 0 == twt_wait_counter )
        {
            scs_dev->queue->sf = ICC_ERR_CWT;
            scs_dev->queue->pf = scs_dev->queue->ip;
            work_step    = 0;
            	
            if( USC == scs_dev->terminal_ch )
            {
			    USART_ITConfig( SC_BIG_UART, USART_IT_TC, DISABLE );
			    USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, DISABLE );
			}
			else
			{
			    USART_ITConfig( SC_SML_UART, USART_IT_TC, DISABLE );
			    USART_ITConfig( SC_SML_UART, USART_IT_RXNE, DISABLE );   
			}
			USART_TOV_Timer( 0, 0 );
        }  
    }
	TIM5->SR = 0;
	
    return;
}

/**
 * ============================ 硬件初始化 ================================
 */
int  emv_hard_init( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    
    /*初始化SAM卡共用电源、时钟标志信号*/
    sam_vcc_used_flag  = 0;
    sam_clk_used_flag  = 0;    
    sam_slot_last_flag = 0;

	/*配置所用到的外设时钟*/
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOE, ENABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOF, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );
	
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM5, ENABLE );
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE );
	
    /*配置定时器中断系统*/
    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQChannel;
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  	NVIC_Init(&NVIC_InitStructure);
    
	/*智能卡的用户卡在位检测中断使用系统定时器扫描方式*/
	;
	
	/*配置用户卡信号线*/
	GPIO_InitStructure.GPIO_Pin   = ICC_VCC_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( ICC_GPIO_PORT_2, &GPIO_InitStructure );
	GPIO_SetBits( ICC_GPIO_PORT_2, ICC_VCC_PIN );

	GPIO_InitStructure.GPIO_Pin   = ICC_RST_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( ICC_GPIO_PORT_2, &GPIO_InitStructure );
	GPIO_ResetBits( ICC_GPIO_PORT_2, ICC_RST_PIN );
    
    /*用户卡片在位检测信号,必须拔出Jtag否则一直为低电平*/
    //AFIO->MAPR &= ~( 7 << 24 );
    //AFIO->MAPR |=  ( 1 << 26 );
    
    GPIO_InitStructure.GPIO_Pin   = ICC_DET_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( ICC_GPIO_PORT_1, &GPIO_InitStructure );
	GPIO_SetBits( ICC_GPIO_PORT_1, ICC_DET_PIN );
    
	/*配置SAM卡信号*/
	GPIO_InitStructure.GPIO_Pin   = SAM_VCC_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( SAM_GPIO_PORT_1, &GPIO_InitStructure );
	GPIO_SetBits( SAM_GPIO_PORT_1, SAM_VCC_PIN );

	GPIO_InitStructure.GPIO_Pin   = SAM_RST_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( SAM_GPIO_PORT_1, &GPIO_InitStructure );
	GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_RST_PIN );

    /*配置SAM卡片选信号*/
    GPIO_InitStructure.GPIO_Pin   = SAM_CSL_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( SAM_GPIO_PORT_2, &GPIO_InitStructure );
	GPIO_ResetBits( SAM_GPIO_PORT_2, SAM_CSL_PIN );

    GPIO_InitStructure.GPIO_Pin   = SAM_CSH_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init( SAM_GPIO_PORT_2, &GPIO_InitStructure );
	GPIO_ResetBits( SAM_GPIO_PORT_2, SAM_CSH_PIN );
    
    USART1_Config();
    USART2_Config();
    
    Set_IRQProcessFunc( USART1_IRQChannel, Usart1_Isr );
    Set_IRQProcessFunc( USART2_IRQChannel, Usart2_Isr );
    Set_IRQProcessFunc( TIM5_IRQChannel, Timer5_Isr );
    
	return 0;
}

/*卸载软、硬件资源*/
int  emv_hard_release( void )
{
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM5, DISABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2, DISABLE );
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, DISABLE );
    
    Reset_IRQProcessFunc( USART1_IRQChannel );
    Reset_IRQProcessFunc( USART2_IRQChannel );
    Reset_IRQProcessFunc( TIM5_IRQChannel );
    
    return 0;
}

/**
 * 对于反向发送情况进行数据处理
 */
static unsigned char scs_inverse_ch( unsigned char ch )
{
    int           i   = 0;
    unsigned char ich = 0;
    
    if( scs_crg & CONV_EN )
    {
        ich = ch ^ 0xFF;
        ch  = 0;
        for( i = 0; i < 8; i++ )
            if( ( ich & ( 1 << i ) ) )ch |= 1 << ( 7 - i );
    }
    
    return ch;
}

/**
 * 检查通道号的合法性
 */
int isvalid_channel( int slot )
{
    slot &= 0x07;

	if(  ( 0 != slot ) 
	   &&( 1 != slot )
	   &&( 2 != slot )
	   &&( 3 != slot )
	   &&( 4 != slot ) 
	  )return 0;
	
	return 1;
}

/**
 * 切换通道号
 */
int select_slot( int slot )
{
    int channel = slot & 0x07;
    
    /*SAM卡硬件切换, 用户卡不用切换*/
    if( USC != channel )
    {
        if( channel == sam_slot_last_flag )return channel;
        
        switch( channel )
        {
        case SAM1:
            GPIO_ResetBits( SAM_GPIO_PORT_2, SAM_CSL_PIN );
            GPIO_ResetBits( SAM_GPIO_PORT_2, SAM_CSH_PIN );
        break;
        case SAM2:
            GPIO_SetBits( SAM_GPIO_PORT_2, SAM_CSL_PIN );
            GPIO_ResetBits( SAM_GPIO_PORT_2, SAM_CSH_PIN );
        break;
        case SAM3:
            GPIO_ResetBits( SAM_GPIO_PORT_2, SAM_CSL_PIN );
            GPIO_SetBits( SAM_GPIO_PORT_2, SAM_CSH_PIN );
        break;
        case SAM4:
            GPIO_SetBits( SAM_GPIO_PORT_2, SAM_CSL_PIN );
            GPIO_SetBits( SAM_GPIO_PORT_2, SAM_CSH_PIN );
        break;
        default:
        break;
        }
        sam_slot_last_flag = channel;
    }

    return channel;
}

/**
 * 电源控制
 */
static int emv_hard_power_signal( int channel, int vcc )
{
    if( USC == channel )
    {
        switch( vcc )
        {
        case 0:
		    GPIO_SetBits( ICC_GPIO_PORT_2, ICC_VCC_PIN );
	    break;
//        case 1800:
//        break;
//        case 3000:
//        break;
//        case 5000:
//        break;
        default:
		    GPIO_ResetBits( ICC_GPIO_PORT_2, ICC_VCC_PIN );
        break;
        }
    }
    else
    {
    	if( vcc )
    	{
    	    if( !sam_vcc_used_flag )
            {
                USART1_Config();/*调试串口与SAM共用USART1配置*/
                GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_VCC_PIN );/*电源未开则打开*/
    	    }
            sam_vcc_used_flag |=  1 << channel;
    	}
    	else 
    	{
    	    /*没有其他通道占用*/
    	    if( !( sam_vcc_used_flag & ( ~( 1 << channel ) ) ) )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_VCC_PIN );/*关闭电源*/
            sam_vcc_used_flag &= ~( 1 << channel );
        }
    }
	
	return 0;
}

/**
 * 异步卡时钟控制
 */
static int emv_hard_clock_enable( int channel, int enable )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
    if( USC == channel )
    {
		if( enable )
		{
		    SC_BIG_UART->CR2 |= 1 << 11;
		    
			GPIO_InitStructure.GPIO_Pin   = ICC_CLK_PIN; 
			GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
			GPIO_Init( ICC_GPIO_PORT_1, &GPIO_InitStructure );
		}
		else
		{
		   SC_BIG_UART->CR2 &= ~( 1 << 11 );
		   ICC_GPIO_PORT_1->ODR  &= ~ICC_CLK_PIN;
		   
		   GPIO_InitStructure.GPIO_Pin   = ICC_CLK_PIN;
		   GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		   GPIO_Init( ICC_GPIO_PORT_1, &GPIO_InitStructure ); 
	    }
    }
    else
    {
    	if( enable )
		{
		    //dbg_msg( 1, "CLK-EN(%d) = %X\r\n", channel, sam_clk_used_flag );
		    if( !sam_clk_used_flag )
		    {
		        SC_SML_UART->CR2 |= 1 << 11;
		    
			    GPIO_InitStructure.GPIO_Pin   = SAM_CLK_PIN; 
			    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
			    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
			    GPIO_Init( SAM_GPIO_PORT_1, &GPIO_InitStructure );
		    }
		    sam_clk_used_flag |= 1 << channel;
		}
		else
		{
		    //dbg_msg( 1, "CLK-DIS(%d) = %X\r\n", channel, sam_clk_used_flag );
		    if( !( sam_clk_used_flag & ( ~( 1 << channel ) ) ) )
		    {
		        SC_SML_UART->CR2 &= ~( 1 << 11 );
		        SAM_GPIO_PORT_1->ODR  &= ~SAM_CLK_PIN;
		   
		        GPIO_InitStructure.GPIO_Pin   = SAM_CLK_PIN;
		        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	            GPIO_Init( SAM_GPIO_PORT_1, &GPIO_InitStructure ); 
	        }
	        sam_clk_used_flag &= ~( 1 << channel );
	    }
	}
    
    return 0;
	
}

/**
 * 复位信号控制
 */
static int emv_hard_reset_signal( int channel, int level )
{
    switch( channel )
    {
    case USC:
        if( level )GPIO_SetBits( ICC_GPIO_PORT_2, ICC_RST_PIN ); /*RST High*/
        else GPIO_ResetBits( ICC_GPIO_PORT_2, ICC_RST_PIN ); /*RST Low*/
    break;
    case SAM1:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST High*/
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST Low*/
    break;
    case SAM2:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST High*/
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST Low*/
    break;
    case SAM3:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST High*/
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST Low*/
    break;
    case SAM4:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST High*/
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_RST_PIN ); /*RST Low*/
    break;
    default:
    break;
    }

    
	return 0;
}

/*数据信号控制*/
static int emv_hard_io_signal( int channel, int level )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    switch( channel )
    {
    case USC:
        GPIO_InitStructure.GPIO_Pin   = ICC_IO_PIN;
	    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	    GPIO_Init( ICC_GPIO_PORT_1, &GPIO_InitStructure );
	    
        if( level )GPIO_SetBits( ICC_GPIO_PORT_1, ICC_IO_PIN );
        else GPIO_ResetBits( ICC_GPIO_PORT_1, ICC_IO_PIN );
        
    break;
    case SAM1:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
    break;
    case SAM2:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
    break;
    case SAM3:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
    break;
    case SAM4:
    	if( level )GPIO_SetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
        else GPIO_ResetBits( SAM_GPIO_PORT_1, SAM_IO_PIN );
    break;
    default:
    break;
    }
    
    return 0;   
}

/**
 * 干扰中断控制
 */
int emv_disturb_interruption( struct emv_core *pdev, int enable )
{
    return 0;
}

/**
 * 智能卡软件工作参数配置
 */
int emv_hard_config( struct emv_core *pdev )
{    
    USART_TOV_Timer( 0, 0 );
    
    scs_dev  = pdev;
    if( USC == pdev->terminal_ch )/*用户卡配置*/
    {	
		USART_ITConfig( SC_BIG_UART, USART_IT_TC, DISABLE );
		USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, DISABLE );
		USART2_ISR_Clr();
		
        emv_core_init( pdev, ( struct emv_queue* )&( scs_queue[ 0 ] ) );
        pdev->queue->pf = INVALID_INDEX;
        pdev->queue->sf = 0;
        
        if( pdev->terminal_conv )scs_crg |= CONV_EN;
        else scs_crg &= ~CONV_EN;
        
		if( 0 == pdev->terminal_ptype )/*T=0*/
		{
			if( 1 == ( pdev->queue->ip - pdev->queue->op ) )
			{		
				USART_SetGuardTime( SC_BIG_UART, 5 );
			}
			else
			{	
				if( pdev->terminal_cgt >15 )USART_SetGuardTime( SC_BIG_UART, pdev->terminal_cgt - 11 );
				else USART_SetGuardTime( SC_BIG_UART, pdev->terminal_cgt - 11 + 5 );
			}			
		}
		else
		{
			USART_SetGuardTime( SC_BIG_UART, pdev->terminal_cgt - 11 );
		}
		
		USART_SmartCardSetBrr( FCLK_CARD / ( pdev->terminal_fi / pdev->terminal_di ) );
		
        scs_rwt  =  pdev->terminal_di * 113;
        scs_etu  =  pdev->terminal_fi / pdev->terminal_di;
		scs_cgt  =  pdev->terminal_cgt;

		if( 1 == pdev->terminal_ptype )/*T=1*/
		{
			scs_bwt  = pdev->terminal_bwt;
			scs_cwt  = pdev->terminal_cwt;			
			scs_crg &= ~CRPT_EN;
			SC_BIG_UART->CR2 &= ~(3<<12);
			SC_BIG_UART->CR2 |=  (1<<12);
		}
		else
		{
			scs_cwt  = pdev->terminal_wwt;			
			scs_bwt  = scs_cwt;	
			scs_crg  |= CRPT_EN;		
			SC_BIG_UART->CR2 &= ~(3<<12);
			SC_BIG_UART->CR2 |=  (3<<12);
		}

		repeat_chc = 4;			
    }
    else/*SAM卡配置*/
    {
		USART_ITConfig( SC_SML_UART, USART_IT_TC, DISABLE );
		USART_ITConfig( SC_SML_UART, USART_IT_RXNE, DISABLE );
		USART1_ISR_Clr();
		
        emv_core_init( pdev, ( struct emv_queue* )&( scs_queue[ 1 ] ) );
        pdev->queue->pf = INVALID_INDEX;
        pdev->queue->sf = 0;
        
        if( pdev->terminal_conv )scs_crg |= CONV_EN;
        else scs_crg &= ~CONV_EN;
        
		if( 0 == pdev->terminal_ptype )
		{
			if( 1 == ( pdev->queue->ip - pdev->queue->op ) )
			{		
				USART_SetGuardTime( SC_SML_UART, 5 );
			}
			else
			{	
				if( pdev->terminal_cgt > 15 )USART_SetGuardTime( SC_SML_UART, pdev->terminal_cgt - 11 );
				else USART_SetGuardTime( SC_SML_UART, pdev->terminal_cgt - 11 + 5 );
			}			
		}
		else
		{
			USART_SetGuardTime( SC_SML_UART, pdev->terminal_cgt - 11 );
		}
		
		USART_SmartCardSetBrr( FCLK_CARD / ( pdev->terminal_fi / pdev->terminal_di ) );
		
        scs_rwt  =  pdev->terminal_di * 113;
        scs_etu  =  pdev->terminal_fi / pdev->terminal_di;
		scs_cgt  =  pdev->terminal_cgt;

		if( 1 == pdev->terminal_ptype )
		{
			scs_bwt  = pdev->terminal_bwt;
			scs_cwt  = pdev->terminal_cwt;			
			scs_crg &= ~CRPT_EN;
			SC_SML_UART->CR2 &= ~(3<<12);
			SC_SML_UART->CR2 |=  (1<<12);
		}
		else
		{
			scs_cwt  = pdev->terminal_wwt;			
			scs_bwt  = scs_cwt;	
			scs_crg  |= CRPT_EN;		
			SC_SML_UART->CR2 &= ~(3<<12);
			SC_SML_UART->CR2 |=  (3<<12);
		}
		 
		repeat_chc = 4;	
    }
    
    return 0;
}

/**
 * 卡片在位检测函数
 */
int emv_hard_detect( struct emv_core *pdev )
{    
    unsigned char det_status;
        
    if( USC == pdev->terminal_ch )
    {  
        det_status = GPIO_ReadInputDataBit( ICC_GPIO_PORT_1, ICC_DET_PIN );
        //dbg_msg( 1, "Detect PIN : %02X\r\n", det_status );
        if( 0 == det_status )return 1;
    }
	else /*目前没有使用*/
	{
		return 1;
	}
	
    return 0;
}

/**
 * 使能通道电源
 */
int emv_hard_power_pump( struct emv_core *pdev )
{
    emv_hard_config( pdev );
    emv_queue_flush( pdev );
	
	emv_hard_power_signal( pdev->terminal_ch, pdev->terminal_vcc );
	
	work_step = SC_UART_IGDT_EVENT;
	wait_counter = 100;
	USART_TOV_Timer( 1, scs_etu );
	while( SC_UART_IGDT_EVENT == work_step );

    return 0;
}

/**
 * 关闭通道电源
 */
int emv_hard_power_dump( struct emv_core *pdev )
{	
    volatile int i;
    
    if( USC != pdev->terminal_ch )
    {
      emv_hard_clock_enable( pdev->terminal_ch, 0 );
      for( i = 0; i < 720; i++ );
    }
    else
    {
        emv_hard_reset_signal( pdev->terminal_ch, 0 );
        for( i = 0; i < 720; i++ );
        
        emv_hard_io_signal( pdev->terminal_ch, 0 );
        for( i = 0; i < 720; i++ );
        
        emv_hard_clock_enable( pdev->terminal_ch, 0 );
        for( i = 0; i < 720; i++ );
    }
    emv_hard_power_signal( pdev->terminal_ch, 0 );
    pdev->terminal_state = EMV_IDLE;
    pdev->terminal_open  = 0;
    for( i = 0; i < 7200; i++ );

    return 0;
}

/**
 * 通道冷复位
 */
int emv_hard_cold_reset( struct emv_core *pdev )
{
    pdev->terminal_state = EMV_COLD_RESET;
    pdev->terminal_fi = pdev->terminal_implict_fi;
    pdev->terminal_di = pdev->terminal_implict_di;
    pdev->terminal_conv  = 0;
    
    if( USC == pdev->terminal_ch )USART2_Config();
    emv_hard_config( pdev );
    scs_crg &= ~CRPT_EN;

    if( 0 == pdev->terminal_spec )/*EMV规范*/
    {
        scs_bwt = 121 * pdev->terminal_di;
    }
    else /*ISO7816-3规范*/
    {
        scs_bwt = ( 121 + 15 )* pdev->terminal_di;
    }

    scs_cwt = 10096 * pdev->terminal_di;
    scs_cgt = 12;
	
    if( 0 == pdev->terminal_spec )
    {
        twt_wait_counter = 20176 * pdev->terminal_di;
        twt_state = TWT_READY;
    }
    else
    {
        twt_state = TWT_STOP;
    }

    scs_stat         =   SC_UART_WAIT_TS | SC_UART_RECV_EV;         	    	
    work_step        =   SC_UART_CLKS_EVENT;                       
    wait_counter     =   5 * pdev->terminal_di;

    emv_hard_clock_enable( pdev->terminal_ch, 0 );	
    USART_TOV_Timer( 1, scs_etu );  
    
    return 0;
}

/**
 * 通道热复位
 */
int emv_hard_warm_reset( struct emv_core *pdev )
{
    pdev->terminal_state = EMV_WARM_RESET;
    pdev->terminal_fi    = pdev->terminal_implict_fi; // restor default parameters 
    pdev->terminal_di    = pdev->terminal_implict_di;
    pdev->terminal_conv  = 0;
	
    emv_hard_config( pdev );
	scs_crg &= ~CRPT_EN;
	
	if( 0 == pdev->terminal_spec )/* ShangHai 2010-8-27 RuanYu */
    {
        scs_bwt = 121 * pdev->terminal_di;
    }
    else
    {
        scs_bwt = ( 121 + 15 )* pdev->terminal_di;
    }
	
    scs_cwt = 10096 * pdev->terminal_di;
	scs_cgt = 12;
	
	if( 0 == pdev->terminal_spec )
    {
        twt_wait_counter = 20176 * pdev->terminal_di;
        twt_state = TWT_READY;
    }
    else
    {
        twt_state = TWT_STOP;
    }
	
    scs_stat     =  SC_UART_WAIT_TS | SC_UART_RECV_EV;
    work_step    =  SC_UART_RSTL_EVENT;
    wait_counter =  5 * pdev->terminal_di;
    USART_TOV_Timer( 1, scs_etu );

    return 0;
}

/**
 * 智能卡数据通信
 */
int emv_hard_xmit( struct emv_core *pdev )
{	
	unsigned char ch;
	
    twt_state    = TWT_STOP; 
	
	if( USC == pdev->terminal_ch )USART2_ISR_Clr();
	else USART1_ISR_Clr();
	
	/*反向保护时间*/
	work_step = SC_UART_IGDT_EVENT;
	wait_counter = 64 * scs_dev->terminal_di;
	USART_TOV_Timer( 1, scs_etu );
	while( SC_UART_IGDT_EVENT == work_step );
	
	/*通信配置*/
    emv_hard_config( pdev );
    
	scs_stat = SC_UART_SEND_EV;
    ch = pdev->queue->qbuf[ pdev->queue->op++ ];
    
    if( USC == pdev->terminal_ch )
    {
    	if( CRPT_EN & scs_crg )USART_SmartCardNACKCmd( SC_BIG_UART, ENABLE ); // Enable the NACK Transmission */
		else USART_SmartCardNACKCmd( SC_BIG_UART, DISABLE ); // Enable the NACK Transmission */ 
		
        SC_BIG_UART->DR   = scs_inverse_ch( ch );	
		
		USART_ITConfig( SC_BIG_UART, USART_IT_RXNE, DISABLE );
		USART_ITConfig( SC_BIG_UART, USART_IT_TC, ENABLE );
    }
    else
    {
    	if( CRPT_EN & scs_crg )USART_SmartCardNACKCmd( SC_SML_UART, ENABLE ); // Enable the NACK Transmission */
		else USART_SmartCardNACKCmd( SC_SML_UART, DISABLE ); // Enable the NACK Transmission */ 
		
        SC_SML_UART->DR   = scs_inverse_ch( ch );	
		
		USART_ITConfig( SC_SML_UART, USART_IT_RXNE, DISABLE );
		USART_ITConfig( SC_SML_UART, USART_IT_TC, ENABLE );	
    }
    
    /*等待发送完成*/
	if( MAX_QBUF_SIZE == pdev->queue->op )pdev->queue->op = 0;
    while( scs_stat & SC_UART_SEND_EV )if( pdev->queue->sf )break;
    
    return 0;
}
#endif