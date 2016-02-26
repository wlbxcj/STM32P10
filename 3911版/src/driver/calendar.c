/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : calendar.c
* Author             : MCD Application Team
* Date First Issued  : 05/21/2007
* Description        : This file includes the calendar driver for the 
*                      STM32F10x-EVAL demonstration.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "calendar.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Time Structure definition */
struct time_t
{
  u8 sec_l;
  u8 sec_h;
  u8 min_l;
  u8 min_h;
  u8 hour_l;
  u8 hour_h;
};
struct time_t time_struct;

/* Alarm Structure definition */
struct alarm_t
{
  u8 sec_l;
  u8 sec_h;
  u8 min_l;
  u8 min_h;
  u8 hour_l;
  u8 hour_h;
};
struct alarm_t alarm_struct;

/* Date Structure definition */
struct date_t
{
  u8 month;
  u8 day;
  u16 year;
};
struct date_t date_s;

u8 SecondUpdata = 0;
//static u32 wn = 0, dn = 0, dc = 0;
u8 MonLen[12]= {32, 29, 32, 31, 32, 31, 32, 32, 31, 32, 31, 32};
u8 MonthNames[12][20] =
        {"  JANUARY           ", "  FEBRUARY          ", "  MARCH             ",
         "  APRIL             ", "  MAY               ", "  JUNE              ",
         "  JULY              ", "  AUGUST            ", "  SEPTEMBER         ",
         "  OCTOBER           ", "  NOVEMBER          ", "  DECEMBER          "};

/* Private function prototypes -----------------------------------------------*/
static u8 IsLeapYear(u16 nYear);
//static void WeekDayNum(u32 nyear, u8 nmonth, u8 nday);
static void RTC_Configuration(void);
static void Date_PreAdjust(void);


void ChangeDate(u8 *pDateData);

/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : Calendar_Init
* Description    : Initializes calendar application.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Calendar_Init(void)
{
  u32 i = 0, tmp = 0;
  
  if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  {
    /* RTC Configuration */
    RTC_Configuration();
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();  
    /* Change the current time */
    RTC_SetCounter(Time_Regulate());

    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);    
    /* Adjust Date */
    Date_PreAdjust();        
  }
  else
  {
    /* PWR and BKP clocks selection ------------------------------------------*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
    
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
  
    /* Enable the RTC Second */  
    RTC_ITConfig(RTC_IT_SEC, ENABLE);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
    
    /* Initialize Date structure */
    date_s.month = (BKP_ReadBackupRegister(BKP_DR3) & 0xFF00) >> 8;
    date_s.day = (BKP_ReadBackupRegister(BKP_DR3) & 0x00FF);
    date_s.year = BKP_ReadBackupRegister(BKP_DR2);

    if(RTC_GetCounter() / 86399 != 0)
    {
      for(i = 0; i < (RTC_GetCounter() / 86399); i++)
      {
        Date_Update();
      }
      RTC_SetCounter(RTC_GetCounter() % 86399);
      BKP_WriteBackupRegister(BKP_DR2, date_s.year);
      tmp = date_s.month << 8;
      tmp |= date_s.day; 
      BKP_WriteBackupRegister(BKP_DR3, tmp);
    }
  }
}

/*******************************************************************************
* Function Name  : Time_Regulate
* Description    : Returns the time entered by user, using menu vavigation keys.
* Input          : None
* Output         : None
* Return         : Current time RTC counter value
*******************************************************************************/
u32 Time_Regulate(void)
{
  u32 Tmp_HH = 0, Tmp_MM = 0, Tmp_SS = 0;

    time_struct.hour_h = 0x00;
    time_struct.hour_l = 0x00;    
    time_struct.min_h = 0x00;    
    time_struct.min_l = 0x00;    
    time_struct.sec_h = 0x00;    
    time_struct.sec_l = 0x00;

   Tmp_HH = time_struct.hour_h * 10 + time_struct.hour_l;
   Tmp_MM = time_struct.min_h * 10 + time_struct.min_l;
   Tmp_SS = time_struct.sec_h * 10 + time_struct.sec_l;
   
  /* Return the value to store in RTC counter register */
  return((Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS));
}


/*******************************************************************************
* Function Name  : Date_PreAdjust
* Description    : Pre-Adjusts the current date (MM/DD/YYYY).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Date_PreAdjust(void)
{
  u32 tmp = 0;


  if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
  {
    //Warning
  }
  else
  {
    /* Change the current date */
  /* Initialize Date structure */
    date_s.year = 2010;        
    date_s.month = 05;
    date_s.day = 05;

    BKP_WriteBackupRegister(BKP_DR2, date_s.year);
    tmp = date_s.month << 8;
    tmp |= date_s.day; 
    BKP_WriteBackupRegister(BKP_DR3, tmp);
  }
}

