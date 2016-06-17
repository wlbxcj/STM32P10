#include "KF701DH.h"
#include "stm32f10x_lib.h"
#include "..\..\inc\FunctionList.h"

#include  "kb.h"
#include  "base.h"
#include  "comm.h"
#include  "vosapi.h"
#include "buzzer.h"

//static  const DWORD BeepFreqTab[8]={0,493,453,403,378,328,278,253};
static  const DWORD  BeepFreqTab[8]={0,100,200,400,800,1600,2000,2500}; 

extern BYTE k_LcdBackLightMode;
#if 0
static  const BYTE Key_Tab[26]={
	KEYCANCEL, KEYCLEAR, KEYF2,    KEYF1, 
	KEY0,      KEY3,     KEY2,     KEY1,
	KEYENTER,  KEY6,     KEY5,     KEY4,
	0,         KEY9,     KEY8,     KEY7,      
	0,         0,        0,        0,  
	0,         0,        0,        0,
	0,         0
	};
#endif
static  const BYTE Key_Tab[26]={
	KEYF1, KEYF2, KEYCLEAR ,    KEYCANCEL, 
	KEY1,      KEY2,     KEY3,     KEY0,
	KEY4,  KEY5,     KEY6,     KEYENTER,
	KEY7,         KEY8,     KEY9,     KEY7,      
	0,         0,        0,        0,  
	0,         0,        0,        0,
	0,         0
	};

#define  KB_BUF_MAX      32
volatile static BYTE kb_buf[KB_BUF_MAX]; /* 读取键值缓冲区 */
volatile static BYTE kb_in_ptr;          /* 读入地址指针   */
volatile static BYTE kb_out_ptr;         /* 读出地址指针   */
volatile static BYTE kb_buf_over;        // 键值缓冲溢出错误，0表示未溢出，1表示溢出
volatile static BYTE kb_backcode;

volatile static BYTE kb_KeyCode1;        // 按键索引号
volatile static BYTE kb_KeyCode2;        // 按键索引号
volatile static int  kb_TimerScanHandle; // 保存按键扫描定时器句柄
volatile static int  kb_TimerStep;       // 保存定时器扫描步骤，可取值0,1,2
volatile static int  kb_OffTimerCount;
volatile static DWORD  kb_SoundFrequency;/*锋鸣器的频率*/
volatile static int  kb_SoundDelay;      /*锋鸣器响声的延时*/
//volatile static int  kb_SoundTempDelay;
volatile static int  kb_BeepIntBusy;
//volatile static int  kb_BeepFunBusy;
volatile static int  kb_Buffer_Busy; // 是否正在清缓冲，0表示否，1表示正在清缓冲

volatile int  g_KbIntrStatus;//用于指示当前按键中断处理的状态：0＝空闲

int g_iKbStopSoundFlag = 0;  // 按键声音禁止标志，1禁止，0不禁止
int g_iAuthForNumberKeyFlag=0;   //=1 auth success  =0  auth error or no auth
int g_iAppCallGetChFlag=0;       //=1 calling app   =0  calling vos

extern BYTE  g_byStartX, g_byStartY;
extern BYTE     g_byFontType;
extern BYTE     g_byFontAttr;
DWORD g_dwPreDisplayLen = 0;

//extern FONT_ATTR g_sFontAttr;
volatile static int g_iKeyScanFlag = 0;  // 按键扫描标志 0无扫描，1在扫描

void   s_TimerScanKey(void);
BYTE   kb_ScanKey(int mode);


static void Enable_KB_irq(void)
{
    EXTI_InitTypeDef EXTI_InitStructure ;
    EXTI_InitStructure.EXTI_Line = Keyboard_EXTI_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt ;
    //EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising ;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling ;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE ;
    EXTI_Init(&EXTI_InitStructure) ;
  
  
}

static void Disable_KB_irq(void)
{
    EXTI_InitTypeDef EXTI_InitStructure ;
    EXTI_InitStructure.EXTI_Line = Keyboard_EXTI_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt ;
    //EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising ;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling ;
    EXTI_InitStructure.EXTI_LineCmd = DISABLE ;
    EXTI_Init(&EXTI_InitStructure) ;
  
  
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int s_KbInit(void)
{
    int i;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    EXTI_DeInit();
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8; //-in
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    //GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    
    EXTI_ClearITPendingBit(EXTI_Line5) ;   //清除中断标志位
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC ,GPIO_PinSource5 ); //Pinsource不能取或
    EXTI_ClearITPendingBit(EXTI_Line6) ;   //清除中断标志位
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC ,GPIO_PinSource6 ); 
    EXTI_ClearITPendingBit(EXTI_Line7) ;   //清除中断标志位
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC ,GPIO_PinSource7 );
    EXTI_ClearITPendingBit(EXTI_Line8) ;   //清除中断标志位
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC ,GPIO_PinSource8 );

    //设置Row1-Row4为下降沿中断
    Enable_KB_irq();
    
    
    
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12; //-out
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    //GPIO_SetBits(GPIOC,GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12);
    //GPIO_SetBits(GPIOC,GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12);
    GPIO_ResetBits(GPIOC ,GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12) ; 
  
    //初始化按键的值
    for(i=0;i<KB_BUF_MAX;i++)
    {
        kb_buf[i]=0xff;
    }

    kb_in_ptr = 0;
    kb_out_ptr = 0;
    kb_buf_over = 0;
    kb_backcode = 0;

    kb_KeyCode1 = NOKEY;
    kb_KeyCode2 = NOKEY;


    kb_TimerScanHandle = 0;
    //kb_TimerStep = 0;
    kb_OffTimerCount = 0;

    //kb_SoundFrequency = BeepFreqTab[4];
    kb_SoundFrequency = BeepFreqTab[7];
    kb_SoundDelay = 1;
    //kb_SoundTempDelay = 1;
    kb_BeepIntBusy = 0;
    //kb_BeepFunBusy = 0;
    kb_Buffer_Busy = 0;

    g_iKbStopSoundFlag = 0;
    g_iAuthForNumberKeyFlag=0;
    g_iAppCallGetChFlag=0;

    g_KbIntrStatus=0; 
    return 0;
}

void TimerScanBegin()
{
     Disable_KB_irq();
     kb_TimerStep = 0;
     kb_OffTimerCount = 0;
     g_KbIntrStatus = 1;
     g_iKeyScanFlag = 1;
     //add 12/11/16
     KillTimerEvent(kb_TimerScanHandle);
     kb_TimerScanHandle = SetTimerEvent(2, s_TimerScanKey); // 每隔20ms扫描一次
  
}

