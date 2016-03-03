/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : 10/08/2007
* Description        : Main Interrupt Service Routines.
*                      This file can be used to describe all the exceptions
*                      subroutines that may occur within user application.
*                      When an interrupt happens, the software will branch
*                      automatically to the corresponding routine.
*                      The following routines are all empty, user can write code
*                      for exceptions handlers and peripherals IRQ interrupts.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "kf701dh.h"
#include "TaxBox.h"
#include "Display.h"
#include "calendar.h"
#include "RF1356.h"
#include "usb_int.h"
#include "kb.h"  //gplian
#include "dll.h"
#include "lcd.h"

#define s_UartPrint trace_debug_printf //sxl
u16 capture = 0;
extern vu16 CCR1_Val;
extern u8 *pLedData;
extern u8 ActDig;
extern u8 SecondUpdata;

extern void _IC1Interrupt(void);
extern void USB_Istr(void);
extern void Timer5_Isr(void);
extern void _SetLock_EraseApp(void);
extern int TAMPERClear(void);
extern BYTE Lib_KbGetCh(void);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : NMIException
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMIException(void)
{
}

/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFaultException(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
    ledall_off();
    debug_printf(0,0,0,"HardFaultException");
    delay_ms(100);
    ledall_on();
    delay_ms(100);
  }
}

/*******************************************************************************
* Function Name  : MemManageException
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManageException(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
    ledall_off();
    debug_printf(0,0,0,"MemManageException");
    delay_ms(100);
    ledall_on();
    delay_ms(100);
  }
}

/*******************************************************************************
* Function Name  : BusFaultException
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFaultException(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
    ledall_off();
    debug_printf(0,0,0,"BusFaultException");
    delay_ms(100);
    ledall_on();
    delay_ms(100);
  }
}

/*******************************************************************************
* Function Name  : UsageFaultException
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFaultException(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
    ledall_off();
    debug_printf(0,0,0,"UsageFaultException");
    delay_ms(100);
    ledall_on();
    delay_ms(100);
  }
}

/*******************************************************************************
* Function Name  : DebugMonitor
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMonitor(void)
{
}

/*******************************************************************************
* Function Name  : SVCHandler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVCHandler(void)
{
}

/*******************************************************************************
* Function Name  : PendSVC
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSVC(void)
{
}

/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTickHandler(void)
{
    extern bool SysTickReached;
    SysTickReached=TRUE;
    /*stop the system tick counter*/
    /* Disable the SysTick Counter */
    SysTick_CounterCmd(SysTick_Counter_Disable);
    /* Clear the SysTick Counter */
    SysTick_CounterCmd(SysTick_Counter_Clear);
}

/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{
  //unsigned char bVal,sBuf[2048]={0};
  if(BKP_GetITStatus() != RESET)
  { /* Tamper detection event occurred */

#if 0    
        bVal = RM_flash_erase_page(0);
        RM_flash_erase_page(1);
        RM_flash_erase_page(2);
        RM_flash_erase_page(3);
        RM_flash_erase_page(4);
        RM_flash_erase_page(5);
        RM_flash_erase_page(6);
        flash_write_operate(0,(unsigned short*)sBuf,2048);
#endif
        _SetLock_EraseApp();//13/04/20
        
        TAMPERClear();
        
	Lib_LcdCls();
	//Lib_LcdPrintxy(0, 2*8, 0x00, "DEVICE LOCK ONLINE!!![%d]",bVal);
	Lib_LcdPrintxy(0, 2*8, 0x00, "DEVICE LOCKED!!!");
	Lib_KbGetCh();

    //test
    for(;;);
    
    /* Clear Tamper pin interrupt pending bit */
    BKP_ClearITPendingBit();

    /* Clear Tamper pin Event(TE) pending flag */
    BKP_ClearFlag();

    /* Disable Tamper pin */
    BKP_TamperPinCmd(DISABLE);
    
    /* Enable Tamper pin */
    BKP_TamperPinCmd(ENABLE);
      
  }
  
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
  /* If counter is equal to 86339: one day was elapsed */
#if 0  
  if((RTC_GetCounter()/3600 == 23)&&(((RTC_GetCounter()%3600)/60) == 59)&&
     (((RTC_GetCounter()%3600)%60) == 59)) /* 23*3600 + 59*60 + 59 = 86339 */
#endif
  if(RTC_GetCounter()>=86399)    
  {
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    /* Reset counter value */
#if 0    
    RTC_SetCounter(0x0);
#endif
    RTC_SetCounter(RTC_GetCounter()-86399);
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Increment the date */
    Date_Update();
  }
  SecondUpdata = 1;  
  /* Clear the RTC Second Interrupt pending bit */  
  RTC_ClearITPendingBit(RTC_IT_SEC);  
}