/*******************************************************************************
* Function Name  : Date_Update
* Description    : Updates date when time is 23:59:59.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Date_Update(void)
{
  
  unsigned char sBuf[10];
  
  //13/02/20 备用电池供电
  //if( (date_s.month==0) && (date_s.day ==0) && (date_s.year==0))
  if(PWR_GetFlagStatus(PWR_FLAG_SB)==RESET)
  //  if(RCC_GetFlagStatus(RCC_FLAG_PORRST)==RESET)
  {
    date_s.month = (BKP_ReadBackupRegister(BKP_DR3) & 0xFF00) >> 8;
    date_s.day = (BKP_ReadBackupRegister(BKP_DR3) & 0x00FF);
    date_s.year = BKP_ReadBackupRegister(BKP_DR2);
    
  }
       
  if(date_s.month == 1 || date_s.month == 3 || date_s.month == 5 || date_s.month == 7 ||
     date_s.month == 8 || date_s.month == 10 || date_s.month == 12)
  {
    if(date_s.day < 31)
    {
      date_s.day++;
    }
    /* Date structure member: date_s.day = 31 */
    else
    {
      if(date_s.month != 12)
      {
        date_s.month++;
        date_s.day = 1;
      }
      /* Date structure member: date_s.day = 31 & date_s.month =12 */
      else
      {
        date_s.month = 1;
        date_s.day = 1;
        date_s.year++;
      }
    }
  }
  else if(date_s.month == 4 || date_s.month == 6 || date_s.month == 9 ||
          date_s.month == 11)
  {
    if(date_s.day < 30)
    {
      date_s.day++;
    }
    /* Date structure member: date_s.day = 30 */
    else
    {
      date_s.month++;
      date_s.day = 1;
    }
  }
  else if(date_s.month == 2)
  {
    if(date_s.day < 28)
    {
      date_s.day++;
    }
    else if(date_s.day == 28)
    {
      /* Leap year */
      if(IsLeapYear(date_s.year))
      {
        date_s.day++;
      }
      else
      {
        date_s.month++;
        date_s.day = 1;
      }
    }
    else if(date_s.day == 29)
    {
      date_s.month++;
      date_s.day = 1;
    }
  }
  
  //13/02/20
  sBuf[0] = 0x20;
  sBuf[1] = (((date_s.year % 100)/10)<<4) | (date_s.year%10);
  sBuf[2] = ((date_s.month/10)<<4) | (date_s.month%10);
  sBuf[3] = ((date_s.day/10)<<4) | (date_s.day%10);
  ChangeDate(sBuf);
  
}


/*******************************************************************************
* Function Name  : WeekDayNum
* Description    : Determines the week number, the day number and the week day 
*                  number.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
/*
static void WeekDayNum(u32 nyear, u8 nmonth, u8 nday)
{
  u32 a = 0, b = 0, c = 0, s = 0, e = 0, f = 0, g = 0, d = 0;
  s32 n = 0;

  if(nmonth < 3)
  {
    a = nyear - 1;
  }
  else
  {
    a = nyear;
  }
  
  b = (a/4) - (a/100) + (a/400);
  c = ((a - 1)/4) - ((a - 1)/100) + ((a - 1)/400);
  s = b - c;

  if(nmonth < 3)
  {
    e = 0;
    f =  nday - 1 + 31 * (nmonth - 1);
  }
  else
  {
    e = s + 1;
    f = nday + (153*(nmonth - 3) + 2)/5 + 58 + s; 
  }

  g = (a + b) % 7;
  d = (f + g - e) % 7;
  n = f + 3 - d;

  if (n < 0)
  {
    wn = 53 - ((g - s)/5);
  }
  else if (n > (364 + s))
  {
    wn = 1;
  }
  else
  {
    wn = (n/7) + 1;
  }

  dn = d;
  dc = f + 1;
}
*/
/*******************************************************************************
* Function Name  : IsLeapYear
* Description    : Check whether the passed year is Leap or not.
* Input          : None
* Output         : None
* Return         : 1: leap year
*                  0: not leap year
*******************************************************************************/
static u8 IsLeapYear(u16 nYear)
{
  if(nYear % 4 != 0) return 0;
  if(nYear % 100 != 0) return 1;
  return (u8)(nYear % 400 == 0);
}

/*******************************************************************************
* Function Name  : Alarm_Regulate
* Description    : Returns the alarm time entered by user, using demoboard keys.
* Input          : None
* Output         : None
* Return         : Alarm time value to be loaded in RTC alarm register.
*******************************************************************************/
u32 Alarm_Regulate(void)
{
return(0);
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None.
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_Configuration(void)
{
  int i;
  /* PWR and BKP clocks selection --------------------------------------------*/
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);

  /* Reset Backup Domain */
  BKP_DeInit();

  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);