void LocBeep()
{
extern unsigned char BeepFlag;
extern unsigned short BeepDelay;

#if 0
    //test
       Lib_LcdPrintxy(0,0,0,"kb_SoundDelay[%d][%d]",kb_SoundDelay,kb_SoundFrequency);
        Lib_KbGetCh();
#endif        
    if(kb_SoundDelay==0)
    {
      Buzzer_Off();
      BeepFlag=0;
      
      
    }
  //Buzzer_Ring(kb_SoundFrequency); 
    //if(kb_SoundDelay)
    if(kb_SoundDelay &&  (BeepFlag==0) )
    {
      //Buzzer_Init(kb_SoundFrequency+600);
      //Buzzer_Init(1100);
      //Buzzer_Init(900);
      //Buzzer_Init(3200);
      Buzzer_Init(kb_SoundFrequency+500);
      
      //Buzzer_Init(nFrequency);
      if(kb_SoundDelay<7)
        BeepDelay = 7;  //7=
      else  
        BeepDelay = kb_SoundDelay;  //7->kb_SoundDelay*7->kb_SoundDelay
      BeepFlag = 1;
      //delay_ms(70);
      //TIM_Cmd(TIM4, DISABLE); //
    }
  
}

/****************************************************************************
  函数名     :  void s_TimerScanKey(void)
  描述       :  按键响应处理函数，有按键时该函数被调用，没有按键该函数不会被调用。
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void s_TimerScanKey(void)
{
    unsigned char ucTemp1, ucTemp2 = 0;
    //g_iKeyScanFlag = 1;
    //s_UartPrint(COM_DEBUG,"t1");
    switch(kb_TimerStep)
    {
    case 0:
        kb_KeyCode1=kb_ScanKey(1);
        if(kb_KeyCode1==NOKEY)
        {
            kb_OffTimerCount++;
            if (kb_OffTimerCount > 3)//3
            {
                kb_OffTimerCount = 0;
                kb_TimerStep = 2;
            }
        }
        else
        {
            kb_TimerStep++;
        }
        break;
    case 1:
        ucTemp1 = kb_backcode;
        ucTemp2 = kb_KeyCode1;
        //if(kb_backcode!=kb_KeyCode1)
        if (ucTemp1 != ucTemp2)
        {
            kb_KeyCode2=kb_KeyCode1;
        }
        else
        {
            kb_KeyCode2=kb_ScanKey(1);
        }

        ucTemp1 = kb_KeyCode2;
        ucTemp2 = kb_KeyCode1;
        //if(kb_KeyCode2==kb_KeyCode1)
        if (ucTemp1 == ucTemp2)
        {
            if(kb_KeyCode2<20)  // 按键索引号
            {
                /*if(Key_Tab[kb_KeyCode2]==KEYPRINTERUP)
                {
                    s_ManualFeedPaper(1);
                }
                else */if((kb_buf_over==0)&&(kb_Buffer_Busy==0))
                {
                    ucTemp1 = Key_Tab[kb_KeyCode2];
                    //kb_buf[kb_in_ptr]=Key_Tab[kb_KeyCode2];
                    kb_buf[kb_in_ptr] = ucTemp1;
                    kb_in_ptr++;
                    if(kb_in_ptr>=KB_BUF_MAX)
                        kb_in_ptr=0;

                    ucTemp1 = kb_in_ptr;
                    ucTemp2 = kb_out_ptr;
                    //if(kb_in_ptr==kb_out_ptr)
                    if(ucTemp1 == ucTemp2)
                    {
                        kb_buf_over=1;
                    }
                }
                kb_backcode=kb_KeyCode2;
            }
            
            if(k_LcdBackLightMode==1)
              Lib_LcdSetBackLight(1);
            
            //s_LcdBackLightRestart();
            //Lib_LcdSetBackLight(1);
            //s_TimerKeySound(0);
            LocBeep();
#if 0            
            Lib_KbSound(0,0);
#endif            
        }
        else
        {
            kb_KeyCode1=kb_KeyCode2;
        }
        kb_TimerStep++;
        break;
    case 2:
#if 0
        LocBeep();
#endif        
        //s_TimerKeySound(1);
#if 0        
        Lib_KbSound(1,1);
#endif        
        kb_KeyCode2 = kb_ScanKey(0);
        if(kb_KeyCode2 == NOKEY)
        {
            kb_OffTimerCount++;
            if (kb_OffTimerCount > 1)
            {
                kb_OffTimerCount = 0;
#if 0                
                LocBeep();
#endif                
                //s_TimerKeySound(2);
#if 0                
                Lib_KbSound(2,2);
#endif                
                KillTimerEvent(kb_TimerScanHandle);
#if 0
                s_KillTimerEvent(2);
                s_KillTimerEvent(3);
#endif                
                g_iKeyScanFlag = 0;
                //s_UartPrint(COM_DEBUG,"t");
                //s_ManualFeedPaper(0);
                kb_TimerStep = 0;
                //AT91_write32(AT91C_PIOC_CODR, KB_OUT_MASK);
                GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
                //EXTI_EnableITBit
                Enable_KB_irq();
#if 0                
                AT91_read32(AT91C_PIOC_ISR);
                AT91_write32(AT91C_AIC_ICCR, (1<<INTR_ID_PIOC));
                AT91_write32(AT91C_PIOC_IER, KB_IN_MASK);  //enable pio input interupt
#endif                
                g_KbIntrStatus=0;
            }
        }
        
        break;
    default:
        break;
    }
    //g_iKeyScanFlag = 0;
}


