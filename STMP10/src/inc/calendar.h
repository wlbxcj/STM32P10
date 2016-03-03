/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : calendar.h
* Author             : MCD Application Team
* Date First Issued  : 05/21/2007
* Description        : This file contains all the functions prototypes for the
*                      calendar software driver.
********************************************************************************
* History:
* 05/21/2007: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CALENDAR_H
#define __CALENDAR_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
u8 ReadDigit(u8 ColBegin, u8 CountBegin, u8 ValueMax, u8 ValueMin) ;
void Calendar_Init(void);
u32 Time_Regulate(void);
void Time_Adjust(void);
void Time_Show(void);
void Time_Display(u32 TimeVar);
//void Date_Regulate(void);
void Date_Adjust(void);
void Date_Display(u16 nYear, u8 nMonth, u8 nDay);
void Date_Show(void);
void Date_Update(void);
u32 Alarm_Regulate(void);
void Alarm_Adjust(void);
void Alarm_PreAdjust(void);
void Alarm_Display(u32 AlarmVar);
void Alarm_Show(void);
void DateGetYYMMDD(u8 *pDateData);

#endif /* __CALENDAR_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