/*******************************************************************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void)
{
  
  //for kf311
  /* Clear the EXTI line 8 pending bit */
#if 0
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    { 
        //AT skx
        //extern void PN512_s_RFIsr(void); 
        //PN512_s_RFIsr();//ClearITPendingBit in the PN512_s_RFIsr 
        extern void s_RFIsr(void);
        s_RFIsr();//ClearITPendingBit in the s_RFIsr 
        /* Clear the EXTI line 8 pending bit */
        //EXTI_ClearITPendingBit(EXTI_Line8);
        //RFinterrupt();
    }
#endif  
    
}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
//sxl 13/10/16
void EXTI1_IRQHandler(void)
  {
    if (EXTI_GetITStatus(EXTI_Line1) != RESET)    //EXTI_GetITStatus只有当没有屏蔽和有中断标志这两种都符合条件的时候，才会返回SET
                                                    //但是STM32只有中断标志时也会会进入中断，虽然我们mask了中断，但是仍然会进入而且没有清除中断标志
                                                    //所以会一直进入中断导致死机，解决办法是重新判断一下中断标志，如果存在则清除
   {
    EXTI_ClearITPendingBit(EXTI_Line1);  //EXTI_GetITStatus返回RESET，这里不执行，没有清中断
    _IC1Interrupt();
   }
 
if (EXTI_GetFlagStatus(EXTI_Line1) == SET)  //如果有中断则清除
    EXTI_ClearITPendingBit(EXTI_Line1);

#if 0 
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    { 
        //AT skx
        //extern void PN512_s_RFIsr(void); 
        //PN512_s_RFIsr();//ClearITPendingBit in the PN512_s_RFIsr 
      
#if 0      
        extern void s_RFIsr(void);
        s_RFIsr();//ClearITPendingBit in the s_RFIsr 
#endif
        
        PN512_s_RFIsr();
        /* Clear the EXTI line 8 pending bit */
        //EXTI_ClearITPendingBit(EXTI_Line8);
        //RFinterrupt();
    }
#endif
}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMAChannel1_IRQHandler
* Description    : This function handles DMA Stream 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : DMAChannel2_IRQHandler
* Description    : This function handles DMA Stream 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : DMAChannel3_IRQHandler
* Description    : This function handles DMA Stream 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : DMAChannel4_IRQHandler
* Description    : This function handles DMA Stream 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel4_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : DMAChannel5_IRQHandler
* Description    : This function handles DMA Stream 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : DMAChannel6_IRQHandler
* Description    : This function handles DMA Stream 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMAChannel7_IRQHandler
* Description    : This function handles DMA Stream 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC_IRQHandler
* Description    : This function handles ADC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC1_2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN_TX_IRQHandler(void)
{
    CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN_RX0_IRQHandler(void)
{
    USB_Istr();
}

/*******************************************************************************
* Function Name  : CAN_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_RX1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_SCE_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void EXTI9_5_IRQHandler(void)
{

   unsigned long tmpFlag=0 ;  //保存需要的中断标志位
   //unsigned char i = 0 ;  //循环变量
   //test
   unsigned char sBuf[10];
   
    
   tmpFlag = EXTI->PR & Keyboard_EXTI_Line ;  //只取设定过的标志位
   EXTI->PR = tmpFlag ;

#if 0   
   if(g_IccIntrStatus)
   {
                          //中断禁止
          return;
   }
#endif   
   
   switch(tmpFlag)   //判断是哪个标志位置位
   {
   case Keyboard_EXTI_Row1:
     //GPIO_ResetBits(GPIOC ,Keyboard_Line) ;//disable 12/10/11
     memset(sBuf,0,sizeof(sBuf));
    
     
     //test
     EXTI->PR = Keyboard_EXTI_Row1 ;
   //  GPIO_SetBits(GPIOC ,Keyboard_Line);
#if 0     
     sBuf[0] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_9);
     sBuf[1] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_10);
     sBuf[2] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_11);
     sBuf[3] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_12);
    
     sBuf[0] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_5);
     sBuf[1] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_6);
     sBuf[2] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_7);
     sBuf[3] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_8);
     
 
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8; //-in
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12; //-in
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
 
    //GPIO_Write(GPIOC,0xfE1f);//扫描列值
    GPIO_ResetBits(GPIOC ,GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8) ; 
  
    
     sBuf[0] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_9);
     sBuf[1] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_10);
     sBuf[2] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_11);
     sBuf[3] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_12);
#endif     
     
#if 0     
     for(i=0 ;i<4 ;i++)
     {
        GPIO_SetBits(GPIOC ,Keyboard_Line);
       
        //GPIO_SetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        GPIO_ResetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        //GPIO_SetBits(GPIOC ,Keyboard_Line & (~(Keyboard_LineBase<<i)));
/*        
        //test
     sBuf[0] = GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_1);
     sBuf[1] = GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_2);
     sBuf[2] = GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_3);
     sBuf[3] = GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_4);
       
     sBuf[4] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_9);
     sBuf[5] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_10);
     sBuf[6] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_11);
     sBuf[7] = GPIO_ReadInputDataBit(GPIOC ,GPIO_Pin_12);
*/    
        //if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_1))
        if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_1)==0)
          
        {
          Delay(DELAY_COUNT) ;  //延时消抖
          if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_1)==0)
          {
              Keyboard_Val = 1+i ;
              Keyboard_Change_Flag = 1 ;
              break ;
          }
        }
     }