/****************************************************************************
  函数名     :  BYTE kb_ScanKey(int mode)
  描述       :  扫描按键
  输入参数   :  1、int mode：扫描模式，
                     0：只扫描有无按键，不管按键值
                   非0：表示扫描并返回按键的键值索引号
  输出参数   :  无
  返回值     :  mode=0时：
                      0：有按键；0xff：无按键；
                mode!=0时：
                      有按键时返回0~19，返回的是按键的行列索引号
                      无按键时返回0xff
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
BYTE kb_ScanKey(int mode)
{
    DWORD temp,temp1,temp2,ch1,ch2;//,ch;
    DWORD i,j,k,n;
    if(mode==0)
    {
        //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
        GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
        
        for(j=0;j<3;j++)
        {
            for(i=0;i<100;i++);
            //for(i=0;i<1000;i++);
            //ch1=AT91_read32(AT91C_PIOC_PDSR) & KB_IN_MASK;
            ch1=GPIO_ReadInputData(GPIOC) & KB_IN_MASK;
            
            for(i=0;i<100;i++);
            //for(i=0;i<1000;i++);
            //ch2=AT91_read32(AT91C_PIOC_PDSR) & KB_IN_MASK;
            ch2=GPIO_ReadInputData(GPIOC) & KB_IN_MASK;
            
            if(ch1==ch2)
                break;
            ch1=KB_IN_MASK;

        }
        if(ch1==KB_IN_MASK)
            return NOKEY;
        else
            return 0x00;
    }
    else
    {
        //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
        GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
        
        for(j=0;j<3;j++)
        {
            for(i=0;i<100;i++);
            //for(i=0;i<1000;i++);
            //ch1=AT91_read32(AT91C_PIOC_PDSR) & KB_IN_MASK;
            ch1=GPIO_ReadInputData(GPIOC) & KB_IN_MASK;
            for(i=0;i<100;i++);
            //for(i=0;i<1000;i++);
            //ch2=AT91_read32(AT91C_PIOC_PDSR) & KB_IN_MASK;
            ch2=GPIO_ReadInputData(GPIOC) & KB_IN_MASK;
            
            if(ch1==ch2)
                break;
            ch1=KB_IN_MASK;

        }
        if(ch1==KB_IN_MASK)
            return NOKEY;

        //有按键*/
        for(i=0;i<4;i++)    //行
        {
            //AT91_write32(AT91C_PIOC_SODR,KB_OUT_MASK);
            GPIO_SetBits(GPIOC ,Keyboard_Line);
            
            //AT91_write32(AT91C_PIOC_CODR,(1<<(i+19)));
            GPIO_ResetBits(GPIOC ,(Keyboard_LineBase<<i)) ;
            
            for(n=0;n<10;n++)
            {
                for(k=0;k<100;k++);//200
                //for(k=0;k<0xFFFF;k++);//200
                //temp1=AT91_read32(AT91C_PIOC_PDSR) & KB_IN_MASK;
                temp1=GPIO_ReadInputData(GPIOC) & KB_IN_MASK;
                
                for(k=0;k<100;k++);//200
                //for(k=0;k<0xffff;k++);//200
                //temp2=AT91_read32(AT91C_PIOC_PDSR) & KB_IN_MASK;
                temp2=GPIO_ReadInputData(GPIOC) & KB_IN_MASK;
                
                if(temp1==temp2)
                    break;
            }
            temp=temp1;

            if(temp!=KB_IN_MASK)
            {
                if(( temp & Keyboard_Row_1 )==0)
                {
                    //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
                    GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
                    
                    return (i*4+3);
                }
                if(( temp & Keyboard_Row_2 )==0)
                {
                    //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
                    GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
                    
                    return (i*4+2);
                }
                if(( temp & Keyboard_Row_3 )==0)
                {
                    //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
                    GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
                    
                    return (i*4+1);
                }
                if(( temp & Keyboard_Row_4 )==0)
                {
                    //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
                    GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
                    
                    return (i*4);
                }
            }
        }

        //AT91_write32(AT91C_PIOC_CODR,KB_OUT_MASK);
        GPIO_ResetBits(GPIOC ,Keyboard_Line) ;
        
        return NOKEY;
    }
}


