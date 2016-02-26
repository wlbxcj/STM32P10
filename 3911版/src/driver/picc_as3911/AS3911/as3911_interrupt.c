/*
 *****************************************************************************
 * Copyright @ 2009 by austriamicrosystems AG                                *
 * All rights are reserved.                                                  *
 *                                                                           *
 * Reproduction in whole or in part is prohibited without the written consent*
 * of the copyright owner. Austriamicrosystems reserves the right to make    *
 * changes without notice at any time. The software is provided as is and    *
 * Austriamicrosystems makes no warranty, expressed, implied or statutory,   *
 * including but not limited to any implied warranty of merchantability or   *
 * fitness for any particular purpose, or that the use will not infringe any *
 * third party patent, copyright or trademark. Austriamicrosystems should    *
 * not be liable for any loss or damage arising from its use.                *
 *****************************************************************************
 */

/*
 * PROJECT: AS3911 firmware
 * $Revision: $
 * LANGUAGE: ANSI C
 */

/*! \file as3911_interrupt.c
 *
 * \author Oliver Regenfelder
 *
 * \brief AS3911 interrupt handling and ISR
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/

#include "as3911_interrupt.h"
#include "as3911_irq.h"
#include "as3911.h"
//#include "timer_driver.h"

/*
******************************************************************************
* DEFINES
******************************************************************************
*/

#define s_UartPrint trace_debug_printf 
/*
******************************************************************************
* MACROS
******************************************************************************
*/

/*!
 *****************************************************************************
 * \brief Clear the interrupt flag associated with the as3911 interrupt.
 *****************************************************************************
 */
//#define AS3911_IRQ_CLR() { _IC1IF = 0; }

/*!
 *****************************************************************************
 * \brief Evaluates to true if there is a pending interrupt request from the
 * AS3911.
 *****************************************************************************
 */
//#define AS3911_IRQ_IS_SET() ( _RB9 != 0) 
//#define AS3911_IRQ_ON()  EXTI_EnableITBit(EXTI_Line6)
//#define AS3911_IRQ_OFF() EXTI_DisableITBit(EXTI_Line6)

/*
******************************************************************************
* LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/*! AS3911 interrutp mask. */
static volatile u32 as3911InterruptMask = 0;
/*! Accumulated AS3911 interrupt status. */
static volatile u32 as3911InterruptStatus = 0;

/*
******************************************************************************
* LOCAL TABLES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL VARIABLE DEFINITIONS
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
//sxl
/*
unsigned long SysTick_Get()
{
  return RTC_GetCounter(); 
}

unsigned char SysTick_Timeout(unsigned long after)
{
  return (((long)SysTick_Get() - (long)after) < 0) ? 0 : 1;
  
}
*/

s8 as3911EnableInterrupts(u32 mask)
{
    s8 error = ERR_NONE;
    u32 irqMask = 0;

    AS3911_IRQ_OFF();

    error |= as3911ContinuousRead(AS3911_REG_IRQ_MASK_MAIN, (u8*) &irqMask, 3);
    irqMask &= ~mask;
    as3911InterruptMask |= mask;
    error |= as3911ContinuousWrite(AS3911_REG_IRQ_MASK_MAIN, (u8*) &irqMask, 3);

    AS3911_IRQ_ON();

    if (ERR_NONE == error)
        return ERR_NONE;
    else
        return ERR_IO;
}

s8 as3911DisableInterrupts(u32 mask)
{
    s8 error = ERR_NONE;
    u32 irqMask = 0;

    AS3911_IRQ_OFF();

    error |= as3911ContinuousRead(AS3911_REG_IRQ_MASK_MAIN, (u8*) &irqMask, 3);
    irqMask |= mask;
    as3911InterruptMask &=  ~mask;
    error |= as3911ContinuousWrite(AS3911_REG_IRQ_MASK_MAIN, (u8*) &irqMask, 3);

    AS3911_IRQ_ON();

    if (ERR_NONE == error)
        return ERR_NONE;
    else
        return ERR_IO;
}

s8 as3911ClearInterrupts(u32 mask)
{
    s8 error = ERR_NONE;
    u32 irqStatus = 0;

    AS3911_IRQ_OFF();

    error |= as3911ContinuousRead(AS3911_REG_IRQ_MAIN, (u8*) &irqStatus, 3);
	//LOG("as3911ClearInterrupts irqStatus = 0x%x", irqStatus);
    as3911InterruptStatus |= irqStatus & as3911InterruptMask;
    as3911InterruptStatus &= ~mask;

    AS3911_IRQ_ON();

    if (ERR_NONE == error)
        return ERR_NONE;
    else
        return ERR_IO;
}