#endif     
     //GPIO_SetBits(GPIOC ,Keyboard_Line) ;
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     
     EXTI->PR = Keyboard_EXTI_Row1 ;
     break ;
   case Keyboard_EXTI_Row2:
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     EXTI->PR = Keyboard_EXTI_Row2 ;
#if 0     
     for(i=0 ;i<4 ;i++)
     {
        GPIO_SetBits(GPIOC ,Keyboard_Line);
       
        //GPIO_SetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        GPIO_ResetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        //GPIO_SetBits(GPIOC ,Keyboard_Line & (~(Keyboard_LineBase<<i)));
        
        if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_2)==0)
        {
          Delay(DELAY_COUNT) ;  //延时消抖
          if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_2)==0)
          {
              //Keyboard_Val = 6+i ;
              Keyboard_Val = 5+i ;
              Keyboard_Change_Flag = 1 ;
              break ;
          }
        }
     }
     //GPIO_SetBits(GPIOC ,Keyboard_Line) ;
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     EXTI->PR = Keyboard_EXTI_Row2 ; 
#endif     
     break ;
   case Keyboard_EXTI_Row3:
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     EXTI->PR = Keyboard_EXTI_Row3 ;
#if 0     
     for(i=0 ;i<4 ;i++)
     {
        GPIO_SetBits(GPIOC ,Keyboard_Line);
       
        //GPIO_SetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        GPIO_ResetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        //GPIO_SetBits(GPIOC ,Keyboard_Line & (~(Keyboard_LineBase<<i)));
        
        if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_3)==0)
        {
          Delay(DELAY_COUNT) ;  //延时消抖
          if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_3)==0)
          {
              //Keyboard_Val = 11+i ;
              Keyboard_Val = 9+i ;
              Keyboard_Change_Flag = 1 ;
              break ;
          }
        }
     }
     //GPIO_SetBits(GPIOC ,Keyboard_Line) ;
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     
     EXTI->PR = Keyboard_EXTI_Row3 ;