/****************************************************************************
  函数名     :  void kb_sound(BYTE mode, DWORD dlytime)
  描述       :  控制在按键时蜂鸣器按指定的频率和间隔发声。当mode取值大于7时,
                该函数会以mode%8为发声频率。
  输入参数   :  1、BYTE mode - 频率选择，可以是0~6的值
                   1    最低频率。
                   7    最高频率。
                   0    不发声
                2、DWORD dlytime - 间隔时间(单位：ms)（0~65535）
                   0xffff---按下后一直响，松开时才停止
                   0x0000----不发声
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Lib_KbSound(BYTE mode,WORD DlyTimeMs)
{
    BYTE freq;
    while(kb_BeepIntBusy);
    freq=mode%8;
    if(freq==0)
        kb_SoundDelay=0;
    else
    {
        if(DlyTimeMs==0)
            kb_SoundDelay=0;
        //else if(DlyTimeMs<20)
        else if(DlyTimeMs<10)
            kb_SoundDelay=1;
        else if(DlyTimeMs==0xffff)
            kb_SoundDelay=DlyTimeMs;
        else
            //kb_SoundDelay=DlyTimeMs/20;
            kb_SoundDelay=DlyTimeMs/10;
    }
    kb_SoundFrequency=BeepFreqTab[freq];
    
    return 0;
}

/****************************************************************************
  函数名     :  BYTE Lib_KbGetCh(void)
  描述       :  从键盘缓冲区中读取一键值,无键时等待,不显示到屏幕上
  输入参数   :  无
  输出参数   :  无
  返回值     :  返回取得的键值代码
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
BYTE Lib_KbGetCh(void)
{
    BYTE kbcode;
    BYTE ucTemp1, ucTemp2 = 0;

    for(;;)
    {
        ucTemp1 = kb_in_ptr;
        ucTemp2 = kb_out_ptr;
        //while((kb_in_ptr==kb_out_ptr)&&(kb_buf_over==0))
        while((ucTemp1 == ucTemp2)&&(kb_buf_over==0))
        {
            ucTemp1 = kb_in_ptr;
            ucTemp2 = kb_out_ptr;
//            if(k_ICC_CardInSert || k_MCR_BrushFlag)
//             {
//                 k_ICC_CardInSert=0;
//                 k_MCR_BrushFlag=0;
// 				if(k_LcdBackLightMode == 1)
// 					Lib_LcdSetBackLight(1);
//             }
         }
        kb_buf_over = 0;
        if (kb_buf[kb_out_ptr] != NOKEY)
        {
            kbcode = kb_buf[kb_out_ptr];
            kb_buf[kb_out_ptr] = NOKEY;
            kb_out_ptr++;
            if (kb_out_ptr >= KB_BUF_MAX)
                kb_out_ptr = 0;
                
            
            //if((0 == g_iAuthForNumberKeyFlag) && (1 == g_iAppCallGetChFlag))
            //{
            //    if(kbcode>=0x30 && kbcode<=0x39) kbcode='*';
            //    //s_UartPrint(COM_DEBUG,"ch=*\r\n");
            //}
            
            //s_UartPrint(COM_DEBUG,"ch=%02x\r\n",kbcode);
            return kbcode;
        }
        kb_out_ptr++;
        if (kb_out_ptr >= KB_BUF_MAX)
            kb_out_ptr = 0;
    }  
  
}

/****************************************************************************
  函数名     :  BYTE Lib_KbUnGetCh(void)
  描述       :  返回当前键盘缓冲的第一个字符内容,无键立即返回
  输入参数   :  无
  输出参数   :  无
  返回值     :  返回取得的键值代码
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
BYTE Lib_KbUnGetCh(void)
{
    BYTE ucTemp1, ucTemp2 = 0;

    ucTemp1 = kb_in_ptr;
    ucTemp2 = kb_out_ptr;
    //if((kb_in_ptr==kb_out_ptr)&&(kb_buf_over==0))
    if((ucTemp1 == ucTemp2)&&(kb_buf_over==0))
    {
        return NOKEY;
    }

    return kb_buf[kb_out_ptr];
}

/****************************************************************************
  函数名     :
  描述       :  检测键值缓冲区中是否有未读取的键值
  输入参数   :
  输出参数   :
  返回值     :  -1000------无键值
                0X00------有键值
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int Lib_KbCheck(void)
{
	//Lib_DelayMs(1);
    if(kb_buf[kb_out_ptr]!=NOKEY)
        return 0x00;
    return KB_NoKey_Err;
}

/****************************************************************************
  函数名     :
  描述       :  清键值缓冲区，并清按键板中的缓冲区主CPU发送清键命令给键盘板
                CPU。须注意中断引起指针乱
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void Lib_KbFlush(void)
{
    int i;
    kb_Buffer_Busy=1;
    for(i=0;i<KB_BUF_MAX;i++)
        kb_buf[i]=0xff;
    kb_in_ptr=0;
    kb_out_ptr=0;
    kb_buf_over=0;
    kb_Buffer_Busy=0;
}

/****************************************************************************
  函数名     :
  描述       :  设置按键板在按键时是否发声
  输入参数   :  BYTE mode=0;不发声
                         =1;发声
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
int  Lib_KbMute(BYTE mode)
{
    if(mode==0)
        (void)Lib_KbSound(0,0);
    else
        (void)Lib_KbSound(4,20);
    return 0;
}


const BYTE g_abyVisualAsciiTable[] =
{"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.*, #:;+-=?$&%!~@^()|/_[]{}<>`\'\"\\"};

const BYTE g_abyKeyTableUpper[9][6]={"1.,","2ABC","3DEF","4GHI","5JKL","6MNO","7PQRS","8TUV","9WXYZ"};
const BYTE g_abyKeyTableLower[9][6]={"1.,","2abc","3def","4ghi","5jkl","6mno","7pqrs","8tuv","9wxyz"};
const BYTE g_abyKeyIndexLen[9]={3,4,4,4,4,4,5,4,5};
const BYTE g_abyKeySymbol[33]="0\\* #:;+-=?$&%!~@^()|/_[]{}<>`\'\"";


/****************************************************************************
  函数名     :  void s_DisplayPwd(BYTE byLen, BYTE byMode, BYTE byDisplayLine)
  描述       :  按指定格式显示密码
  输入参数   :  1、BYTE byLen：要显示的密码字符长度。
                   大字最多只能显示15个字符，超过15个字符按15个字符显示；
                   小字最多只能显示20个字符，超过20个字符按20个字符显示。
                2、BYTE byMode：显示格式，
                      bit6：1大写，0小写
                      bit2：1左对齐，0右对齐
                      bit0：1正显示，0反显示
                3、BYTE byDisplayLine：显示所在列号
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-03  V1.0         创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
void s_DisplayPwd(BYTE x, BYTE y, BYTE byMode, BYTE byLen, BYTE byClearLastFlag)
{
    BYTE i, byStartX, byWidth, byDisplayCut;
    int iTemp;

    if (0 != (byMode&IS_FONT_WIDTH_8))  // 字符大小
    {
        byWidth = 8;
        if (byLen > 15)
        {
            byLen = 15;
        }
        (void)Lib_LcdSetFont(8, 16, 0);
//        Lib_LcdClrLine(byDisplayLine, byDisplayLine+15);
    }
    else
    {
        byWidth = 6;
        if (byLen > 20)
        {
            byLen = 20;
        }
        (void)Lib_LcdSetFont(8, 16, 0);
//        Lib_LcdClrLine(byDisplayLine, byDisplayLine+7);
    }

    if (0 != (byMode&IS_FLUSH_LEFT))
    { // 左对齐
        byStartX = x;
        if (0 != (byMode&IS_FONT_WIDTH_8))
        {
            byDisplayCut = (byStartX+7)/8;
            if ((byLen+byDisplayCut) > 15)
            {
                byLen = 15-byDisplayCut;
            }
        }
        else
        {
            byDisplayCut = (byStartX+5)/6;
            if ((byLen+byDisplayCut) > 20)
            {
                byLen = 20-byDisplayCut;
            }
        }
    }
    else
    { // 右对齐
        byStartX = 128-(byLen+1)*byWidth;
    }

    if (0 != byClearLastFlag)
    {
        iTemp = byStartX;
        iTemp -= byWidth;
        if ((0==(byMode&IS_FLUSH_LEFT)) && (iTemp>=0))
        {
            (void)Lib_LcdSetAttr(0);
            Lib_LcdGotoxy(iTemp, y);
            (void)Lib_Lcdprintf(" ");
        }
    }
    if (0 == (byMode&DISPLAY_NOT_REVER))
    {
        (void)Lib_LcdSetAttr(1);
    }
    else
    {
        (void)Lib_LcdSetAttr(0);
    }
    Lib_LcdGotoxy(byStartX, y);
    for (i=0; i<byLen; i++)
    {
        (void)Lib_Lcdprintf("*");
    }
    (void)Lib_LcdSetAttr(0);
    (void)Lib_Lcdprintf("_");
    if (0 != byClearLastFlag)
    {
        if (0 != (byMode&IS_FLUSH_LEFT))
        {
            if ((g_byStartX+byWidth) <= 128)
            {
                (void)Lib_Lcdprintf(" ");
            }
        }
    }
    g_dwPreDisplayLen = byLen+1;
}

/****************************************************************************
  函数名     :  void ClearString(BYTE x, BYTE y,  BYTE byMode)
  描述       :  清除指定位置的字符
  输入参数   :  1、BYTE x：列号
                2、BYTE y：行号
                3、BYTE byMode：模式
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2008-01-09   V1.0        创建
****************************************************************************/
void ClearString(BYTE x, BYTE y,  BYTE byMode)
{
    DWORD i;
    BYTE byWidth;
    if (0 != (byMode&IS_FONT_WIDTH_8))
    {
        Lib_LcdSetFont(8, 16, 0);
        byWidth = 8;
    }
    else
    {
        Lib_LcdSetFont(8, 16, 0);
        byWidth = 6;
    }

    if (0 != (byMode&IS_FLUSH_LEFT))
    { // 左对齐
        Lib_LcdGotoxy(x, y);
    }
    else
    { // 右对齐
        Lib_LcdGotoxy(128-g_dwPreDisplayLen*byWidth, y);
    }

    for (i=0; i<g_dwPreDisplayLen; i++)
    {
        Lib_Lcdprintf(" ");
    }
}

