/***************************
*
*for system timer use timer4
*
***************************/
#include "stm32f10x_lib.h"
#include "KF701DH.h"
//#include "KF701DV.h"
#include "Toolkit.h"
#include "SysTimer.h"

/*
设置4个软定时器，三个事件定时
定时器：0-3为应用软件使用
//定时器：4-6为应用定时事件
定时器：4-8为应用定时事件

定时器：0-3 系统内部
定时器：4-8 应用
定时器: 9-11 内部事件
*/

#define  MAX_USER_TIMER  (4+5)
#define  MAX_USER_EVENT  3
//#define  MAX_TIMER       7
#define  MAX_TIMER       (9+3)
static unsigned long TimerCount[MAX_TIMER]={0};
static unsigned long OldCount[MAX_USER_EVENT]={0};
static void (*TimerProc[MAX_USER_EVENT])(void);

void Tim3Init(void);
void s_TimerProc(void);
void TimerSet(unsigned char TimerID, unsigned long Cnts);
unsigned long TimerCheck(unsigned char TimerID);
int SetTimerEvent(unsigned short uElapse, void (*TimerFunc)(void));

void KillTimerEvent(int handle);

//static int  k_Timer4StartFlag;
volatile unsigned int   k_Timer4Count10MS;

extern unsigned char     k_LcdBackLightMode;
extern int               k_LcdBackLightTime;

//AT skx
/* ---------------------------------------------------------------
  TIM2 Configuration:  Timing Mode:
  TIM3CLK = 36*2 MHz, Prescaler = 0x48, TIM3 counter clock = 1 MHz 
--------------------------------------------------------------- */   
void Tim3Init(void)
{
      TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
     //Time base configuration
      TIM_Cmd(TIM3, DISABLE);
      TIM_TimeBaseStructure.TIM_Period = 1000*10;//10MS a time         
      TIM_TimeBaseStructure.TIM_Prescaler = 0x00;       
      TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
      TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
      
      TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    
      //Prescaler configuration
      TIM_PrescalerConfig(TIM3, 0x48, TIM_PSCReloadMode_Update);
    
      //TIM IT enable
      TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    
      //清timer4各中断标志
      TIM_ClearITPendingBit(TIM3, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4 | TIM_IT_Trigger);
      //  enable TIM4
      TIM_Cmd(TIM3, ENABLE);
      //k_Timer4StartFlag=1;
}

// Cnts：单位 SYSTICK=10MS
void TimerSet(unsigned char TimerID, unsigned long Cnts)
{
     TimerCount[TimerID%MAX_TIMER]=Cnts;
     //OpenTimer4();
}

unsigned long TimerCheck(unsigned char TimerID)
{
    if(!TimerCount[TimerID%MAX_TIMER])
        return 0;
    if(TimerCount[TimerID%MAX_TIMER]>=10)
      return (unsigned short)(TimerCount[TimerID%MAX_TIMER]/10);
    return 1;
}
 


int SetTimerEvent(unsigned short uElapse, void (*TimerFunc)(void))
{
    int i;
	
    if(TimerProc[0])
    {
        if(TimerProc[1])
        {
	    if(TimerProc[2])
            return -1;
	    else
	    {
		i=2;
	    }
			
        }
	else
	{
	    i=1;
	}
        
    }
    else{
        i=0;
    }
        
    TimerProc[i]=TimerFunc;
    OldCount[i]=uElapse;
    TimerSet(i+MAX_USER_TIMER,(unsigned long)uElapse);
    return i;
}

void KillTimerEvent(int handle)
{
    if(handle > MAX_USER_EVENT)
        return;
    if(handle<0)
        return;
    TimerProc[handle]=NULL;
    TimerCount[handle+MAX_USER_TIMER]=0;
    OldCount[handle]=0;
}


unsigned int GetTimerCount(void)
{
    return k_Timer4Count10MS;
}

/***************************************************************************
 * 系统定时器的处理函数
 ***************************************************************************/ 