#endif
     
     break ;
   case Keyboard_EXTI_Row4:
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     EXTI->PR = Keyboard_EXTI_Row4 ; 
#if 0     
     for(i=0 ;i<4 ;i++)
     {
        GPIO_SetBits(GPIOC ,Keyboard_Line);
       
        //GPIO_SetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        GPIO_ResetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
        //GPIO_SetBits(GPIOC ,Keyboard_Line & (~(Keyboard_LineBase<<i)));
        
        if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_4)==0)
        {
          Delay(DELAY_COUNT) ;  //延时消抖
          if(GPIO_ReadInputDataBit(GPIOC ,Keyboard_Row_4)==0)
          {
              //Keyboard_Val = 16+i ;
              Keyboard_Val = 13+i ;
              Keyboard_Change_Flag = 1 ;
              break ;
          }
        }
     }
     //GPIO_SetBits(GPIOC ,Keyboard_Line) ;
     GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
     EXTI->PR = Keyboard_EXTI_Row4 ; 
#endif
     
     break ;
   default:
     break ;
   }
   
   if(tmpFlag==Keyboard_EXTI_Row1 ||
      tmpFlag==Keyboard_EXTI_Row2 ||
      tmpFlag==Keyboard_EXTI_Row3 ||
      tmpFlag==Keyboard_EXTI_Row4 
      )
   {
     //trace_debug_printf("Keyboard_EXTI_Row%02x",tmpFlag);
      TimerScanBegin();     
   }
        
#if 0  
  static int test=0;
  /* Clear the EXTI line 8 pending bit */

    if(EXTI_GetITStatus(EXTI_Line8) != RESET)
    { 
        //AT skx
        //extern void PN512_s_RFIsr(void); 
        //PN512_s_RFIsr();//ClearITPendingBit in the PN512_s_RFIsr 
        extern void s_RFIsr(void);
        s_RFIsr();//ClearITPendingBit in the s_RFIsr 
        /* Clear the EXTI line 8 pending bit */
        //EXTI_ClearITPendingBit(EXTI_Line8);
        //RFinterrupt();
    }
#endif   
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern void IT_SendWait(void);
void TIM2_IRQHandler(void)
{
  //u16 DigData;
  
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    /*
    ActDig >>= 1;
    if(ActDig == 0) ActDig = 0x80;
    LedBits_Control(FALSE);
    DigData = GPIO_ReadOutputData(GPIOC); 
    DigData &= 0xFF00;  
    switch(ActDig)
    {
      case 0x80:  DigData |= *(pLedData + 0);   GPIO_Write(GPIOC, DigData); LedBit1_Control(TRUE);  break;
      case 0x40:  DigData |= *(pLedData + 1);   GPIO_Write(GPIOC, DigData); LedBit2_Control(TRUE);  break;
      case 0x20:  DigData |= *(pLedData + 2);   GPIO_Write(GPIOC, DigData); LedBit3_Control(TRUE);  break;
      case 0x10:  DigData |= *(pLedData + 3);   GPIO_Write(GPIOC, DigData); LedBit4_Control(TRUE);  break;
      case 0x08:  DigData |= *(pLedData + 4);   GPIO_Write(GPIOC, DigData); LedBit5_Control(TRUE);  break;
      case 0x04:  DigData |= *(pLedData + 5);   GPIO_Write(GPIOC, DigData); LedBit6_Control(TRUE);  break;
      case 0x02:  DigData |= *(pLedData + 6);   GPIO_Write(GPIOC, DigData); LedBit7_Control(TRUE);  break;
      case 0x01:  DigData |= *(pLedData + 7);   GPIO_Write(GPIOC, DigData); LedBit8_Control(TRUE);  break;
      default : break;
    }
    */
  }
  
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler(void)
{
  //AT skx
  extern void s_TimerProc(void);
  if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    s_TimerProc();
  }
  
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles TIM5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler(void)
{
//#if 0  
  Timer5_Isr();
//#endif  
}