/****************************************************************************
  函数名     :  int s_KbGetStrPwd(BYTE *strOutPwd, BYTE byMinLen,
                    BYTE byMaxLen, BYTE byMode, DWORD dwTimeOut10ms)
  描述       :  获取用户密码输入函数
  输入参数   :  1、BYTE byMinLen：输入的最小长度
                2、BYTE byMaxLen：输入的最大长度
                3、BYTE byMode：输入模式
                      bit7：1当输入长度为0时可按ENTER键退出，0当输入长度为0
                            不可按ENTER键退出
                      bit6：1大写，0小写
                      bit2：1左对齐，0右对齐
                      bit0：1正显示，0反显示
                4、DWORD dwTimeOut10ms：输入超时时间长度，单位10毫秒
  输出参数   :  1、BYTE *strOutPwd：用户输入的密码
  返回值     :  0：成功；
                -1011：超时退出；
                -1012：用户按取消键退出；
                -1013：用户按ENTER键退出。
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-07   V1.0        创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
int s_KbGetStrPwd(BYTE *strOutPwd, BYTE byMinLen, BYTE byMaxLen, BYTE byMode,
                 DWORD dwTimeOut10ms)
{
    BYTE byRet, byLen, byStartY, byStartX, byBackSpaceFlag=0;
    int iFlag;

    byStartY = g_byStartY;
    byStartX = g_byStartX;

    strOutPwd[0] = 0;
    iFlag = 0;
    byLen = 0;
    s_SetTimer(1, dwTimeOut10ms);
    while (1)
    {
        if(0 == s_CheckTimer(1))
        {
            return KB_InputTimeOut_Err;
        }
#if 0 //for pci enter pin
        if(0 == s_CheckTimer(3))
        {
            return KB_InputTimeOut_Err;
        }
#endif        
        if (0 == iFlag)
        {
            s_DisplayPwd(byStartX, byStartY, byMode, byLen, byBackSpaceFlag);
            byBackSpaceFlag = 0;
            iFlag = 1;
        }
        if (0 != Lib_KbCheck())
        {
            continue;
        }

        s_SetTimer(1, dwTimeOut10ms);  // 定时器重新计时
        byRet = Lib_KbGetCh();
        switch(byRet)
        {
        case KEYENTER:
            if ((0==byLen) && (0!=(byMode&CAN_ENTER_EXIT)))
            {
                strOutPwd[0] = 0;
                return KB_UserEnter_Exit;
            }
            if((byLen>=byMinLen) && (byLen<=byMaxLen))
            {
                strOutPwd[byLen] = 0;
                return 0;
            }
            Lib_Beep();
            Lib_Beep();
            break;
        case KEYCANCEL:
            strOutPwd[0] = KEYCANCEL;
            strOutPwd[1] = 0;
            return KB_UserCancel_Err;
        case KEYCLEAR:      // 清除
            if (0 == byLen)
            {
                Lib_Beep();
                Lib_Beep();
            }
            else
            {
                byLen = 0;
                strOutPwd[0] = 0;
                ClearString(byStartX, byStartY, byMode);
            }
            break;
        case KEYBACKSPACE:  // 退格
            if (0 != byLen)
            {
                byLen--;
                byBackSpaceFlag = 1;
                strOutPwd[byLen] = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        default:
            if ((byRet>='0') && (byRet<='9') && (byLen<byMaxLen))
            {
                strOutPwd[byLen++] = byRet;
                strOutPwd[byLen] = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        }
        iFlag = 0;
    }
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-07   V1.0        创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
void s_DisplayString(BYTE x, BYTE y, BYTE mode, BYTE *pbyStr, BYTE byClearLastFlag)
{
    BYTE byStartX, byWidth, byLen, byDisplayOff, byDisplayCut;
    int iTemp;

    byDisplayOff = 0;
    byLen = strlen((char*)pbyStr);
    if (0 != (mode&IS_FONT_WIDTH_8))  // 字符大小
    {
        byWidth = 8;
        if (byLen > 15)
        {
            byDisplayOff = byLen - 15;
        }
        (void)Lib_LcdSetFont(8, 16, 0);
    }
    else
    {
        byWidth = 6;
        if (byLen > 20)
        {
            byDisplayOff = byLen - 20;
        }
        (void)Lib_LcdSetFont(8, 16, 0);
    }

    byDisplayCut = 0;
    if (0 != (mode&IS_FLUSH_LEFT))
    {  // 左对齐
        byStartX = x;
        if (0 != (mode&IS_FONT_WIDTH_8))
        {
            byDisplayCut = (byStartX+7)/8;
            if ((byLen+byDisplayCut) > 15)
            {
                byDisplayOff = byLen - 15 + byDisplayCut;
            }
        }
        else
        {
            byDisplayCut = (byStartX+5)/6;
            if ((byLen+byDisplayCut) > 20)
            {
                byDisplayOff = byLen - 20 + byDisplayCut;
            }
        }
    }
    else
    { // 右对齐
        byStartX = 128-(byLen+1-byDisplayOff)*byWidth;
    }

    if (0 != byClearLastFlag)
    {
        iTemp = byStartX;
        iTemp -= byWidth;
        if ((0==(mode&IS_FLUSH_LEFT)) && (iTemp>=0))
        {
            (void)Lib_LcdSetAttr(0);
            Lib_LcdGotoxy(byStartX-byWidth, y);
            (void)Lib_Lcdprintf(" ");
        }
    }
    if (0 == (mode&DISPLAY_NOT_REVER))
    {
        (void)Lib_LcdSetAttr(1);
    }
    else
    {
        (void)Lib_LcdSetAttr(0);
    }
    Lib_LcdGotoxy(byStartX, y);
    (void)Lib_Lcdprintf("%s", &pbyStr[byDisplayOff]);
    (void)Lib_LcdSetAttr(0);
    (void)Lib_Lcdprintf("_");
    if (0 != byClearLastFlag)
    {
        if (0 != (mode&IS_FLUSH_LEFT))
        {
            if ((g_byStartX+byWidth) <= 128)
            {
                (void)Lib_Lcdprintf(" ");
            }
        }
    }
    g_dwPreDisplayLen = strlen((char*)(&pbyStr[byDisplayOff]))+1;
}

/****************************************************************************
  函数名     :  void GetEyeableString(BYTE *pbyInString, BYTE *pbyOutString)
  描述       :  获取可显示的字符串
  输入参数   :  1、BYTE *pbyInString：要转换的字符串
  输出参数   :  1、BYTE *pbyOutString：转换后的可显示的字符串
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-07   V1.0        创建
  2、 黄俊斌     2008-01-03  V1.1         从VOS302移植到VOS305
****************************************************************************/
void GetEyeableString(BYTE *pbyInString, BYTE *pbyOutString)
{
    int iLen, i, j, iTemp, k;

    iTemp = strlen((char*)g_abyVisualAsciiTable);
    iLen = strlen((char*)pbyInString);
    k = 0;
    for (i=0; i<iLen; i++)
    {
        for (j=0; j<iTemp; j++)
        {
            if (pbyInString[i] == g_abyVisualAsciiTable[j])
            {
                pbyOutString[k++] = pbyInString[i];
                break;
            }
        }
    }
    pbyOutString[k] = 0;
}