extern unsigned char LcdDatex,LcdDatey,LcdTimex,LcdTimey;
extern unsigned char ShowDateFlag,ShowTimeFlag;
extern unsigned int ShowTimeDly,ShowDtaeDly;
extern unsigned char DisplayArr[];
extern unsigned char DisplayType;
extern unsigned char BeepFlag;
extern unsigned short BeepDelay;
void s_TimerProc(void)
{
    int i = 0, j = 0;
    
    k_Timer4Count10MS++;
    
    for(i=0;i<MAX_TIMER;i++)
    {
        if(TimerCount[i]){
            TimerCount[i]--;
            if((i>=MAX_USER_TIMER)&&(!TimerCount[i])){
                if((TimerProc[i-MAX_USER_TIMER]!=NULL))
                {
                   
                    TimerProc[i-MAX_USER_TIMER]();
                  
                    if(OldCount[i-MAX_USER_TIMER]!=0){
                      
//#if 0   //产生 Lib_CheckTimer！=0
                        TimerCount[i]=OldCount[i-MAX_USER_TIMER];
//#endif                        
                        
                    }
                    else{
                        TimerCount[i]=0;
                        TimerProc[i-MAX_USER_TIMER]=NULL;
                    }
                }
            }
            j=1;
        }
    }
    
    //实现beep定时
    BeepDelay--;
    if(BeepFlag && !BeepDelay )
    {
      Buzzer_Off();
      BeepFlag=0;
    }
    
    if(k_LcdBackLightMode==1)
	{  //delay 1min light
        if(k_LcdBackLightTime==0) Lib_LcdSetBackLight(0);
        if(k_LcdBackLightTime>0)
	{
            k_LcdBackLightTime--;
        }
        k_LcdBackLightMode=1;
    }
    
    /*
    //实现自刷新时间
    #define LEDTYPE                0x01
    #define LCDTYPE                0x00
    ShowDtaeDly--;
    ShowTimeDly--;
    if(ShowTimeFlag && !ShowTimeDly)
    {
      if(DisplayType==LCDTYPE)
      {
        Lcdstring_On(LcdTimex,LcdTimey,&DisplayArr[11]);
      }
      else
      {
        Ledarray_On( &DisplayArr[0] );
      }
      ShowTimeDly=99;//one second show a time
    }
    if(ShowDateFlag && !ShowDtaeDly )
    {
      if(DisplayType==LCDTYPE)
      {
        Lcdstring_On(LcdDatex,LcdDatey,&DisplayArr[0]);
      }
      else
      {
        Ledarray_On( &DisplayArr[0] );
      }
      ShowTimeDly=100*60*60;//one hour show a time
    }
    */
    
    //clear int in stm32f10x_it.c
  
  //test only
  /*
  static int flag=0;
  
  if(flag%2)
    GPIOC->BSRR = GPIO_Pin_13;
  else
    GPIOC->BRR = GPIO_Pin_13;
  flag++;
  //debug_printf(0,0,0,"sys timer in:%d",flag);
  */
}

void   s_SetTimer(uchar TimerNo, ulong count)
{
  TimerSet(TimerNo,count);
}



ulong  s_CheckTimer(uchar TimerNo)
{
    return TimerCheck(TimerNo);
}

//test fun 
static int handle=-1;
void timer_isr(void)
{
  static int flag=0;
  
  if(flag%2)
    GPIOC->BSRR = GPIO_Pin_13;
  else
    GPIOC->BRR = GPIO_Pin_13;
  flag++;
  debug_printf(0,0,0,"set time event:%d",flag);
  if(flag==0x20)  
  {
    debug_printf(0,0,0,"kill time event:%d",flag);
    KillTimerEvent( handle );
  }
  
}
void systimer_test(char cmd)
{
  
  handle=SetTimerEvent(30, timer_isr);
  if(handle)debug_printf(0,0,0,"set time event,handle:%d",handle);
  
  TimerSet(0, 1000/10);
  TimerSet(1, 2000/10);
  TimerSet(2, 3000/10);
  TimerSet(3, 4000/10);
  while(1)
  {
    if( !TimerCheck( 0) ){debug_printf(0,0,0,"Timer 0 timeout:1 S"); TimerSet(0, 5000/10);}
    if( !TimerCheck( 1) ){debug_printf(0,0,0,"Timer 1 timeout:2 S");TimerSet(1, 5000/10);}
    if( !TimerCheck( 2) ){debug_printf(0,0,0,"Timer 2 timeout:3 S");TimerSet(2, 5000/10);}
    if( !TimerCheck( 3) )
    {
      debug_printf(0,0,0,"Timer 3 timeout:4 S");
      break;
    }
  }
  debug_printf(0,0,0,"systimer_test finish.");
  while(1);
  
}

void   Lib_SetTimer(uchar TimerNo, unsigned short Cnt100ms)
{
  uchar bTimerNo;
  bTimerNo =  TimerNo%5; 
  
  TimerSet(bTimerNo+4,10*Cnt100ms);//4-8 app  10ms
  //void TimerSet(unsigned char TimerID, unsigned short Cnts)
}

unsigned short Lib_CheckTimer(uchar TimerNo)
{
  uchar bTimerNo;
  bTimerNo =  TimerNo%5; 
  return TimerCheck( bTimerNo+4);
  
}

void  Lib_StopTimer(uchar TimerNo)
{
  uchar bTimerNo;
  bTimerNo =  TimerNo%5; 
  
    TimerCount[(bTimerNo+4)%MAX_TIMER]=0;
}