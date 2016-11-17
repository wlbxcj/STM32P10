/****************************************Copyright (c)****************************************************
**                               vanstone Co.,LTD
**
**                                 http://www.vanstone.com.cn
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           TSC2046.c
** Last modified Date:  2014-9-17
** Last Version:        V1.0
** Descriptions:        touch
**
**--------------------------------------------------------------------------------------------------------
** Created by:          WXF
** Created date:        2014-9-17
** Version:             V1.0
** Descriptions:        touch
**
**--------------------------------------------------------------------------------------------------------
** Modified by:					WXF
** Modified date:				2014-9-17
** Version:
** Descriptions:
**
** Rechecked by:
*********************************************************************************************************/
#include "stm32f10x_lib.h"
#include "tsc2046.h"
#include "comm.h"
#include "RF1356.h"

#define TSC2046_GetCounts	10

#define TSC3026_SECLECT()       GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define TSC3026_UNSECLECT()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define	 TSC2046_CMD_CHX 	((unsigned char)0x90)	//1001 0000 ��ַ�ʽ��ȡXλ��
#define	 TSC2046_CMD_CHY	((unsigned char)0xD0)	//1101 0000 ��ַ�ʽ��ȡYλ��

void Timer_Delay(unsigned int count)
{
    volatile unsigned int i,j;

    i = count;
    for(j = 0;j < 3000;j++)
        for(;i > 0;i--);
}

void TSC2046_init(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4; 
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TSC3026_UNSECLECT();  
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
/*    
    //Connect EXTI Line8 to PB0 
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);   

    //EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
*/
}

#if 0
void Timer_T2Init(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* TIM4  enable */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* TIM4 configuration */
  TIM_TimeBaseStructure.TIM_Period = 6000;		//1ms
  TIM_TimeBaseStructure.TIM_Prescaler = 3;			// Ԥ��Ƶ24M/4=6M
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM2,ENABLE);  

  TIM_Cmd(TIM2, DISABLE);  
  /* Enable TIM2 update interrupt */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}
#endif

unsigned char TSC2046_GetBusyStatus(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12);
}

unsigned char TSC2046_GetIRQStatus(void)
{
    return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11);
}

/* ���PENIRQ����Ч�Ҷ�ȡ��������������Χ���򷵻�1�����򷵻�0 */
unsigned char TSC2046_Read_Single(unsigned short *X_Position, unsigned short *Y_Position)
{	
	unsigned short	X1,Y1;
    unsigned short	x1,x2,y1,y2;
	unsigned char 	t,t1,count=0;
	unsigned short 	databuffer[2][TSC2046_GetCounts];//������
	unsigned short	temp=0;	 
	
    while( (!TSC2046_GetIRQStatus()) && count<TSC2046_GetCounts )//ѭ������XPT2046_GetCounts��
	{	
		if(TSC2046_Read_Position(&databuffer[0][count],&databuffer[1][count]))
		{	  
			count++;	/* �����ɹ��ż��� */  
		}
	} 

	if(count== TSC2046_GetCounts)//һ��Ҫ����XPT2046_GetCounts������,������
	{
//	    while(!TSC2046_GetIRQStatus());	//������ȴ������ͷ�
		do//������X��������
		{	
			t1=0;		  
			for(t=0;t<count-1;t++)
			{
				if( databuffer[0][t]> databuffer[0][t+1] )//��������
				{
					temp=databuffer[0][t+1];
					databuffer[0][t+1]=databuffer[0][t];
					databuffer[0][t]=temp;
					t1=1; 
				}  
			}
		}while(t1); 
			  
		do//������Y��������
		{	
			t1=0;		 
			for(t=0;t<count-1;t++)
			{
				if(databuffer[1][t]>databuffer[1][t+1])//��������
				{
					temp=databuffer[1][t+1];
					databuffer[1][t+1]=databuffer[1][t];
					databuffer[1][t]=temp;
					t1=1;	 
				}  
			}
		}while(t1);

		x1=databuffer[0][TSC2046_GetCounts/2-1]; x2=databuffer[0][TSC2046_GetCounts/2]; //x3=databuffer[0][8];		
		y1=databuffer[1][TSC2046_GetCounts/2-1]; y2=databuffer[1][TSC2046_GetCounts/2]; //y3=databuffer[1][8];
			   
		if( ((x1>x2)&&(x1>x2+30)) || ((x2>x1)&&(x2>x1+30)) || ((y1>y2)&&(y1>y2+30)) || ((y2>y1)&&(y2>y1+30)) )
			return 0;	/* ������ɢ��̫�󣬶��� */	 		  
		else
		{
			X1=(databuffer[0][TSC2046_GetCounts/2-1]+databuffer[0][TSC2046_GetCounts/2])/2;
			Y1=(databuffer[1][TSC2046_GetCounts/2-1]+databuffer[1][TSC2046_GetCounts/2])/2;	
			if(X1<=4096&&Y1<=4096) //12λ�������ֵ
			{	
				(*X_Position)=X1;
			    (*Y_Position)=Y1;
			    trace_debug_printf("\nX1=%d,Y1=%d\n",X1,Y1);
				return 1;
			}
			return 0;	/* �������ֵ����4096������ */	
		}   
	}//if(count== GetCounts)
	return 0;	/* �ɼ����ݴ������㣬���� */
}  