#if 0  
  /* Wait till LSE is ready */
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
  }
#endif  
  //for RTC fail 13/01/21
  //for(;;)
  for(i=0;i<0x1ffff;i++)
  {
    if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
      break;
  }
  
  
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait for RTC registers synchronization */
  RTC_WaitForSynchro();

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  
  /* Enable the RTC Second */  
  RTC_ITConfig(RTC_IT_SEC, ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
  
  /* Set RTC prescaler: set RTC period to 1sec */
  RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
  
  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForLastTask();
}

void ChangeDate(u8 *pDateData)
{
  u32 tmp = 0;
    
    date_s.year = (*pDateData/0x10)*1000 + (*pDateData%0x10)*100 + (*(pDateData+1)/0x10)*10 + *(pDateData+1)%0x10;    
    date_s.month = (*(pDateData+2)/0x10)*10 + *(pDateData+2)%0x10;  
    date_s.day = (*(pDateData+3)/0x10)*10 + *(pDateData+3)%0x10;  
    
#if 0
    #ifdef KF311_M
		return;
	#endif
#endif                
    BKP_WriteBackupRegister(BKP_DR2, date_s.year);
    tmp = date_s.month << 8;
    tmp |= date_s.day; 
    BKP_WriteBackupRegister(BKP_DR3, tmp);  
}

void ChangeTime(u8 *pTimeData)
{
  u32 Tmp_HH = 0, Tmp_MM = 0, Tmp_SS = 0;

    time_struct.hour_h = *pTimeData/0x10;
    time_struct.hour_l = *pTimeData%0x10;    
    time_struct.min_h = *(pTimeData+1)/0x10;    
    time_struct.min_l = *(pTimeData+1)%0x10;     
    time_struct.sec_h = *(pTimeData+2)/0x10;    
    time_struct.sec_l = *(pTimeData+2)%0x10; 



    Tmp_HH = time_struct.hour_h * 10 + time_struct.hour_l;
    Tmp_MM = time_struct.min_h * 10 + time_struct.min_l;
    Tmp_SS = time_struct.sec_h * 10 + time_struct.sec_l;
  
    /* Wait until last write operation on RTC registers has finished */
	#ifdef KF322_M
    RTC_WaitForLastTask();  
	#endif
	
    /* Change the current time */
    RTC_SetCounter(Tmp_HH*3600 + Tmp_MM*60 + Tmp_SS);
     
    //Lib_DelayMs(2);
    RTC_WaitForLastTask(); //12/11/08

    //test121108
    //trace_debug_printf("change rtc[%d]",RTC_GetCounter());
    
}



/*******************************************************************************
* Function Name  : TimeGet
* Description    : Displays the current time.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u8 TimeGet(u8 *pTimeData)
{
  u32 TimeVar;
  
  if(SecondUpdata)
  {
    TimeVar = RTC_GetCounter();
    *(pTimeData + 0) = 0x00;
    *(pTimeData + 1) = 0x00;    
  /* Display time hours */
    *(pTimeData + 2) =(u8)( TimeVar / 3600)/10;
    *(pTimeData + 3) =(u8)(((TimeVar)/3600)%10);
  /* Display time minutes */
    *(pTimeData + 4) =(u8)(((TimeVar)%3600)/60)/10;
    *(pTimeData + 5) =(u8)(((TimeVar)%3600)/60)%10;
  /* Display time seconds */
    *(pTimeData + 6) =(u8)(((TimeVar)%3600)%60)/10;
    *(pTimeData + 7) =(u8)(((TimeVar)%3600)%60)%10;
    
    SecondUpdata = 0;    
    return(1);
  }

  return(0);
}

void DateGet(u8 *pDateData)
{
  /* Display date year */
    *(pDateData + 0) = date_s.year/1000;
    *(pDateData + 1) = (date_s.year % 1000)/100;
    *(pDateData + 2) = (date_s.year % 100)/10;
    *(pDateData + 3) = date_s.year%10;
  /* Display date month */
    *(pDateData + 4) =date_s.month/10;
    *(pDateData + 5) =date_s.month%10;
  /* Display date day */
    *(pDateData + 6) = date_s.day/10;
    *(pDateData + 7) = date_s.day%10;
}

void DateGetYYMMDD(u8 *pDateData)
{
  
   *(pDateData + 0) = date_s.year%100;
   *(pDateData + 1) = date_s.month;
   *(pDateData + 2) = date_s.day;
   
}
/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