s8 as3911WaitForInterruptTimed(u32 mask, u16 timeout, u32 *irqs)
{
    bool_t timerExpired = FALSE;
    u32 irqStatus = 0;
    u32 tmpirqStatus = 0;//sxl
	//ulong after_tick;
    unsigned int uiBeginTime = GetTimerCount();
    if (timeout > 0){
      GetTimerCount(timeout/10);   //13/10/17 sxl
		//TimerSet(4, timeout); 
		//SetTimer(1, timeout);
        //timerStart(timeout);
       // after_tick = SysTick_Get() + timeout*6;//10 MS
    }
 //sxl    
    do
    {
      irqStatus = as3911InterruptStatus & mask;
       


      if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
      {
     
      AS3911_IRQ_OFF();
      as3911ContinuousRead(AS3911_REG_IRQ_MAIN, (u8*) &tmpirqStatus, 3);
      as3911InterruptStatus |= tmpirqStatus & as3911InterruptMask;
      irqStatus = as3911InterruptStatus & mask;
      AS3911_IRQ_ON();
      //s_UartPrint("as3911ContinuousRead=%d\r\n",as3911ContinuousRead);//sxl
      //s_UartPrint("as3911InterruptStatus = %d\r\n", as3911InterruptStatus);//sxl
      //s_UartPrint("tmpirqStatus = %d\r\n", tmpirqStatus);//sxl
      //s_UartPrint("as3911InterruptMask = %d\r\n", as3911InterruptMask);//sxl
      
      }
  
 

        if (timeout > 0)
        {
            //if (!timerIsRunning())
            //if(!TimerCheck(4)) //13/10/16 sxl
            //if(!CheckTimer(1))
          if(( GetTimerCount()-uiBeginTime )>(60))   //sxl

            //if(SysTick_Timeout(after_tick))
	    {
                timerExpired = TRUE;
	    }
        }
    //s_UartPrint("GetTimerCount=%d,uiBeginTime=%d",GetTimerCount(),uiBeginTime);
    //s_UartPrint("tmpirqStatus = %d, as3911WaitForInterruptTimed irqStatus = 0x%x, mask = 0x%x, timerExpired = %d\r\n",
    //            tmpirqStatus,irqStatus, mask, timerExpired); //sxl
    } while (!irqStatus && !timerExpired);
   // s_UartPrint("O"); //sxl
#if 0
    s_UartPrint("timerExpired=%d\r\n",timerExpired);
    if (timerExpired)
    {
   
    as3911ContinuousRead(AS3911_REG_IRQ_MAIN, (u8*) &irqStatus, 3);
    s_UartPrint("!!!!!!!!!!irqStatus = 0x%x\r\n", irqStatus);
    as3911ContinuousRead(AS3911_REG_IRQ_MAIN, (u8*) &irqStatus, 3);
    s_UartPrint("!!!!!!!!!!irqStatus = 0x%x\r\n", irqStatus);
    
    }
#endif
    AS3911_IRQ_OFF();
    as3911InterruptStatus &= ~irqStatus;
    AS3911_IRQ_ON();

    *irqs = irqStatus;
	LOG("as3911WaitForInterruptTimed:mask = 0x%x, irqStatus = 0x%x\r\n", mask, irqStatus); 
       // s_UartPrint("irqStatus2 = %d\r\n", irqStatus);//sxl
         
    return ERR_NONE;
}

s8 as3911GetInterrupts(u32 mask, u32 *irqs)
{
  
    AS3911_IRQ_OFF();
    //as3911ContinuousRead(AS3911_REG_IRQ_MAIN, (u8*) &as3911InterruptStatus, 3); //sxl
    *irqs = as3911InterruptStatus & mask;
    as3911InterruptStatus &= ~mask;

    AS3911_IRQ_ON();
    //sxl 13/9/17
   if (EXTI_GetFlagStatus(EXTI_Line1) == SET)
   EXTI_ClearITPendingBit(EXTI_Line1);
   
    return ERR_NONE;
}

//void __attribute__((interrupt, no_auto_psv)) _IC1Interrupt(void)
// void as3911Isr(void)
void _IC1Interrupt(void)
{
    do
    {
        u32 irqStatus = 0;

        AS3911_IRQ_CLR();

        as3911ContinuousRead(AS3911_REG_IRQ_MAIN, (u8*) &irqStatus, 3);
        as3911InterruptStatus |= irqStatus & as3911InterruptMask;
    } while (AS3911_IRQ_IS_SET());
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