/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void)
{
  if( SPI_GetITStatus(SPI1, SPI_IT_RXNE) != RESET )
  {
    SPI_ClearITPendingBit(SPI1, SPI_IT_RXNE);
  }
  else if( SPI_GetITStatus(SPI1, SPI_IT_OVR) != RESET )
  {
    SPI_ClearITPendingBit(SPI1, SPI_IT_OVR);
  }
  else if(SPI_GetITStatus(SPI1, SPI_IT_MODF) != RESET)//other err 
  {
    SPI_ClearITPendingBit(SPI1, SPI_IT_MODF);
  }
  else if(SPI_GetITStatus(SPI1, SPI_IT_CRCERR) != RESET)//other err 
  {
    SPI_ClearITPendingBit(SPI1, SPI_IT_CRCERR);
  }
  //#ifdef SPI2_DEBUG
  //debug_printf(0,0,0,"SPI_UNKNOW_ERR,SR:%08x,CR2:%08x",Get_Spi2_Status(),Get_Spi2_Cr2());
  //#endif
}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

void SPI2_IRQHandler(void)
{
  /*
  //#include "spiprotocol.H"
  //#define SPI2  SPI1
  if( SPI_GetITStatus(SPI2, SPI_IT_RXNE) != RESET )
  {
    if( spi2_is_send )//send
    {
      //spi2_rx_in=0;
      //spi2_rx_out=0;
      (void)SPI_ReceiveData(SPI2);
      // Loop while DR register in not emplty 
      while(SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET);
      SPI_SendData(SPI2, spi2_tx_buff[spi2_tx_index++]);
      if( spi2_tx_index >= TX_BUFFER_SIZE )spi2_tx_index=0;
      #ifdef SPI2_DEBUG
      debug_printf(0,0,0,"S,%02x,%02x,%08x,%08x",spi2_tx_index,
                   spi2_tx_buff[spi2_tx_index-1],Get_Spi2_Status(),Get_Spi2_Cr2());
      #endif
    }
    else//read
    {
      spi2_rx_buff[spi2_rx_in++]=(u8)SPI_ReceiveData(SPI2);
      if(spi2_rx_in>=RX_BUFFER_SIZE) spi2_rx_in=0;
      //不显式回复数据，默认置换0x00给master
      // Loop while DR register in not emplty 
      //while(SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET);
      //SPI_SendData(SPI2, 0xa5);
      #ifdef SPI2_DEBUG
      debug_printf(0,0,0,"R,%02x,%02x,%02x,%08x,%08x",spi2_rx_in,spi2_rx_out,
                   spi2_rx_buff[spi2_rx_in-1],Get_Spi2_Status(),Get_Spi2_Cr2());
      #endif
    }
    SPI_ClearITPendingBit(SPI2, SPI_IT_RXNE);
  }
  else if( SPI_GetITStatus(SPI2, SPI_IT_OVR) != RESET )
  {
    spi2_overerr_flag=1;
    // Wait to receive a byte 
    while(SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET);
    
    SPI_ReceiveData(SPI2);//read a dirty data
   
    #ifdef SPI2_DEBUG
    debug_printf(0,0,0,"SPI_OVER_ERR,SR:%08x",Get_Spi2_Status());
    #endif
      SPI_ClearITPendingBit(SPI2, SPI_IT_OVR);
  }
  else if(SPI_GetITStatus(SPI2, SPI_IT_MODF) != RESET)//other err 
  {
    spi2_othererr_flag=1;
    //SPI_ClearFlag(SPI2, SPI_FLAG_MODF|SPI_FLAG_BSY|SPI_FLAG_CRCERR);
    
    #ifdef SPI2_DEBUG
    debug_printf(0,0,0,"SPI_MODE_ERR,SR:%08x,CR2:%08x",Get_Spi2_Status(),Get_Spi2_Cr2());
    #endif
    SPI_ClearITPendingBit(SPI2, SPI_IT_MODF);
  }
  else if(SPI_GetITStatus(SPI2, SPI_IT_CRCERR) != RESET)//other err 
  {
    spi2_othererr_flag=1;
    //SPI_ClearFlag(SPI2, SPI_FLAG_MODF|SPI_FLAG_BSY|SPI_FLAG_CRCERR);
    
    #ifdef SPI2_DEBUG
    debug_printf(0,0,0,"SPI_CRC_ERR,SR:%08x",Get_Spi2_Status());
    #endif
    SPI_ClearITPendingBit(SPI2, SPI_IT_CRCERR);
  }
  */
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void)
{
    extern void isr_com1(void);
    isr_com1();  
#if 0  
    /* If a Frame error is signaled by the card */
    if(USART_GetITStatus(USART1, USART_IT_FE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_FE);
        USART_ReceiveData(USART1);

        /* Resend the byte that failed to be received (by the Smartcard) correctly */
        
        //extern void SC_ParityErrorHandler(USART_TypeDef * UARTx);//skx on 20100707
        //SC_ParityErrorHandler(USART1);
    }

    /* If the USART3 detects a parity error */
    if(USART_GetITStatus(USART1, USART_IT_PE) != RESET)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
        {
        }
        /* Clear the USART3 Parity error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_PE);
        USART_ReceiveData(USART1);
    }

    /* If a Overrun error is signaled by the card */
    if(USART_GetITStatus(USART1, USART_IT_ORE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
        USART_ReceiveData(USART1);
    }

    /* If a Noise error is signaled by the card */
    if(USART_GetITStatus(USART1, USART_IT_NE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_NE);
        USART_ReceiveData(USART1);
    }
    
    /* If a error is signaled by the card */
    if(USART_GetITStatus(USART1, USART_IT_ERR) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART1, USART_IT_ERR);
        USART_ReceiveData(USART1);
    }  
#endif    
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{
    extern void isr_com2(void);
    isr_com2();  
  
  
#if 0  
    /* If a Frame error is signaled by the card */
    if(USART_GetITStatus(USART2, USART_IT_FE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART2, USART_IT_FE);
        USART_ReceiveData(USART2);

        /* Resend the byte that failed to be received (by the Smartcard) correctly */
        //extern void SC_ParityErrorHandler(USART_TypeDef * UARTx);//skx on 20100707
        //SC_ParityErrorHandler(USART2);
    }

    /* If the USART3 detects a parity error */
    if(USART_GetITStatus(USART2, USART_IT_PE) != RESET)
    {
        while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET)
        {
        }
        /* Clear the USART3 Parity error pending bit */
        USART_ClearITPendingBit(USART2, USART_IT_PE);
        USART_ReceiveData(USART2);
    }

    /* If a Overrun error is signaled by the card */
    if(USART_GetITStatus(USART2, USART_IT_ORE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART2, USART_IT_ORE);
        USART_ReceiveData(USART2);
    }

    /* If a Noise error is signaled by the card */
    if(USART_GetITStatus(USART2, USART_IT_NE) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART2, USART_IT_NE);
        USART_ReceiveData(USART2);
    }
    
    /* If a error is signaled by the card */
    if(USART_GetITStatus(USART2, USART_IT_ERR) != RESET)
    {
        /* Clear the USART3 Frame error pending bit */
        USART_ClearITPendingBit(USART2, USART_IT_ERR);
        USART_ReceiveData(USART2);
    } 