/* ����һ���ֽڵ�XPT2046����XPT2046��ȡһ���ֽ� */
static unsigned char TSC2046_ReadByte(unsigned char value)
{
	return (SPIDataSendReceive(value));
}

/* �ȼ�⵽�����жϺ���ô˺�����ȡλ�ã�������ȷ����1�����򷵻�0 */
unsigned char TSC2046_Read_Position(unsigned short *X_Position, unsigned short *Y_Position)
{
    unsigned short temp;

    TSC3026_SECLECT();
    //delay_ms(1);
    SPIDataSendReceive( TSC2046_CMD_CHY );
    //delay_ms(1);
//    while(TSC2046_GetBusyStatus());	//�ȴ�busy�źű��
    Timer_Delay(5);
    temp = TSC2046_ReadByte(0x00);
    Timer_Delay(5);
//    trace_debug_printf("\n*temp1 = %x***\n",temp);
    temp <<= 8;    
//    trace_debug_printf("\n*temp2 = %x***\n",temp);
    temp |= TSC2046_ReadByte(0x00);    
//    trace_debug_printf("\n*temp3 = %x***\n",temp);
    (*Y_Position) = temp >> 3;
    (*Y_Position) &= 0x0FFF;
	TSC3026_UNSECLECT();
//    trace_debug_printf("\n*Y_Position = %d***\n",*Y_Position);
    #if 1
	TSC3026_SECLECT();
	//delay_ms(1);
	Timer_Delay(5);
    SPIDataSendReceive( TSC2046_CMD_CHX ) ;
    Timer_Delay(5);
//    while(TSC2046_GetBusyStatus());	//�ȴ�busy�źű��
    //delay_ms(1);
    temp = TSC2046_ReadByte(0x00) ;
//    trace_debug_printf("\n*tempx= %d***\n",temp);
    temp <<= 8;
    temp |= TSC2046_ReadByte(0x00) ;
    (*X_Position) = temp >> 3;
    (*X_Position) &= 0x0FFF;
    TSC3026_UNSECLECT();
//    trace_debug_printf("\n*X_Position = %d***\n",*X_Position);
	/* ����һ����Χ������ڷ�Χ֮�ڣ�����1����Ȼ����0 */
	if((*X_Position) > 100 && (*Y_Position)> 100 && (*X_Position) < 4000 && (*Y_Position) < 4000)
	    return 1;//�����ɹ�(��Χ����)
	else
		return 0;
	#endif
}
unsigned short TSC2046_Read_Y(void)
{
    unsigned short yPos = 0, Temp = 0, Temp0 = 0, Temp1 = 0;

    /* Select the TP: Chip Select low */
    TSC3026_SECLECT();
//    Timer_Delay(1);
    /* Send Read xPos command */
    SPIDataSendReceive( TSC2046_CMD_CHY );
//    Timer_Delay(5);
    /* Read a byte from the TP */
    Temp0 = TSC2046_ReadByte(0x00);
    /* Read a byte from the TP */
    Temp1 = TSC2046_ReadByte(0x00);  
//	Timer_Delay(5);
    /* Deselect the TP: Chip Select high */
    TSC3026_UNSECLECT();
    Temp = (Temp0 << 8) | Temp1; 
    yPos = Temp>>3;
    
    return yPos;
}
unsigned short TSC2046_Read_X(void)
{
    unsigned short xPos = 0, Temp = 0, Temp0 = 0, Temp1 = 0;
    
    /* Select the TP: Chip Select low */
    TSC3026_SECLECT();
    Timer_Delay(5);
    /* Send Read xPos command */
    SPIDataSendReceive( TSC2046_CMD_CHX );
//    Timer_Delay(5);
    /* Read a byte from the TP */
    Temp0 = TSC2046_ReadByte(0x00);
    /* Read a byte from the TP */
    Temp1 = TSC2046_ReadByte(0x00);  
//    Timer_Delay(5);
    /* Deselect the TP: Chip Select high */
    TSC3026_UNSECLECT();
    Temp = (Temp0 << 8) | Temp1; 
    xPos = Temp>>3;
        
    return xPos;
}
/********************************************************************************************************
                                           END OF FILE
*********************************************************************************************************/