/****************************************************************************
  函数名     :  void StringToNumber(BYTE *pbyInString, BYTE *pbyOutNumber)
  描述       :  将输入的字符串转换成每个字符所在的数字字符。
  输入参数   :  1、BYTE *pbyInString：要转换的字符串
  输出参数   :  1、BYTE *pbyOutNumber：转换后的数字串
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-11-05   V1.0        创建
  2、 黄俊斌     2008-01-03  V1.1         从VOS302移植到VOS305
****************************************************************************/
void StringToNumber(BYTE *pbyInString, BYTE *pbyOutNumber)
{
    int i = 0, j, k, l = 0;

    while (0 != pbyInString[i])
    {
        for (j=0; j<9; j++)
        {
            for (k=0; k<g_abyKeyIndexLen[j]; k++)
            {
                if (g_abyKeyTableUpper[j][k] == pbyInString[i])
                {
                    pbyOutNumber[l++] = g_abyKeyTableUpper[j][0];
                    goto MyStop;
                }
                else if (g_abyKeyTableLower[j][k] == pbyInString[i])
                {
                    pbyOutNumber[l++] = g_abyKeyTableLower[j][0];
                    goto MyStop;
                }
            }
        }
        for (j=0; j<32; j++)
        {
            if (g_abyKeySymbol[j] == pbyInString[i])
            {
                pbyOutNumber[l++] = g_abyKeySymbol[0];
                goto MyStop;
            }
        }
MyStop:
        i++;
    }
    pbyOutNumber[l] = 0;
}