#endif    
    
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
#if 0  
  Usart3_Isr();
#endif  
  //AT skx
#if 0  
    //void UARTCom_IT(void);
    extern void isr_com3(void);
    isr_com3();
#endif    
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern void IT_PowerOff(void);
void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line10) != RESET)
    {
        /* Clear the EXTI line 10 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line10);
        
        //AT skx
        //AppFlag.bPowerLost = TRUE;//!AppFlag.bPowerLost;
        // IT_PowerOff();
    }
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_BRK_IRQHandler
* Description    : This function handles TIM8 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_UP_IRQHandler
* Description    : This function handles TIM8 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_TRG_COM_IRQHandler
* Description    : This function handles TIM8 Trigger and commutation interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_CC_IRQHandler
* Description    : This function handles TIM8 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC3_IRQHandler
* Description    : This function handles ADC3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : FSMC_IRQHandler
* Description    : This function handles FSMC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
}



/*******************************************************************************
* Function Name  : SPI3_IRQHandler
* Description    : This function handles SPI3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles TIM6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles TIM7 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel1_IRQHandler
* Description    : This function handles DMA2 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel2_IRQHandler
* Description    : This function handles DMA2 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel3_IRQHandler
* Description    : This function handles DMA2 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : This function handles DMA2 Channel 4 and DMA2 Channel 5
*                  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel4_5_IRQHandler(void)
{
}
/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