/****************************************************************************
  函数名     :  int s_KbGetString(BYTE *strOut, BYTE byMinLen, BYTE byMaxLen,
                        BYTE byMode, DWORD dwTimeOut10ms)
  描述       :  获取用户输入的字符串
  输入参数   :  1、BYTE *strOut：要显示的字符串，用户可在这基础上更改，字符串以
                   0x00结尾，字符串中不可以包含小于0x20或者大于0x7e的特殊字符，
                   如果包含了这些字符，这些字符会被当做0x00来处理。
                2、BYTE byMinLen：字符串的最小长度，注：minlen>0，minlen<=maxlen
                3、BYTE byMaxLen：字符串的最大长度，注：LCD_WIDTH_MINI>=maxlen>0
                4、BYTE byMode：输入方式的定义:(优先级顺序为bit3>bit4>bit5，
                   且mode&0x38不能等于0)
                        bit7    1（0） 能（否）回车退出（输入长度为0时才有效）
                        bit6    1（0） 大（小）字体  8X16/6X8
                        bit5    1（0） 能（否）输数字
                        bit4    1（0） 能（否）输字符
                        bit3    1（0） 是（否）密码方式
                        bit2    1（0） 左（右）对齐输入
                        bit1    1（0） 有（否）小数点
                        bit0    1（0） 正（反）显示
                5、DWORD dwTimeOut10ms：超时时间，单位是秒。
                   当取值为0时，表示取默认值120秒。
  输出参数   :  1、BYTE *strOut：输出指定长度范围的字符串，以0x00结尾，字包含
                   取值范围是0x20至0x7e
  返回值     :  0:成功输入,-1013:用户按Enter键退出,-1011:输入超时,-1012:用户取消输入
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-07  V1.0         创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
int s_KbGetString(BYTE *strOut, BYTE byMinLen, BYTE byMaxLen, BYTE byMode,
                 DWORD dwTimeOut10ms)
{
    BYTE byRet, byLen, byStartY, byStartX, byBackSpaceFlag = 0,
        strTemp[130]; // 保存字符串中每个字符所在按键的数字
    int iFlag, iAlphaLowerFlag, iIndex=0;

    byStartY = g_byStartY;
    byStartX = g_byStartX;

    strOut[byMaxLen] = 0;
    iFlag = 0;
    iAlphaLowerFlag = 0;

    // 去掉不可见字符
    GetEyeableString(strOut, strTemp);
    strcpy((char*)strOut, (char*)strTemp);
    byLen = strlen((char*)strOut);

    g_dwPreDisplayLen = byLen;

    StringToNumber(strOut, strTemp);

    s_SetTimer(1, dwTimeOut10ms);
    while (1)
    {
        if(0 == s_CheckTimer(1))
        {
            return KB_InputTimeOut_Err;
        }
        if (0 == iFlag)
        {
            s_DisplayString(byStartX, byStartY, byMode, strOut, byBackSpaceFlag);
            byBackSpaceFlag = 0;
            iFlag = 1;
        }
        if (0 != Lib_KbCheck())
        {
            continue;
        }

        s_SetTimer(1, dwTimeOut10ms);  // 定时器重新计时
        byRet = Lib_KbGetCh();
        switch(byRet)
        {
        case KEYENTER:
            if ((0==byLen) && (0!=(byMode&CAN_ENTER_EXIT)))
            {
                strOut[0] = 0;
                return KB_UserEnter_Exit;
            }
            if((byLen>=byMinLen) && (byLen<=byMaxLen))
            {
                strOut[byLen] = 0;
                return 0;
            }
            Lib_Beep();
            Lib_Beep();
            break;
        case KEYCANCEL:
            strOut[0] = KEYCANCEL;
            strOut[1] = 0;
            return KB_UserCancel_Err;
        case KEYCLEAR:      // 清除
            if (0 == byLen)
            {
                Lib_Beep();
                Lib_Beep();
            }
            else
            {
                byLen = 0;
                strOut[0] = 0;
                strTemp[byLen] = 0;
                iIndex = 0;
                ClearString(byStartX, byStartY, byMode);
            }
            break;
        case KEYBACKSPACE:  // 退格
            if (0 != byLen)
            {
                byLen--;
                byBackSpaceFlag = 1;
                strOut[byLen] = 0;
                strTemp[byLen] = 0;
                iIndex = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        case KEYFN:
            if (0 == iAlphaLowerFlag)
            {
                if ('0' != strTemp[byLen-1])
                {
                    strOut[byLen-1] = g_abyKeyTableLower[strTemp[byLen-1]-'1'][iIndex];
                }
                iAlphaLowerFlag = 1;
            }
            else
            {
                if ('0' != strTemp[byLen-1])
                {
                    strOut[byLen-1] = g_abyKeyTableUpper[strTemp[byLen-1]-'1'][iIndex];
                }
                iAlphaLowerFlag = 0;
            }
            break;
        case KEYALPHA:
		case KEYSTAR:
            if (0 == byLen)
            {
                Lib_Beep();
                Lib_Beep();
            }
            else
            {
                if ('0' == strTemp[byLen-1])
                {
                    iIndex++;
                    iIndex %= 32;
                    strOut[byLen-1] = g_abyKeySymbol[iIndex];
                }
                else if ((strTemp[byLen-1]>='1') || (strTemp[byLen-1]<='9'))
                {
                    iIndex++;
                    iIndex %= g_abyKeyIndexLen[strTemp[byLen-1]-'1'];
                    if (0 == iAlphaLowerFlag)
                    {
                        strOut[byLen-1] = g_abyKeyTableUpper[strTemp[byLen-1]-'1'][iIndex];
                    }
                    else
                    {
                        strOut[byLen-1] = g_abyKeyTableLower[strTemp[byLen-1]-'1'][iIndex];
                    }
                }
                else
                {
                    Lib_Beep();
                    Lib_Beep();
                }
            }
            break;
        default:
            if ((byRet>='0') && (byRet<='9') && (byLen<byMaxLen))
            {
                strTemp[byLen] = byRet;  // 数字
                strOut[byLen] = byRet;   // 字符
                byLen++;
                strOut[byLen] = 0;
                strTemp[byLen] = 0;
                iIndex = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        }
        iFlag = 0;
    }
}

/****************************************************************************
  函数名     :  void s_Kb_DataStrConvAmt(BYTE *pbyAmtIn, BYTE byInlen, BYTE *pbyDispOut)
  描述       :  将输入金额字符串修改成两位小数的字符串
  输入参数   :  1、BYTE *pbyAmtIn：输入的金额字符串
                2、BYTE byInlen：输入长度
  输出参数   :  1、BYTE *pbyDispOut：输出的字符串格式
                     pbyDispOut[0]：输出字符串长度
                     pbyDispOut[1~pbyDispOut[0]]：输出的字符串内容
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void s_Kb_DataStrConvAmt(BYTE *pbyAmtIn, BYTE byInlen, BYTE *pbyDispOut)
{
    BYTE i;

    // 去掉靠左边的'0'
    for (i=0; i<byInlen; i++)
    {
        if (pbyAmtIn[i] != '0')
            break;
    }
    byInlen -= i;
    pbyAmtIn += i;

    strcpy((char*)pbyDispOut, "0.00");
    switch (byInlen)
    {
    case 0: // "0.00"
        return ;
    case 1:  // "0.0X"
        pbyDispOut[3] = pbyAmtIn[0];
        return ;
    case 2: // "0.XX"
        memcpy(&pbyDispOut[2], &pbyAmtIn[0], 2);
        return ;
    default:  // XXXX.XX
        memcpy(pbyDispOut, pbyAmtIn, byInlen-2);
        pbyDispOut[byInlen-2] = '.';
        memcpy(&pbyDispOut[byInlen-1], &pbyAmtIn[byInlen-2], 2);
        pbyDispOut[byInlen+1] = 0;
        return ;
    }
}

/****************************************************************************
  函数名     :
  描述       :
  输入参数   :
  输出参数   :
  返回值     :
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-07  V1.0         创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
int s_KbGetStrDotNumber(BYTE *strOut, BYTE byMinLen, BYTE byMaxLen, BYTE byMode,
                 DWORD dwTimeOut10ms)
{
    BYTE byRet, byLen, byStartY, byStartX, abyTemp[130], byBackSpaceFlag = 0;
    int iFlag;

    byStartY = g_byStartY;
    byStartX = g_byStartX;

    strOut[byMaxLen] = 0;
    iFlag = 0;
    byLen = strlen((char*)strOut);
    s_SetTimer(1, dwTimeOut10ms);
    while (1)
    {
        if(0 == s_CheckTimer(1))
        {
            return KB_InputTimeOut_Err;
        }
        if (0 == iFlag)
        {
            s_Kb_DataStrConvAmt(strOut, byLen, abyTemp);
            s_DisplayString(byStartX, byStartY, byMode, abyTemp, byBackSpaceFlag);
            byBackSpaceFlag = 0;
            iFlag = 1;
        }
        if (0 != Lib_KbCheck())
        {
            continue;
        }

        s_SetTimer(1, dwTimeOut10ms);  // 定时器重新计时
        byRet = Lib_KbGetCh();
        switch(byRet)
        {
        case KEYENTER:
            if ((0==byLen) && (0!=(byMode&CAN_ENTER_EXIT)))
            {
                strOut[0] = 0;
                return KB_UserEnter_Exit;
            }
            if((byLen>=byMinLen) && (byLen<=byMaxLen))
            {
                strOut[byLen] = 0;
                return 0;
            }
            Lib_Beep();
            Lib_Beep();
            break;
        case KEYCANCEL:
            strOut[0] = KEYCANCEL;
            strOut[1] = 0;
            return KB_UserCancel_Err;
        case KEYCLEAR:      // 清除
            if (0 == byLen)
            {
                Lib_Beep();
                Lib_Beep();
            }
            else
            {
                ClearString(byStartX, byStartY, byMode);
                byLen = 0;
                strOut[0] = 0;
            }
            break;
        case KEYBACKSPACE:  // 退格
            if (0 != byLen)
            {
                byLen--;
                byBackSpaceFlag = 1;
                strOut[byLen] = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        default:
            if ((byRet>='0') && (byRet<='9') && (byLen<byMaxLen))
            {
                strOut[byLen++] = byRet;
                strOut[byLen] = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        }
        iFlag = 0;
    }
}


/****************************************************************************
  函数名     :  int s_KbGetStrNumber(BYTE *strOut, BYTE byMinLen,
                     BYTE byMaxLen, BYTE byMode, DWORD dwTimeOut10ms)
  描述       :  获取用户输入的数字串
  输入参数   :  1、BYTE byMinLen：输入的最小长度
                2、BYTE byMaxLen：输入的最大长度
                3、BYTE byMode：输入模式
                      bit7：1当输入长度为0时可按ENTER键退出，0当输入长度为0
                            不可按ENTER键退出
                      bit6：1大写，0小写
                      bit2：1左对齐，0右对齐
                      bit0：1正显示，0反显示
                4、DWORD dwTimeOut10ms：输入超时时间长度，单位10毫秒
  输出参数   :  1、BYTE *strOut：用户输入的数字串，该参数既做输入参数又做输出参数
  返回值     :  0：成功
                -1011：超时退出
                -1012：用户取消输入
                -1013：用户按ENTER键退出
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-10-07   V1.0        创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
int s_KbGetStrNumber(BYTE *strOut, BYTE byMinLen, BYTE byMaxLen, BYTE byMode,
                 DWORD dwTimeOut10ms)
{
    BYTE byRet, byLen, byStartY, byStartX, byBackSpaceFlag=0;
    int iFlag, iDotFlag = 0;

    byStartY = g_byStartY;
    byStartX = g_byStartX;

    strOut[byMaxLen] = 0;
    iFlag = 0;
    byLen = strlen((char*)strOut);
    s_SetTimer(1, dwTimeOut10ms);
    while (1)
    {
        if(0 == s_CheckTimer(1))
        {
            return KB_InputTimeOut_Err;
        }
        if (0 == iFlag)
        {
            s_DisplayString(byStartX, byStartY, byMode, strOut, byBackSpaceFlag);
            byBackSpaceFlag = 0;
            iFlag = 1;
        }
        if (0 != Lib_KbCheck())
        {
            continue;
        }

        s_SetTimer(1, dwTimeOut10ms);  // 定时器重新计时
        byRet = Lib_KbGetCh();
        switch(byRet)
        {
        case KEYENTER:
            if ((0==byLen) && (0!=(byMode&CAN_ENTER_EXIT)))
            {
                strOut[0] = 0;
                return KB_UserEnter_Exit;
            }
            if((byLen>=byMinLen) && (byLen<=byMaxLen) && (1!=iDotFlag))
            {
                strOut[byLen] = 0;
                return 0;
            }
            Lib_Beep();
            Lib_Beep();
            break;
        case KEYCANCEL:
            strOut[0] = KEYCANCEL;
            strOut[1] = 0;
            return KB_UserCancel_Err;
        case KEYCLEAR:      // 清除
            if (0 == byLen)
            {
                Lib_Beep();
                Lib_Beep();
            }
            else
            {
                byLen = 0;
                strOut[0] = 0;
                ClearString(byStartX, byStartY, byMode);
            }
            break;
        case KEYBACKSPACE:  // 退格
            if (0 != byLen)
            {
                byLen--;
                byBackSpaceFlag = 1;
                if ('.' == strOut[byLen])
                {
                    iDotFlag = 0;
                }
                strOut[byLen] = 0;
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        case KEYALPHA:
		case KEYSTAR:
            if (   (2!=iDotFlag)
                && (0!=byLen)
                && (('.'==strOut[byLen-1])||('1'==strOut[byLen-1])))
            {
                if ('.' == strOut[byLen-1])
                {
                    strOut[byLen-1] = '1';
                    iDotFlag = 0;
                }
                else
                {
                    strOut[byLen-1] = '.';
                    iDotFlag = 1;
                }
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        default:
            if ((byRet>='0') && (byRet<='9') && (byLen<byMaxLen))
            {
                strOut[byLen++] = byRet;
                strOut[byLen] = 0;
                if (1 == iDotFlag)
                {
                    iDotFlag = 2;
                }
            }
            else
            {
                Lib_Beep();
                Lib_Beep();
            }
            break;
        }
        iFlag = 0;
    }
}


/****************************************************************************
  函数名     :  int Lib_KbGetStr(BYTE *str, BYTE minlen, BYTE maxlen, BYTE mode,
                   WORD timeoutsec)
  描述       :  获取用户输入字符串函数
  输入参数   :  1、BYTE *str：要显示的字符串，用户可在这基础上更改，字符串以
                   0x00结尾，字符串中不可以包含小于0x20或者大于0x7e的特殊字符，
                   如果包含了这些字符，这些字符会被当做0x00来处理。
                2、BYTE minlen：输出字符最小长度，注：minlen>0，minlen<=maxlen
                3、BYTE maxlen：输出字符最大长度，注：LCD_WIDTH_MINI>=maxlen>0
                4、BYTE mode：输入方式的定义:(优先级顺序为bit3>bit4>bit5，
                   且mode&0x38不能等于0)
                        bit7    1（0） 能（否）回车退出（输入长度为0时才有效）
                        bit6    1（0） 大（小）字体  8X16/6X8
                        bit5    1（0） 能（否）输数字
                        bit4    1（0） 能（否）输字符
                        bit3    1（0） 是（否）密码方式
                        bit2    1（0） 左（右）对齐输入
                        bit1    1（0） 有（否）小数点
                        bit0    1（0） 正（反）显示
                5、WORD timeoutsec：超时时间长度，单位是秒。
                   当取值为0时，表示取默认值120秒。
  输出参数   :  1、BYTE *str：输出指定长度范围的字符串，以0x00结尾，字包含取值
                   范围是0x20至0x7e
  返回值     :  -1010：输入参数错误
  备    注   :  “向上”键等效过去的“退格”键，
                “向下”键等效过去的“字母”键，
                “功能”键和过去的一样，都使用做字母大小写切换
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-11-02   V1.0        创建
  2、 黄俊斌     2008-01-09  V1.1         修改在指定位置输入时清除了该行的其他
                                          内容的错误
****************************************************************************/
int Lib_KbGetStr(BYTE *str, BYTE minlen, BYTE maxlen, BYTE mode,
                   WORD timeoutsec)
{
    DWORD dwTimeOut10ms;
    int iRet;
    BYTE byOldFontYpe, byOldFontAttr, byStartX, byStartY;

    byStartX = g_byStartX;
    byStartY = g_byStartY;
    byOldFontYpe = g_byFontType;
    byOldFontAttr = g_byFontAttr;
    if ((minlen>maxlen) || (maxlen>128) || (0==maxlen) || (NULL==str)
        || (0==(mode&0x38)))
    {
        return KB_InputPara_Err;
    }

    if (0 == timeoutsec)
    {
		dwTimeOut10ms = 6000;  // 60秒
    }
    else
    {
        dwTimeOut10ms = timeoutsec*100;
    }

    if (0 != (mode&CAN_INPUT_PASSWORD))
    {
        iRet = s_KbGetStrPwd(str, minlen, maxlen, (mode&0xc5), dwTimeOut10ms);
    }
    else if (0 != (mode&CAN_INPUT_CHAR))
    {
        iRet = s_KbGetString(str, minlen, maxlen, mode, dwTimeOut10ms);
    }
    else if (0 != (mode&CAN_INPUT_DECIMAL))
    {
        iRet = s_KbGetStrDotNumber(str, minlen, maxlen, mode, dwTimeOut10ms);
    }
    else
    {
        iRet = s_KbGetStrNumber(str, minlen, maxlen, mode, dwTimeOut10ms);
    }
    g_byStartX = byStartX;
    g_byStartY = byStartY;
    g_byFontType = byOldFontYpe;
    g_byFontAttr = byOldFontAttr;
    return iRet;
}

void TestKbGetch()
{
    BYTE byKey;

    Lib_LcdCls();
    //Lib_LcdPrintxyCE(0, 0, 0x81, "按键测试  ", "Key Test");
	Lib_LcdPrintxy(0, 8*0,0x80, "     Key   Test    ");
    while (1)
    {
        if (0 == Lib_KbCheck())
        {
            byKey = Lib_KbGetCh();
//test
//trace_debug_printf("KbGetCh[%02x]",byKey);
            
           /* Lib_LcdClrLine(16/2, 31);
            Lib_LcdGotoxy(0, 16/2);
            Lib_Lcdprintf("  %x", byKey);*/
			Lib_LcdPrintxy(0, 8*2,0x00,"  %02x", byKey);
            if (KEYCANCEL == byKey)
            {
              
              //  return ;
            }
        }
    }
}

