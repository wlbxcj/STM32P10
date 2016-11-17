
#include "KF701DH.h"
#include "stm32f10x_lib.h"
#include "..\..\inc\FunctionList.h"

#include  "lcd.h"
#include "calibration.h"

#include "dll.h"

#include  "base.h"
#include  "comm.h"
#include  "vosapi.h"


#define LcdWrCmd     0
#define LcdRdCmd     1
#define LcdWrData    2
#define LcdRdData    3 

#define MAX_SOFT_HZ_NUM (500)

#define LCD_AD_FLAG (3000)

#define  SetLcdA0_H()      GPIO_SetBits(GPIOB, GPIO_Pin_14)   
#define  SetLcdA0_L()      GPIO_ResetBits(GPIOB, GPIO_Pin_14)   
#define  SetLcdCS_H()      GPIO_SetBits(GPIOA, GPIO_Pin_8)   
#define  SetLcdCS_L()      GPIO_ResetBits(GPIOA, GPIO_Pin_8)  
/*
#define  SetLcdWR_H()      GPIO_SetBits(GPIOC, GPIO_Pin_9)   
#define  SetLcdWR_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_9)   
#define  SetLcdRD_H()      GPIO_SetBits(GPIOC, GPIO_Pin_8)   
#define  SetLcdRD_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_8)   
*/
#define  SetLcdWR_H()      GPIO_SetBits(GPIOC, GPIO_Pin_9)   
#define  SetLcdWR_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_9)   
#define  SetLcdRD_H()      GPIO_SetBits(GPIOC, GPIO_Pin_8)   
#define  SetLcdRD_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_8)   

#ifdef ST7567_M //13/05/26
#define GRAY_MIN_VALUE      10  //40->10
#define DEFAULT_GRAY_BASE   10 //40->10  
#endif

BYTE g_byStartX = 0;    // 当前起始位置坐标x，取值0~127
BYTE g_byStartY = 0;    // 当前起始位置坐标y，取值0~63
BYTE g_byFontType;      // 显示字体。
                        // 0：     ASCII显示模式，汉字16x16，   ASCII字符6x8
                        // 1：GB2312标准显示模式，汉字16x16，   ASCII字符8x16
                        // 2：GB2312特殊显示模式，汉字12x12，   ASCII字符6x12

BYTE g_byFontAttr;      // 液晶显示属性，0表示正常显示，非0表示黑白颠倒显示

static BYTE gs_byLcdBusyFlag = 0;           // 液晶屏忙标志，0表示空闲，非0表示忙，忙的时候不能进行设置
BYTE gs_byBaseGrayVal = DEFAULT_GRAY_BASE; // 灰度基值
//static BYTE gs_byRestoreX = 0;              // 备份起始位置坐标x，取值0~127
//static BYTE gs_byRestoreY = 0;              // 备份起始位置坐标y，取值0~63
//static BYTE gs_byRestoreEnable = 0;         // 备份使能，1可备份，0不可备份，在没有做过备份的时候，该变量取值为0
static BYTE gs_abyDisplayData[6][132];      // 屏幕显示数据缓冲
static BYTE gs_abyCurrentData[6][132];      // 当前屏幕上显示的数据

unsigned long LCDValue; // <3000 新屏 >=3000 旧屏 新屏值2043左右 老屏值3520左右
                        // <= 300 新屏带USB   >= 4000 老屏带USB


// 15个图标的设置值
BYTE g_byIconVal[15] = {2, 3, 4, 20, 22, 24, 26, 28, 30, 42, 61, 77, 93, 108, 122};

BYTE g_byIconBufUpdated;
BYTE g_abyIconBufMode[8];

// 背光显示模式。0 关背光；1 背光保持亮1分钟（在1分钟后，自动关闭），
// 默认值；2 背光常亮。该变量还在磁卡和智能卡模块使用
BYTE k_LcdBackLightMode;
int  k_LcdBackLightTime;

int g_iLcdTimeCount = 6000;   //0

extern int g_iChineseFontFlag;

int g_iHaveFontFlag = 0;  // 字库存在标志，0表示不存在，1表示存在

void s_ScrSetIcon(void);
int  s_Lcdprintf(char *str);
void s_LCD_WrCMD(BYTE data_in, BYTE cmd_mode);
void s_LcdWrStr(BYTE *str, BYTE strLen);
void s_LcdSetIcon(BYTE byIconNo, BYTE byMode);
void s_LcdDrawLogo(BYTE *pDataBuffer);
void s_LcdFlush();

u8 Display_Init(void);
extern unsigned char sign_buffer[136][48];
extern BYTE gs_PadStatus ; 


//for ascii8X8.h
typedef struct 
{
  unsigned short nHz;
  unsigned char  sDot[18];
} HZ_T;


extern HZ_T tHz12X12[];
extern const unsigned char Ch8ModAddr[];

#if 0
BYTE s_ReadGrayValue(void)
{
	BYTE *startAddr;
	BYTE buf[2]={0};
	BYTE level;
#if 0  //tmp disable
	startAddr = (BYTE *)(0xA1028000); 
	hal_flash_read(buf, startAddr, 2);
	if (buf[0] != 0xa5) {
		return 0xff;
	}
#endif
        sys_flash_syspara_read(LCD_GRAY_M,buf,2);
        
        
        //test
        //buf[1] = 10;
    buf[1] = 19;
        
	level = buf[1];
    if (level < GRAY_MIN_LEVEL)
    {
        level = GRAY_MIN_LEVEL;
    }
    else if (level > GRAY_MAX_LEVEL)
    {
        level = GRAY_MAX_LEVEL;
    }

	return level;
}
#endif
/****************************************************************************
  函数名     :  int s_ADCInit(void)
  描述       :  硬件ADC初始化函数
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因

****************************************************************************/

int s_ADCInit(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

//    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init( GPIOC,&GPIO_InitStructure );

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel14 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_28Cycles5);
	ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
	/* Enable ADC1 */
	ADC_Cmd( ADC1,ENABLE );

    return 0;
}

unsigned long Lib_LCDVerValue(void)
{
    unsigned long value;

    /* Enable ADC1 reset calibaration register */   
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));	
		
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);
    delay_ms(1);
	/* Get injected channel14 converted value */
	value = ADC_GetConversionValue(ADC1);
	
	return value;
}


/****************************************************************************
  函数名     :  int s_LcdInit(void)
  描述       :  液晶初始化函数
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因

****************************************************************************/
int s_LcdInit(void)
{
    BYTE Gray;
    
    
    Display_Init();
    s_ADCInit();
    LCDValue = Lib_LCDVerValue();
 //   while(1)
    trace_debug_printf("\n\r******value = %d\n\r",LCDValue);
    g_byFontType = ASCII;                 // Small Font
    g_byFontAttr = 0;                     // Normal
    gs_byLcdBusyFlag = 0;
    g_byIconBufUpdated = 0;
    memset(g_abyIconBufMode, 0, sizeof(g_abyIconBufMode));  
    memset(gs_abyDisplayData, 0, sizeof(gs_abyDisplayData));
    memset(gs_abyCurrentData, 0, sizeof(gs_abyCurrentData));

    
    Lib_LcdCls(); // LCD清屏
    //gs_byRestoreEnable = 0;
    
    
    
    Gray = s_ReadGrayValue();
#ifdef ST7567_M
    Gray = 11;
#endif    
    
    if (Gray == 0xff)
	Lib_LcdSetGray(DEFAULT_GRAY_LEVEL);
    else 
	Lib_LcdSetGray(Gray);

//    s_LCD_WrCMD(0x23, LcdWrCmd);
	s_LCD_WrCMD(0x81, LcdWrCmd);
    s_LCD_WrCMD(0x18, LcdWrCmd);
    return 0;
}

//void LcdWriteCmd(u8 Command,u8 cmd_mode)
void s_LCD_WrCMD(u8 Command,u8 cmd_mode)
{
    u8 i,OutData;
  
    GPIO_ResetBits(GPIOA,GPIO_Pin_8);//LCD_CS=L
    OutData=Command;
    switch(cmd_mode)
    {
    case LcdWrCmd:
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);//LCD_CD=L->LCD_A0
        break;
    case LcdWrData:
        GPIO_SetBits(GPIOB, GPIO_Pin_14);//LCD_CD=H
        break;
    default:
        return;
      
    }
    GPIO_SetBits(GPIOB, GPIO_Pin_13); //LCD_SCL=H
    for(i=0;i<8;i++)
    {
        if(OutData&0x80)
            GPIO_SetBits(GPIOB, GPIO_Pin_15); //LCD_SDA=H
        else
            GPIO_ResetBits(GPIOB, GPIO_Pin_15); //LCD_SDA=L
        GPIO_ResetBits(GPIOB, GPIO_Pin_13); //LCD_SCL=L
        GPIO_SetBits(GPIOB, GPIO_Pin_13); //LCD_SCL=H
        OutData = OutData << 1;
    }
    GPIO_SetBits(GPIOA, GPIO_Pin_8);//LCD_CS=H
    
}

void LcdBL_Control(u8 Control)
{
    /* for kf322
    if (Control)
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);          //Switch on
    else
        GPIO_SetBits(GPIOB, GPIO_Pin_9);            //Switch off
    */

  //for kf311
#if 0 
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif
  
  if(Control)
    GPIO_SetBits(GPIOD, GPIO_Pin_2);//off
  else
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);//on

  

  
}

void Stnlcd_Init(void)
{
#if 0  
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);    

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_9 | GPIO_Pin_15); 
    
    LcdBL_Control(1);     WaitNuS(2000);      //Switch on
    LcdReset();           WaitNuS(10000);     //Reset lcd ic
    LcdWriteCmd(0xE2);    WaitNuS(2000);      //软复位
    LcdWriteCmd(0x2C);    WaitNuS(2000);      //booster circuit on
    LcdWriteCmd(0x2E);    WaitNuS(2000);      //voltage regulator circuit on
    LcdWriteCmd(0x2F);    WaitNuS(2000);      //voltage follower circut on    
    LcdWriteCmd(0x40);    WaitNuS(2000);      //Sets the display RAM display start line address    
    LcdWriteCmd(0x23);    WaitNuS(2000);      //internal resistor ratio,control output voltage        

    LcdWriteCmd(0x81);    WaitNuS(2000);      //Set the V0 output voltage electronic volume register     
    LcdWriteCmd(38);      WaitNuS(2000);      //Electronic volume value,range 0~63,too lager,too depth  
    LcdWriteCmd(0xC8);    WaitNuS(2000);      //Select COM output scan direction (bit 3)0: normal  1: reverse      
    LcdWriteCmd(0xAF);                        //lcd on
    LcdBL_Control(0);                         //Switch off
#endif
   
#if 0    
    //test
    for(;;)
    {
    LcdBL_Control(1);
    delay_ms(100);
    LcdBL_Control(0);
    delay_ms(100);
    }
#endif
    
    LcdBL_Control(1);
    
    s_LCD_WrCMD(0xe2, LcdWrCmd);  // Reset LCD
    s_LCD_WrCMD(0xc8, LcdWrCmd);
    s_LCD_WrCMD(0xa0, LcdWrCmd);
    s_LCD_WrCMD(0xa2, LcdWrCmd); //Sets the LCDdriver voltage bias ratio

#ifndef ST7567_M
    s_LCD_WrCMD(0x23, LcdWrCmd);
#endif
    
    s_LCD_WrCMD(0xd2, LcdWrCmd);
    s_LCD_WrCMD(0x2C, LcdWrCmd);
    delay_ms(60);
    s_LCD_WrCMD(0x2E, LcdWrCmd);
    delay_ms(5);
    s_LCD_WrCMD(0x2f, LcdWrCmd);
    delay_ms(5);
    s_LCD_WrCMD(0xaf, LcdWrCmd);  // 0xaf:display on; 0xae:display off
    s_LCD_WrCMD(0x2f, LcdWrCmd);

    s_LCD_WrCMD(0x40, LcdWrCmd);         //Sets the display RAM display start line address    
    
    s_LCD_WrCMD(0xb0, LcdWrCmd);  //0页
    s_LCD_WrCMD(0x10, LcdWrCmd);  //0列high位
    s_LCD_WrCMD(0, LcdWrCmd);     //0列low位
    
    
#ifdef ST7567_M
    s_LCD_WrCMD(0x22, LcdWrCmd);
    delay_ms(60);
    
    s_LCD_WrCMD(0xa2, LcdWrCmd); //Sets the LCDdriver voltage bias ratio

    delay_ms(60);
    
#endif    
    
    s_LCD_WrCMD(0x81, LcdWrCmd);
    //LcdWriteCmd(gs_byBaseGrayVal,LcdWrCmd);
#ifdef ST7567_M
    s_LCD_WrCMD(18,LcdWrCmd);
#else    
    s_LCD_WrCMD(50,LcdWrCmd);
#endif    
    //Lcd_Cls();    


    
   

    
}
u8 Display_Init(void)
{
    u8 DisplayType;
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    DisplayType  = 0;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8; //-LCD_CS
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    

    
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_1; //-LCD_RST
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_1);//H
                   
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_14|GPIO_Pin_13|GPIO_Pin_15; //-LCD_A0 LCD_SCLK LCD_SDA
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //black light
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
#if 0    
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    DisplayType = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
#endif

    
   // s_LCD_WrCMD(0xa2, LcdWrCmd);
   // s_LCD_WrCMD(0x81, LcdWrCmd);
    //s_LCD_WrCMD(0x3f, LcdWrCmd);

    //while(1);
    //13/07/09
#ifndef ST7567_M
 
    Stnlcd_Init();
#endif
    
    return(DisplayType);
}

void s_SetXY(u8 x, u8 y)
{
    y &= 0x07;
    if(LCDValue < LCD_AD_FLAG)
        x &= 0xff;
    else if(LCDValue >= LCD_AD_FLAG)
        x &= 0x7f;
    //x++;
//    if (y < 4)
//    {
//        y += 4;
//    }
//    else
//    {
//        y -= 4;
//    }
    s_LCD_WrCMD((0xb0|( y & 0x07)), LcdWrCmd);  //y页字行//
    s_LCD_WrCMD((0x10|(x>>4)), LcdWrCmd);  //x列high位//
    s_LCD_WrCMD((x&0x0f), LcdWrCmd);              //x列low位//
    x++;
}

/****************************************************************************
  函数名     :  void s_LcdCls(void)
  描述       :  清屏和清缓冲
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-19  V1.0         创建
****************************************************************************/
void s_LcdCls(void)
{
  

    u8 i, line;

    s_LCD_WrCMD(0xaf, LcdWrCmd);    //display on
    if(LCDValue < LCD_AD_FLAG)
    {
        for (line=0; line<=6; line++)
        {
            if(line>=2)
            {
            #ifdef ST7567_M
              s_SetXY(0, line);
            #else          
              
              s_SetXY(0, line+4);
            #endif
            }
            else
              s_SetXY(0, line);
              
            for (i=0; i<=132; i++) //列号//
            {
                s_LCD_WrCMD(0, LcdWrData);
            }
        } 
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        for (line=0; line<=4; line++)
        {
            if(line>=2)
            {
            #ifdef ST7567_M
              s_SetXY(0, line);
            #else          
              
              s_SetXY(0, line+4);
            #endif
            }
            else
              s_SetXY(0, line);
              
            for (i=0; i<=122; i++) //列号//
            {
                s_LCD_WrCMD(0, LcdWrData);
            }
        } 
    } 
    memset(gs_abyDisplayData, 0, sizeof(gs_abyDisplayData));
    memset(gs_abyCurrentData, 0, sizeof(gs_abyCurrentData));

  g_byStartY = 0;
  g_byStartX = 0;
  
  Lcdarray_clear(0x0f);
  
}


/****************************************************************************
  函数名     :  void Lib_LcdCls(void)
  描述       :  清屏，只清除文字显示区域，不清除图标
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         创建
****************************************************************************/
void Lib_LcdCls(void)
{
    if (gs_byLcdBusyFlag)
    {
        return;
    }
    gs_byLcdBusyFlag = 1;
    s_LcdCls();
    gs_byLcdBusyFlag = 0;
}



/****************************************************************************
  函数名     :
  描述       :  恢复图标显示
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void s_ScrSetIcon(void)
{
    BYTE i;

    if (!g_byIconBufUpdated)
    {
        return;
    }
    g_byIconBufUpdated = 0;
    for (i=1; i<9; i++)
    {
        s_LcdSetIcon(i, g_abyIconBufMode[i-1]);
    }
}

/****************************************************************************
  函数名     :  void s_ControlIcon(int iNumber, BYTE byOnOff)
  描述       :  控制图标显示或者关闭
  输入参数   :  1、int iNumber：图标编号，取值范围是0至14，其他值非法
                2、BYTE byOnOff：开关，0表示关闭，非0表示打开
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-17  V1.0         创建
****************************************************************************/
void s_ControlIcon(int iNumber, BYTE byOnOff)
{
    BYTE byTemp, byData;

    byTemp = g_byIconVal[iNumber];
    if (0 == byOnOff)
    {
        byData = 0x00;
    }
    else
    {
        byData = 0xff;
    }
    s_LCD_WrCMD(0xb8, LcdWrCmd);    // 第8页字行
    s_LCD_WrCMD((0x10|(byTemp>>4)&0x07), LcdWrCmd);  // 列high位
    s_LCD_WrCMD(byTemp&0x0f, LcdWrCmd);   // 列low位
    s_LCD_WrCMD(byData, LcdWrData);    // 开关控制
}

/****************************************************************************
  函数名     :  void s_CloseAllIcon()
  描述       :  关闭所有图标
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         创建
****************************************************************************/
void s_CloseAllIcon()
{
    BYTE i;
    for (i=0; i<15; i++)
    {
        s_ControlIcon(i, CLOSEICON);
    }
}

/****************************************************************************
  函数名     :  void s_LcdSetIcon(BYTE byIconNo, BYTE byMode)
  描述       :  设置图标函数，输入参数错误将无任何操作
  输入参数   :  1、BYTE byIconNo：图标编号，取值为1至8，其他值非法
                2、BYTE byMode：图标显示模式，由图标编号决定
   byIconNo   byMode  含义
   1----------0-------电话图标关闭，2、3和4都关闭
   1----------1-------电话图标摘机，2和4打开，3关闭
   1----------2-------电话图标挂机，3和4打开，2关闭
   2----------0-------无线信号图标关闭，   20、22、24、26、28和30都关闭
   2----------1-------无线信号图标强度为0，20打开，22、24、26、28和30关闭
   2----------2-------无线信号图标强度为1，20和22打开，24、26、28和30关闭
   2----------3-------无线信号图标强度为2，20、22和24打开，26、28和30关闭
   2----------4-------无线信号图标强度为3，20、22、24和26打开，28和30关闭
   2----------5-------无线信号图标强度为4，20、22、24、26和28打开，30关闭
   2----------6-------无线信号图标强度为5，20、22、24、26、28和30都打开
   3----------0-------打印机图标关闭，42关闭
   3----------1-------打印机图标打开，42打开
   4----------0-------IC卡图标关闭，61关闭
   4----------1-------IC卡图标打开，61打开
   5----------0-------锁图标关闭，77关闭
   5----------1-------锁图标打开，77打开
   6----------0-------电池图标关闭，93关闭
   6----------1-------电池图标打开，93打开
   7----------0-------向上图标关闭，108关闭
   7----------1-------向上图标打开，108打开
   8----------0-------向下图标关闭，122关闭
   8----------1-------向下图标打开，122打开
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void s_LcdSetIcon(BYTE byIconNo, BYTE byMode)
{
    BYTE i;
    switch (byIconNo)
    {
    case 1: //telephone
        switch (byMode)
        {
        case 0:   // 关闭
            s_ControlIcon(0, 0);
            s_ControlIcon(1, 0);
            s_ControlIcon(2, 0);
            break;
        case 1:   //摘机
            s_ControlIcon(0, 1);
            s_ControlIcon(1, 0);
            s_ControlIcon(2, 1);
            break;
        case 2:   // 挂机
            s_ControlIcon(0, 0);
            s_ControlIcon(1, 1);
            s_ControlIcon(2, 1);
            break;
        default:
            break;
        }
        break;
    case 2: //signal level
        if (byMode <= 6)
        {
            for (i=0; i<byMode; i++)
            {
                s_ControlIcon(3+i, 1);
            }
            for (i=byMode; i<6; i++)
            {
                s_ControlIcon(3+i, 0);
            }
        }
        break;
    default: //printer, IC, Lock-up, Loudspeaker, Up, Down
        if (byMode <= 1)
        {
            s_ControlIcon(9+byIconNo-2-1, byMode);
        }
        break;
    }
}


/****************************************************************************
  函数名     :  void Lib_LcdSetIcon(BYTE byIconNo, BYTE byMode)
  描述       :  修改液晶图标显示
  输入参数   :  1、BYTE byIconNo：图标编号，取值为1至8，其他值非法
                2、BYTE byMode：图标显示模式，由图标编号决定
   byIconNo   byMode  含义
   1----------0-------电话图标关闭，2、3和4都关闭
   1----------1-------电话图标摘机，2和4打开，3关闭
   1----------2-------电话图标挂机，3和4打开，2关闭
   2----------0-------无线信号图标关闭，   20、22、24、26、28和30都关闭
   2----------1-------无线信号图标强度为0，20打开，22、24、26、28和30关闭
   2----------2-------无线信号图标强度为1，20和22打开，24、26、28和30关闭
   2----------3-------无线信号图标强度为2，20、22和24打开，26、28和30关闭
   2----------4-------无线信号图标强度为3，20、22、24和26打开，28和30关闭
   2----------5-------无线信号图标强度为4，20、22、24、26和28打开，30关闭
   2----------6-------无线信号图标强度为5，20、22、24、26、28和30都打开
   3----------0-------打印机图标关闭，42关闭
   3----------1-------打印机图标打开，42打开
   4----------0-------IC卡图标关闭，61关闭
   4----------1-------IC卡图标打开，61打开
   5----------0-------锁图标关闭，77关闭
   5----------1-------锁图标打开，77打开
   6----------0-------电池(或者扬声器)图标关闭，93关闭
   6----------1-------电池(或者扬声器)图标打开，93打开
   7----------0-------向上图标关闭，108关闭
   7----------1-------向上图标打开，108打开
   8----------0-------向下图标关闭，122关闭
   8----------1-------向下图标打开，122打开
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-17  V1.0         创建
****************************************************************************/
void Lib_LcdSetIcon(BYTE byIconNo, BYTE byMode)
{
    if ((byIconNo==0) || (byIconNo>8))
    {
        return;
    }
    if (gs_byLcdBusyFlag)
    {
        g_byIconBufUpdated = 1;
        g_abyIconBufMode[byIconNo-1] = byMode;
        return;
    }

    gs_byLcdBusyFlag = 1;
    s_LcdSetIcon(byIconNo, byMode);
    g_abyIconBufMode[byIconNo-1] = byMode;
    gs_byLcdBusyFlag = 0;
}

/****************************************************************************
  函数名     :  void s_LcdDrawPlot(BYTE XO, BYTE YO, BYTE Color)
  描述       :  修改数据缓冲内的某个数据位
  输入参数   :  1、BYTE XO：列号，取值0至122
                2、BYTE YO：行号，取值0至31
                3、BYTE Color：颜色，0表示白色，非0表示黑色
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-17   V1.0        创建
****************************************************************************/
void s_LcdDrawPlot(BYTE XO, BYTE YO, BYTE Color)
{
    if (0 == Color)
    {
        gs_abyDisplayData[YO/8][XO] &= (~(1<<(YO%8)));
    }
    else
    {
//        trace_debug_printf("YO / 8 = %d,XO = %d\n\r",YO,XO);
        gs_abyDisplayData[YO/8][XO] |= (1<<(YO%8));
        if(LCDValue < LCD_AD_FLAG)
        {
            if(gs_PadStatus == PAD_STAT_SIGNNING)
            {
                sign_buffer[XO][YO] = 1;
            }
        }
    }
}

/****************************************************************************
  函数名     :  void Lib_LcdDrawPlot(BYTE XO,BYTE YO,BYTE Color)
  描述       :  画点函数
  输入参数   :  1、BYTE XO：列号，取值0至127
                2、BYTE YO：行号，取值0至63
                3、BYTE Color：点的颜色，0表示白色，非0表示黑色
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         修改
  1、 黄俊斌     2007-10-24  V1.1         增加输入参数合法性检查
****************************************************************************/
void Lib_LcdDrawPlot(BYTE XO, BYTE YO, BYTE Color)
{
    if (gs_byLcdBusyFlag)
    {
        return;
    }
    if(LCDValue < LCD_AD_FLAG)
    {
        if ((XO>131) || (YO>47))
        {
            return ;
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        if ((XO>121) || (YO>31))
        {
            return ;
        }   
    }
    gs_byLcdBusyFlag = 1;
    s_LcdDrawPlot(XO, YO, Color);
    s_LcdFlush();
    gs_byLcdBusyFlag = 0;
}

/****************************************************************************
  函数名     :  void Lib_LcdSetGray(BYTE level)
  描述       :  设置显示屏幕的对比度.
  输入参数   :  1、BYTE level：对比度级别[0~63，0最暗，63最亮]，默认值为44。
                其它值无操作。
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  
****************************************************************************/
void Lib_LcdSetGray(BYTE level)
{
    BYTE gray;
    if(level>(GRAY_MAX_VALUE-gs_byBaseGrayVal))
      level = GRAY_MAX_VALUE-gs_byBaseGrayVal;
    
    gray = gs_byBaseGrayVal + level;	
    if (gray < GRAY_MIN_VALUE)
    {
        gray = GRAY_MIN_VALUE;
    }
    else if (gray > GRAY_MAX_VALUE)
    {
        gray = GRAY_MAX_VALUE;
    }
    if (gs_byLcdBusyFlag)
    {
        return;
    }
    
    gs_byLcdBusyFlag = 1;
    s_LCD_WrCMD(0x81, LcdWrCmd);
    s_LCD_WrCMD(gray, LcdWrCmd);
    gs_byLcdBusyFlag = 0;
}

/****************************************************************************
  函数名     :  void Lib_LcdGotoxy(BYTE x, BYTE y)
  描述       :  将光标移到指定位置，输入参数非法将无操作
  输入参数   :  1、BYTE x：列标。 屏幕的水平坐标（0<=x<=127）
                2、BYTE y：行标。 屏幕的垂直坐标（0<=y<=63）
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         修改
  1、 黄俊斌     2007-10-24  V1.1         增加输入参数合法性检查
****************************************************************************/
void Lib_LcdGotoxy(BYTE x, BYTE y)
{
    if (gs_byLcdBusyFlag)
    {
        return;
    }
    if(LCDValue < LCD_AD_FLAG)
    {
        if ((x>131) || (y>47))
        {
            return;
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        if ((x>121) || (y>31))
        {
            return;
        }
    }
    gs_byLcdBusyFlag = 1;
    g_byStartY = y;
    g_byStartX = x;
    gs_byLcdBusyFlag = 0;
}

/****************************************************************************
  函数名     :  int Lib_LcdSetFont(BYTE AsciiFontHeight,
                           BYTE ExtendFontHeight, BYTE Zoom)
  描述       :  设置显示字体
  输入参数   :  1、BYTE AsciiFontHeight：ASCII显示字体高度，可取值8、12、16。
                2、BYTE ExtendFontHeight：扩展字体高度，可取值12、16
                3、BYTE Zoom：放大模式，目前不支持放大，该参数不使用
  备    注   ：(AsciiFontHeight,ExtendFontHeight)可取(8,16)、(12,12)和(16,16)
               三组值，其他组合的值都不正确
  输出参数   :  无
  返回值     :  0设置成功，-1500输入参数不正确
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         创建
****************************************************************************/
int Lib_LcdSetFont(BYTE AsciiFontHeight, BYTE ExtendFontHeight, BYTE Zoom)
{
    if ((16==ExtendFontHeight) && (8==AsciiFontHeight))
    {
        g_byFontType = 0;
        return 0;
    }
    if ((16==ExtendFontHeight) && (16==AsciiFontHeight))
    {
        g_byFontType = 1;
        return 0;
    }
    if ((12==ExtendFontHeight) && (12==AsciiFontHeight))
    {
        g_byFontType = 2;
        return 0;
    }
    return LCD_SetFont_Err;
}

/****************************************************************************
  函数名     :  int Lib_LcdGetFont(BYTE *AsciiFontHeight, BYTE *ExtendFontHeight, BYTE *Zoom)
  描述       :  获取当前显示格式
  输入参数   :  无
  输出参数   :  1、BYTE *AsciiFontHeight：ASCII字体字体大小
                2、BYTE *ExtendFontHeight：扩展文字字体大小，这里指的是汉字
                3、BYTE *Zoom：放大参数，这里永远输出0。
  备  注     :  (*AsciiFontHeight,*ExtendFontHeight)的可能输出值为(8,16)、(12,12)
                和(16,16)
  返回值     :  0表示成功，-1500表示失败
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         创建
****************************************************************************/
int Lib_LcdGetFont(BYTE *AsciiFontHeight, BYTE *ExtendFontHeight, BYTE *Zoom)
{
    if( (AsciiFontHeight==NULL) || (ExtendFontHeight==NULL) ||
        (Zoom==NULL))
      return LCD_GetFont_Err;
    *Zoom = 0;
    switch (g_byFontType)
    {
    case 0:
        *AsciiFontHeight = 8;
        *ExtendFontHeight = 16;
        return 0;
    case 1:
        *AsciiFontHeight = 16;
        *ExtendFontHeight = 16;
        return 0;
    case 2:
        *AsciiFontHeight = 12;
        *ExtendFontHeight = 12;
        return 0;
    default:
        return LCD_GetFont_Err;
    }
}

/****************************************************************************
  函数名     :  BYTE Lib_LcdSetAttr(BYTE attr)
  描述       :  设置显示属性
  输入参数   :  1、BYTE attr：显示属性。
                      0正常显示；
                      非0黑白反显示。
  输出参数   :  无
  返回值     :  原来的显示属性
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         创建
****************************************************************************/
BYTE Lib_LcdSetAttr(BYTE attr)
{
    BYTE tmp;
    tmp = g_byFontAttr;
    if (attr)
    {
        g_byFontAttr = 1;
    }
    else
    {
        g_byFontAttr = 0;
    }
    return tmp;
}

// 写页面地址子程序*********************************************************************
void set_page_address(u8 x)
{
    u8  page_temp;
    //AT skx
    //page_temp=x|0xb0;
    page_temp=(x&0x0f)|0xb0;
    s_LCD_WrCMD(page_temp,LcdWrCmd);
}

// 写列地址子程序*********************************************************************
void set_column_address(u8 x)
{
    u8  column_temp;
    column_temp=x%16;		//取低位列地址
    s_LCD_WrCMD(column_temp,LcdWrCmd);

    column_temp=x/16;		//取高位列地址
    column_temp=column_temp|0x10;
    s_LCD_WrCMD(column_temp,LcdWrCmd);
}

void Lcdarray_clear(u8 LineBit)
{
   u8 i;
           
        if(LineBit & 0x01)        
        {
          set_page_address(2);
          set_column_address(0);
          if(LCDValue < LCD_AD_FLAG)
          {
            for(i=0;i<132;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          }
          else if(LCDValue >= LCD_AD_FLAG)
          {
            for(i=0;i<122;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          }        
        }
        
        if(LineBit & 0x02) 
        {
          set_page_address(3);
          set_column_address(0);
          if(LCDValue < LCD_AD_FLAG)
          {
            for(i=0;i<132;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          } 
          else if(LCDValue >= LCD_AD_FLAG)
          {
            for(i=0;i<122;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          }        
        }
        
        if(LineBit & 0x04) 
        {
          set_page_address(4);
          set_column_address(0);
          if(LCDValue < LCD_AD_FLAG)
          {
            for(i=0;i<132;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          } 
          else if(LCDValue >= LCD_AD_FLAG)
          {
            for(i=0;i<122;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          }    
        }
        
        if(LineBit & 0x08) 
        {
          set_page_address(5);
          set_column_address(0);
          if(LCDValue < LCD_AD_FLAG)
          {
            for(i=0;i<132;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          }
          else if(LCDValue >= LCD_AD_FLAG)
          {
            for(i=0;i<122;i++)	
                s_LCD_WrCMD(0x00,LcdWrData); 
          }               
        }
}

/****************************************************************************
  函数名     :  void s_LcdFlush()
  描述       :  将缓冲区内的数据写入液晶屏
  输入参数   :  无
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-17   V1.0        创建
****************************************************************************/
void s_LcdFlush()
{
    BYTE i, line, byFlag;

    s_LCD_WrCMD(0xaf, 0);    //display on
    byFlag = 0;
    if(LCDValue < LCD_AD_FLAG)
    {
        for (line=0; line<6; line++)
        {
            for (i=0; i<132; i++)                           //列号//
            {
                if (gs_abyDisplayData[line][i] != gs_abyCurrentData[line][i])
                {
                    if (0 == byFlag)
                    {
                        //s_SetXY(i, line);
                        if(line >= 2) 
                        {
    #ifdef ST7567_M
                          s_SetXY(i,line);
    #else
                          s_SetXY(i,line+4);
    #endif                      
                        }
                        else s_SetXY(i,line);
                        s_LCD_WrCMD(gs_abyDisplayData[line][i], LcdWrData);
                    }
                    else
                    {
                        s_LCD_WrCMD(gs_abyDisplayData[line][i], LcdWrData);
                    }
                    gs_abyCurrentData[line][i] = gs_abyDisplayData[line][i];
                    byFlag = 1;
                }
                else
                {
                    byFlag = 0;
                }
            }
            byFlag = 0;
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        for (line=0; line<4; line++)
        {
            for (i=0; i<122; i++)                           //列号//
            {
                if (gs_abyDisplayData[line][i] != gs_abyCurrentData[line][i])
                {
                    if (0 == byFlag)
                    {
                        //s_SetXY(i, line);
                        if(line >= 2) 
                        {
    #ifdef ST7567_M
                          s_SetXY(i,line);
    #else
                          s_SetXY(i,line+4);
    #endif                      
                        }
                        else s_SetXY(i,line);
                        s_LCD_WrCMD(gs_abyDisplayData[line][i], LcdWrData);
                    }
                    else
                    {
                        s_LCD_WrCMD(gs_abyDisplayData[line][i], LcdWrData);
                    }
                    gs_abyCurrentData[line][i] = gs_abyDisplayData[line][i];
                    byFlag = 1;
                }
                else
                {
                    byFlag = 0;
                }
            }
            byFlag = 0;
        }
    }
}

/****************************************************************************
  函数名     :  void Lib_LcdClrLine(BYTE startline, BYTE endline)
  描述       :  清除屏幕上指定行，执行成功后光标会停在（0，endline）处，如果
                输入参数不正确，将无任何操作
  输入参数   :  1、BYTE startline：起始行号，取值0至31
                2、BYTE endline：结束行号，取值0至31
  备   注    :  输入的参数必须满足startline<=endline
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-18  V1.0         因显示方式改变而修改
  1、 黄俊斌     2007-10-24  V1.1         加强输入参数合法性检查
****************************************************************************/
void Lib_LcdClrLine(BYTE startline, BYTE endline)
{
    int i, j, k, l;
    
    if(LCDValue < LCD_AD_FLAG)
    {
        if ((startline>endline) || (startline>47))
        {
            return ;
        }
        if (endline > 47)
        {
            endline = 47;
        }
        if (gs_byLcdBusyFlag)
        {
            return;
        }
        gs_byLcdBusyFlag = 1;
    #if 0    
        for (i=startline/8; i<=endline/8; i++)
        {
          Lcdarray_clear(1<<i);
        }
    #endif
        
    //#if 0
        for (i=startline; i<=endline; i++)
        {
            k = i/8;
            j = i%8;
            for (l=0; l<132; l++)
            {
                gs_abyDisplayData[k][l] &= ~((BYTE)(1<<j));
            }
        }
        for (i=startline/8; i<=endline/8; i++)
        {
          Lcdarray_On(i,0,132,gs_abyDisplayData[i]);
        }
//#endif
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        if ((startline>endline) || (startline>31))
        {
            return ;
        }
        if (endline > 31)
        {
            endline = 31;
        }
        if (gs_byLcdBusyFlag)
        {
            return;
        }
        gs_byLcdBusyFlag = 1;
    #if 0    
        for (i=startline/8; i<=endline/8; i++)
        {
          Lcdarray_clear(1<<i);
        }
    #endif
        
    //#if 0
        for (i=startline; i<=endline; i++)
        {
            k = i/8;
            j = i%8;
            for (l=0; l<122; l++)
            {
                gs_abyDisplayData[k][l] &= ~((BYTE)(1<<j));
            }
        }
        for (i=startline/8; i<=endline/8; i++)
        {
          Lcdarray_On(i,0,122,gs_abyDisplayData[i]);
        }
//#endif
    }
    
    
    s_LcdFlush();

    
    g_byStartY = startline;
    g_byStartX = 0;
    gs_byLcdBusyFlag = 0;
}


int s_GetBackupGB2312DotMatrix(const BYTE *str, BYTE byFontHeight, BYTE *pbyCharDotMatrix)
{
    unsigned short i;
    unsigned short nVal;
    nVal = str[0]<<8|str[1];
    switch (byFontHeight)
    {
      case 12:
        for(i=0;i<MAX_SOFT_HZ_NUM;i++)
        {
          if(nVal ==tHz12X12[i].nHz)
          {
            memcpy(pbyCharDotMatrix,tHz12X12[i].sDot,18);
            return 0;
          }
        }
        return 2;

      default:
        return 1;
    }
}


int s_GetBackupDotMatrix(const BYTE *str, BYTE byFontHeight, BYTE *pbyCharDotMatrix)
{
    switch (byFontHeight)
    {
    case 8:
        if ((str[0]<0x20) || (str[0]>=0x80))
        {
            //memset(pbyCharDotMatrix, 0, 6);
            memset(pbyCharDotMatrix, 0, 8);
        }
        else
        {
            memcpy(pbyCharDotMatrix, &Ch8ModAddr[(str[0]-0x20)*6], 6);
            //memcpy(pbyCharDotMatrix, &ASCII8X8[(str[0]-0x20)*8], 8);
        }
        return 0;
#if 0        
    case 16:
        if ((str[0]<0x20) || (str[0]>=0x80))
        {
            memset(pbyCharDotMatrix, 0, 16);
        }
        else
        {
            memcpy(pbyCharDotMatrix, &Ch16ModAddr[(str[0]-0x20)*16], 16);
        }
        return 0;
#endif        
    default:
        return 1;
    }
}

/****************************************************************************
  函数名     :  int GetCharDotMatrix(BYTE *str, BYTE byCharSet, BYTE byFontHeight,
                     BYTE *pbyCharDotMatrix)
  描述       :  获取字库的字符点阵信息
  输入参数   :  1、char *str：字符，当byCharSet=ASCII_FONT时，str长度为1
                                    当byCharSet=GB2312_FONT时，str长度为2
                2、BYTE byCharSet：字符编码，ASCII_FONT/GB2312_FONT
                3、BYTE byFontHeight：字符的高度，取值8、12、16、24，
                   其中ASCII 可取值 8、16和24，
                       GB2312可取值12、16和24。
  输出参数   :  1、BYTE *pbyCharDotMatrix：字符的点阵信息
  返回值     :  0：成功，1：输入参数错误，2：没有该点阵信息，3：没有字库文件
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-14   V1.0        创建
****************************************************************************/
int GetCharDotMatrix(const BYTE *str, BYTE byCharSet, BYTE byFontHeight,
                     BYTE *pbyCharDotMatrix)
{
    if (1 != g_iHaveFontFlag)
    {
        if (ASCII_FONT == byCharSet)
        {
            return s_GetBackupDotMatrix(str, byFontHeight, pbyCharDotMatrix);
        }
        else if(byCharSet==GB2312_FONT)
        {
            return s_GetBackupGB2312DotMatrix(str, byFontHeight, pbyCharDotMatrix);
          
        }
        return 3;
    }
#if 0
    if ( (1==g_sVFontLibHead.byLangeType) && (0==g_sVFontLibHead.byLangeCodeType) )
    {
        // 处理中文GB2312字库
        switch (byCharSet)
        {
        case ASCII_FONT:
            return s_GetAsciiDotMatrix(str, byFontHeight, pbyCharDotMatrix);
        case GB2312_FONT:
            return s_GetGB2312DotMatrix(str, byFontHeight, pbyCharDotMatrix);
        default:
            return 1;
        }
    }
#endif
    
    return 1;
}

/****************************************************************************
  函数名     :  void s_InitDisplayBuffer(BYTE *pbyBuff, BYTE byCharSet, BYTE byHeight)
  描述       :  获取没有字库时的文字点阵
  输入参数   :  1、BYTE byCharSet：字符集，0表示ASCII，其他值表示扩展字符
                2、BYTE byHeight：字符高度，可取值为8、12和16
  输出参数   :  1、BYTE *pbyBuff：点阵数据
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-07   V1.0        创建
****************************************************************************/
void s_InitDisplayBuffer(BYTE *pbyBuff, BYTE byCharSet, BYTE byHeight)
{
    int i;
    if (ASCII_FONT == byCharSet)
    {
        switch (byHeight)
        {
        case 8:  // 6X8
            for (i=0; i<6; i+=2)
            {
                pbyBuff[i] = 0xaa;
            }
            for (i=1; i<6; i+=2)
            {
                pbyBuff[i] = 0x55;
            }
            return ;
        case 12:   // 6X12
            for (i=0; i<6; i+=2)
            {
                pbyBuff[i] = 0xaa;
                pbyBuff[i+6] = 0xaa;
            }
            for (i=1; i<6; i+=2)
            {
                pbyBuff[i] = 0x55;
                pbyBuff[i+6] = 0x55;
            }
            return;
        default:  // 8X16
            for (i=0; i<8; i+=2)
            {
                pbyBuff[i] = 0xaa;
                pbyBuff[i+8] = 0xaa;
            }
            for (i=1; i<8; i+=2)
            {
                pbyBuff[i] = 0x55;
                pbyBuff[i+8] = 0x55;
            }
            return;
        }
    }
    else
    {
        switch (byHeight)
        {
        case 12:  // 12X12
            for (i=0; i<12; i+=2)
            {
                pbyBuff[i] = 0xaa;
                pbyBuff[i+12] = 0xaa;
            }
            for (i=1; i<12; i+=2)
            {
                pbyBuff[i] = 0x55;
                pbyBuff[i+12] = 0x55;
            }
            break;
        default:  // 16X16
            for (i=0; i<16; i+=2)
            {
                pbyBuff[i] = 0xaa;
                pbyBuff[i+16] = 0xaa;
            }
            for (i=1; i<16; i+=2)
            {
                pbyBuff[i] = 0x55;
                pbyBuff[i+16] = 0x55;
            }
            break;
        }
    }
}

/****************************************************************************
  函数名     :  void s_LcdWrStrHeight8(BYTE *str, BYTE strLen)
  描述       :  在屏幕上显示高度为8像素的字符串，内部使用，如果有汉字，汉字
                按高度为16像素显示
  输入参数   :  1、BYTE *str：要显示的字符串
                2、BYTE strLen：字符串的长度
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-07   V1.0        创建
****************************************************************************/
void s_LcdWrStrHeight8(BYTE *str, BYTE strLen)
{
    BYTE byFontHeight = 8;
    BYTE i;
    int iRet;
    BYTE strBuffer[40];

    for (i=0; i<strLen; i++)
    {
        if (str[i] >= 0x80)
        {
            byFontHeight = 16;  // 如果有汉字，则汉字按16X16显示，ASCII仍然按6X8显示
            break;
        }
    }

    i = 0;
    if(LCDValue < LCD_AD_FLAG)
    {
        while (i < strLen)
        {
            if ((g_byStartX+6) > LCD_WIDTH)
            {
                g_byStartX = 0;
                g_byStartY += byFontHeight;
            }
            if ((g_byStartY+byFontHeight) > LCD_HIGH)
            {
                g_byStartX = 0;
                g_byStartY = 0;
            }
            if ((0x0a==str[i]) || (0x0d==str[i]))
            {
                g_byStartX = 0;
                g_byStartY += byFontHeight;
                i++;
                continue;
            }
            if ((str[i]<0x20) || (0x80==str[i]) || (0xff==str[i]))
            {
                memset(strBuffer, 0, sizeof(strBuffer));
                strBuffer[0] = 6;
                strBuffer[1] = byFontHeight;
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 6;
                i++;
                continue;
            }
            memset(strBuffer, 0, sizeof(strBuffer));
            if (str[i] < 0x80)
            {
                if (16 == byFontHeight)
                {
                    strBuffer[0] = 6;
                    strBuffer[1] = 16;
                    iRet = GetCharDotMatrix(&str[i], ASCII_FONT, 8, &strBuffer[8]);
                }
                else
                {
                    strBuffer[0] = 6;
                    strBuffer[1] = 8;
                    iRet = GetCharDotMatrix(&str[i], ASCII_FONT, 8, &strBuffer[2]);
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 6;
                i++;
                continue;
            }
            else  // str[i]>0x80  && str[i]<0xff
            {
                if ((g_byStartX+16) > LCD_WIDTH)
                {
                    g_byStartX = 0;
                    g_byStartY += 16;
                }
                if ((g_byStartY+16) > LCD_HIGH)
                {
                    g_byStartY = 0;
                }
                strBuffer[0] = 16;
                strBuffer[1] = 16;
                if ((str[i+1]<0x40) || (0x7f==str[i+1]) || (0xff==str[i+1]))
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 16);
                    s_LcdDrawLogo(strBuffer);
                    g_byStartX += 16;
                    i += 2;
                    continue;
                }
    
                iRet = GetCharDotMatrix(&str[i], GB2312_FONT, 16, &strBuffer[2]);
                if (0 != iRet)
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 16);
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 16;
                i += 2;
                continue;
            }
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        while (i < strLen)
        {
            if ((g_byStartX+6) > LCD_WIDTH_OLD)
            {
                g_byStartX = 0;
                g_byStartY += byFontHeight;
            }
            if ((g_byStartY+byFontHeight) > LCD_HIGH_OLD)
            {
                g_byStartX = 0;
                g_byStartY = 0;
            }
            if ((0x0a==str[i]) || (0x0d==str[i]))
            {
                g_byStartX = 0;
                g_byStartY += byFontHeight;
                i++;
                continue;
            }
            if ((str[i]<0x20) || (0x80==str[i]) || (0xff==str[i]))
            {
                memset(strBuffer, 0, sizeof(strBuffer));
                strBuffer[0] = 6;
                strBuffer[1] = byFontHeight;
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 6;
                i++;
                continue;
            }
            memset(strBuffer, 0, sizeof(strBuffer));
            if (str[i] < 0x80)
            {
                if (16 == byFontHeight)
                {
                    strBuffer[0] = 6;
                    strBuffer[1] = 16;
                    iRet = GetCharDotMatrix(&str[i], ASCII_FONT, 8, &strBuffer[8]);
                }
                else
                {
                    strBuffer[0] = 6;
                    strBuffer[1] = 8;
                    iRet = GetCharDotMatrix(&str[i], ASCII_FONT, 8, &strBuffer[2]);
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 6;
                i++;
                continue;
            }
            else  // str[i]>0x80  && str[i]<0xff
            {
                if ((g_byStartX+16) > LCD_WIDTH_OLD)
                {
                    g_byStartX = 0;
                    g_byStartY += 16;
                }
                if ((g_byStartY+16) > LCD_HIGH_OLD)
                {
                    g_byStartY = 0;
                }
                strBuffer[0] = 16;
                strBuffer[1] = 16;
                if ((str[i+1]<0x40) || (0x7f==str[i+1]) || (0xff==str[i+1]))
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 16);
                    s_LcdDrawLogo(strBuffer);
                    g_byStartX += 16;
                    i += 2;
                    continue;
                }
    
                iRet = GetCharDotMatrix(&str[i], GB2312_FONT, 16, &strBuffer[2]);
                if (0 != iRet)
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 16);
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 16;
                i += 2;
                continue;
            }
        }
    }      
}

/****************************************************************************
  函数名     :  void s_LcdWrStrHeight12(BYTE *str, BYTE strLen)
  描述       :  在屏幕上显示高度为12像素的字符串，内部使用
  输入参数   :  1、BYTE *str：要显示的字符串
                2、BYTE strLen：字符串的长度
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-07   V1.0        创建
****************************************************************************/
void s_LcdWrStrHeight12(BYTE *str, BYTE strLen)
{
    BYTE i, j;
    int iRet;
    BYTE strBuffer[26], strTemp[18];
    i = 0;
    
    if(LCDValue < LCD_AD_FLAG)
    {
        while (i < strLen)
        {
            if ((g_byStartX+6) > LCD_WIDTH)
            {
                g_byStartX = 0;
                g_byStartY += 12;
            }
            if ((g_byStartY+12) > LCD_HIGH)
            {
                g_byStartX = 0;
                g_byStartY = 0;
            }
            if ((0x0a==str[i]) || (0x0d==str[i]))
            {
                g_byStartX = 0;
                g_byStartY += 12;
                i++;
                continue;
            }
            memset(strBuffer, 0, sizeof(strBuffer));
            if ((str[i]<0x20) || (0x80==str[i]) || (0xff==str[i]))
            {
                strBuffer[0] = 6;
                strBuffer[1] = 12;
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 8;
                i++;
                continue;
            }
            else if (str[i] < 0x80)
            {
                strBuffer[0] = 6;
                strBuffer[1] = 12;
                iRet = GetCharDotMatrix(&str[i], ASCII_FONT, 8, strTemp);
                for (j=0; j<6; j++)
                {
                    strBuffer[2+j] = (strTemp[j]<<2);
                    strBuffer[8+j] = (strTemp[j]>>6);
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 6;
                i++;
                continue;
            }
            else  // str[i]>0x80  && str[i]<0xff
            {
                if ((g_byStartX+12) > LCD_WIDTH)
                {
                    g_byStartX = 0;
                    g_byStartY += 12;
                }
                if ((g_byStartY+12) > LCD_HIGH)
                {
                    g_byStartY = 0;
                }
                memset(strBuffer, 0, sizeof(strBuffer));
                strBuffer[0] = 12;
                strBuffer[1] = 12;
                if ((str[i+1]<0x40) || (0x7f==str[i+1]) || (0xff==str[i+1]))
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 12);
                    s_LcdDrawLogo(strBuffer);
                    g_byStartX += 12;
                    i += 2;
                    continue;
                }
    
                iRet = GetCharDotMatrix(&str[i], GB2312_FONT, 12, strTemp);
                if (0 != iRet)
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 12);
                }
                else
                {
                    memcpy(&strBuffer[2], strTemp, 12);
                    for (j=0; j<6; j++)
                    {
                        strBuffer[14+j*2  ] = (strTemp[12+j]&0x0f);
                        strBuffer[14+j*2+1] = (strTemp[12+j]&0xf0)>>4;
                    }
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 12;
                i += 2;
                continue;
            }
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        while (i < strLen)
        {
            if ((g_byStartX+6) > LCD_WIDTH_OLD)
            {
                g_byStartX = 0;
                g_byStartY += 12;
            }
            if ((g_byStartY+12) > LCD_HIGH_OLD)
            {
                g_byStartX = 0;
                g_byStartY = 0;
            }
            if ((0x0a==str[i]) || (0x0d==str[i]))
            {
                g_byStartX = 0;
                g_byStartY += 12;
                i++;
                continue;
            }
            memset(strBuffer, 0, sizeof(strBuffer));
            if ((str[i]<0x20) || (0x80==str[i]) || (0xff==str[i]))
            {
                strBuffer[0] = 6;
                strBuffer[1] = 12;
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 8;
                i++;
                continue;
            }
            else if (str[i] < 0x80)
            {
                strBuffer[0] = 6;
                strBuffer[1] = 12;
                iRet = GetCharDotMatrix(&str[i], ASCII_FONT, 8, strTemp);
                for (j=0; j<6; j++)
                {
                    strBuffer[2+j] = (strTemp[j]<<2);
                    strBuffer[8+j] = (strTemp[j]>>6);
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 6;
                i++;
                continue;
            }
            else  // str[i]>0x80  && str[i]<0xff
            {
                if ((g_byStartX+12) > LCD_WIDTH_OLD)
                {
                    g_byStartX = 0;
                    g_byStartY += 12;
                }
                if ((g_byStartY+12) > LCD_HIGH_OLD)
                {
                    g_byStartY = 0;
                }
                memset(strBuffer, 0, sizeof(strBuffer));
                strBuffer[0] = 12;
                strBuffer[1] = 12;
                if ((str[i+1]<0x40) || (0x7f==str[i+1]) || (0xff==str[i+1]))
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 12);
                    s_LcdDrawLogo(strBuffer);
                    g_byStartX += 12;
                    i += 2;
                    continue;
                }
    
                iRet = GetCharDotMatrix(&str[i], GB2312_FONT, 12, strTemp);
                if (0 != iRet)
                {
                    s_InitDisplayBuffer(&strBuffer[2], GB2312_FONT, 12);
                }
                else
                {
                    memcpy(&strBuffer[2], strTemp, 12);
                    for (j=0; j<6; j++)
                    {
                        strBuffer[14+j*2  ] = (strTemp[12+j]&0x0f);
                        strBuffer[14+j*2+1] = (strTemp[12+j]&0xf0)>>4;
                    }
                }
                s_LcdDrawLogo(strBuffer);
                g_byStartX += 12;
                i += 2;
                continue;
            }
        }
    }
}

/****************************************************************************
  函数名     :  void s_LcdWrStr(BYTE *str, BYTE strLen)
  描述       :  在屏幕上显示字符串，内部使用
  输入参数   :  1、BYTE *str：要显示的字符串
                2、BYTE strLen：字符串的长度
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-07   V1.0        创建
****************************************************************************/
void s_LcdWrStr(BYTE *str, BYTE strLen)
{
    switch (g_byFontType)
    {
    case 0:
        s_LcdWrStrHeight8(str, strLen);
        break;
#if 0        
    case 1:
        s_LcdWrStrHeight16(str, strLen);
        break;
#endif
        
    default:
        s_LcdWrStrHeight12(str, strLen);
        break;
    }
}

#if 0
u8 LcdReadData(void)
{
  u16   GpioData;
  
  //AT skx
  //SetLcdA0_L();                 //WaitNuS(2); 
  //SetLcdWR_H();                 //WaitNuS(2);   
  SetLcdA0_H();                 //WaitNuS(2);       //Raising
  SetLcdCS_L();                 //WaitNuS(2);
  SetLcdWR_H();                 //WaitNuS(2);//AT skx
  SetLcdRD_L();                 //WaitNuS(2); 
  WaitNuS(1);//for 140 ns hold L,80 ns hold H
  GpioData = GPIO_ReadOutputData(GPIOC);
  //SetLcdWR_H();                 WaitNuS(2);
  SetLcdRD_H();                   //WaitNuS(2);
  SetLcdCS_H();                   //WaitNuS(2);    
  //SetLcdA0_H();                 //WaitNuS(2);       //Keep high

  return((u8)GpioData);
}


void LcdReadarray_On(u8 LineX,u8 LineY,u8 ByteLength, u8 *NewLcdData)
{
   u8 i,bLine;
   switch(LineX)
   {
     case 0:
       bLine = 0;
       break;
     case 1:
       bLine = 1;
       break;
     case 2:
#ifdef ST7567_M
       bLine = 2;
#else       
       bLine = 6;
#endif       
       break;
     case 3:
#ifdef ST7567_M
       bLine = 3;
#else       
       bLine = 7;
#endif       
       break;
     case 4:
       bLine = 5;
       break;
     case 5:
       bLine = 4;
       break;
     case 6:
       bLine = 3;
       break;
     case 7:
       bLine = 2;
       break;
     default:
       bLine = 7;
       break;
       
   }
	//set_page_address(LineX+2);
	set_page_address(bLine);
	set_column_address(LineY);
	for(i=0;i<ByteLength;i++)	
          //s_LCD_WrCMD(NewLcdData[i],LcdWrData);     
          NewLcdData[i] = LcdReadData();
}
#endif

void Lcdarray_On(u8 LineX,u8 LineY,u8 ByteLength, u8 *NewLcdData)
{
   u8 i,bLine;
   switch(LineX)
   {
     case 0:
       bLine = 0;
       break;
     case 1:
       bLine = 1;
       break;
     case 2:
       bLine = 6;
       break;
     case 3:
       bLine = 7;
       break;
     case 4:
       bLine = 5;
       break;
     case 5:
       bLine = 4;
       break;
     case 6:
       bLine = 3;
       break;
     case 7:
       bLine = 2;
       break;
     default:
       bLine = 7;
       break;
       
   }
	//set_page_address(LineX+2);
	set_page_address(bLine);
	set_column_address(LineY);
	for(i=0;i<ByteLength;i++)	s_LCD_WrCMD(NewLcdData[i],LcdWrData);     
}

/****************************************************************************
  函数名     :  void s_LcdDrawLogo(BYTE *pDataBuffer)
  描述       :  画标志图函数，画标志图之前必须设置画图起始位置，标志图的过程
                中并不会改变画图的起始位置
  输入参数   :  1、BYTE *pDataBuffer：输入的数据，前两个字节是标志图的宽和高，
                   后面是标志图的点阵数据
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-19  V1.0         修改
****************************************************************************/
void s_LcdDrawLogo(BYTE *pDataBuffer)
{
    BYTE i, j, byHigh, byWidth, lineOffset, bitOffset, line, bit, byTemp;
//    BYTE bOld=0xff,abyDisplayData[122];
    
    byWidth = pDataBuffer[0];
    byHigh = pDataBuffer[1];
   
    if(LCDValue < LCD_AD_FLAG)
    {
        for (j=0; j<byHigh; j++)
        {
    //        trace_debug_printf("\r byHigh = %d j=%d\r",byHigh,j);
            if ((g_byStartY+j) > 47)
            {
                return ;  // 纵向溢出部分不处理
            }
            lineOffset = j/8;
            bitOffset = j%8;
            
            //针对中间有断层 >12
        //    if(byHigh==12 && (g_byStartY>11 && g_byStartY<16))
         //   {
        //      g_byStartY=16;
        //    }
             
            line = (g_byStartY+j)/8;
            bit = (g_byStartY+j)%8;
           
                        
            
            for (i=0; i<byWidth; i++)
            {
    //            trace_debug_printf("i = %dtest\n\r",i);
                if ((g_byStartX+i) > 132)
                {
                    break;  // 横向溢出部分不处理
                }
                byTemp = pDataBuffer[2+lineOffset*byWidth+i]&(1<<bitOffset);
                if (((0==byTemp)&&(0==g_byFontAttr)) || ((0!=byTemp)&&(0!=g_byFontAttr)))
                {
                    gs_abyDisplayData[line][g_byStartX+i] &= ~(1<<bit);
                }
                else
                {
                    gs_abyDisplayData[line][g_byStartX+i] |= (1<<bit); 
                    if(gs_PadStatus == PAD_STAT_SIGNNING)
                    {
                        sign_buffer[g_byStartX+i][g_byStartY+j] = 1;
                    }                
                    //trace_debug_printf("g_byStartX+i =%d,line = %d,\n\r",g_byStartX+i,g_byStartY+j);
                }
            }
           
            
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        for (j=0; j<byHigh; j++)
        {
    //        trace_debug_printf("\r byHigh = %d j=%d\r",byHigh,j);
            if ((g_byStartY+j) > 31)
            {
                return ;  // 纵向溢出部分不处理
            }
            lineOffset = j/8;
            bitOffset = j%8;
            
            //针对中间有断层 >12
            if(byHigh==12 && (g_byStartY>11 && g_byStartY<16))
            {
              g_byStartY=16;
            }
             
            line = (g_byStartY+j)/8;
            bit = (g_byStartY+j)%8;
           
                        
            
            for (i=0; i<byWidth; i++)
            {
    //            trace_debug_printf("i = %dtest\n\r",i);
                if ((g_byStartX+i) > 121)
                {
                    break;  // 横向溢出部分不处理
                }
                byTemp = pDataBuffer[2+lineOffset*byWidth+i]&(1<<bitOffset);
                if (((0==byTemp)&&(0==g_byFontAttr)) || ((0!=byTemp)&&(0!=g_byFontAttr)))
                {
                    gs_abyDisplayData[line][g_byStartX+i] &= ~(1<<bit);
                }
                else
                {
                    gs_abyDisplayData[line][g_byStartX+i] |= (1<<bit); 
                             
                }
            }
           
            
        }
    }
    
}


/****************************************************************************
  函数名     :  int Lib_Lcdprintf(char *fmt,...)
  描述       :  在液晶屏上打印字符串，输入格式请参考C语言的printf()函数，调用
                该函数后，液晶屏上的光标位置停留在该打印函数打印的最后一个字
                符的后边
  输入参数   :  1、char *fmt,...：要显示的格式字符串
  输出参数   :  无
  返回值     :  返回在屏幕上显示的字符的个数
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-19  V1.0         修改
****************************************************************************/
int Lib_Lcdprintf(char *fmt,...)
{
    int i;
    char sbuffer[512];
    va_list varg;

    if (0 != gs_byLcdBusyFlag)
    {
        return 0;
    }
    if(strlen(fmt)>sizeof(sbuffer))
      return 0;
    
    gs_byLcdBusyFlag = 1;
    memset(sbuffer, 0, sizeof(sbuffer));
    
    va_start(varg, fmt);
    //i = myvsprintf(sbuffer, fmt, varg);
    i = vsprintf(sbuffer, fmt, varg);
    va_end(varg);
    if( strlen((char*)sbuffer)>sizeof(sbuffer))
      s_LcdWrStr((BYTE*)sbuffer, (BYTE)sizeof(sbuffer));
    else
      s_LcdWrStr((BYTE*)sbuffer, strlen((char*)sbuffer));
    
    s_LcdFlush();
    
    //s_ScrSetIcon();
    gs_byLcdBusyFlag = 0;
    return i;
}

/****************************************************************************
  函数名     :  void Lib_LcdPrintxy(BYTE col, BYTE row, BYTE mode, char *str,...)
  描述       :  在指定位置打印指定格式的字符串，当输入值非法的时候无任何操作
  输入参数   :  1、BYTE col：列号，取值范围是0~131
                2、BYTE row：行号，取值范围是0~47
                3、BYTE mode：显示模式，可取值0x00、0x01、0x02、0x80、0x81和
                   0x82，其他值非法
                4、char *str,...：要在屏幕上显示的格式字符串
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-19  V1.0         修改
  1、 黄俊斌     2007-10-24  V1.1         增加输入参数合法性检查
****************************************************************************/
void Lib_LcdPrintxy(BYTE col, BYTE row, BYTE mode, char *str,...)
{
    BYTE old_fonttype, old_fontattr;
    BYTE old_lcdX, old_lcdY;
    char sbuffer[512];
    va_list varg;

//    trace_debug_printf("LCDValue = %d\n\r",LCDValue);
    if(LCDValue < LCD_AD_FLAG)
    {
        if ((col>131) || (row>47))
        {
            return ;
        }
    }
    else  if(LCDValue >= LCD_AD_FLAG)
    {
        if ((col>121) || (row>31))
        {
            return ;
        }
    }
    if (0 != (mode&0x7c))
    {
        return ;
    }
    if (0x03 == (mode&0x03))
    {
        return ;
    }

    if (gs_byLcdBusyFlag)
        return;
    if(strlen(str)>sizeof(sbuffer))
      return;
    
    memset(sbuffer, 0, sizeof(sbuffer));
    va_start(varg, str);
    //(void)myvsprintf(sbuffer, str, varg);
    (void)vsprintf(sbuffer, str, varg);
    va_end(varg);

    gs_byLcdBusyFlag = 1;
    old_lcdX = g_byStartX;
    old_lcdY = g_byStartY;
    old_fonttype = g_byFontType;
    old_fontattr = g_byFontAttr;
    g_byStartX = col;
    g_byStartY = row;

    g_byFontType = mode&0x03;
    if (g_byFontType > 2)
    {
        g_byFontType = 1;
    }
    if (0 != (mode&0x80))
    {
        g_byFontAttr = 1;
    }
    else
    {
        g_byFontAttr = 0;
    }
    s_LcdWrStr((BYTE *)sbuffer, strlen((char *)sbuffer));
    
    s_LcdFlush();

    
    g_byStartX = old_lcdX;
    g_byStartY = old_lcdY;
    g_byFontType = old_fonttype;
    g_byFontAttr = old_fontattr;
    //s_ScrSetIcon();
    gs_byLcdBusyFlag = 0;

}

/****************************************************************************
  函数名     :  void Lib_LcdSetBackLight(BYTE mode)
  描述       :  设置屏幕背光（在有按键，刷卡，插卡时自动亮）
  输入参数   :  1、BYTE mode：背光显示模式。
                     0 关背光
                     1 背光保持亮1分钟（在1分钟后，自动关闭），默认值
                     2 背光常亮
                     其它值无操作
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
extern int s_OpenSysTimer(void);
void Lib_LcdSetBackLight(BYTE mode)
{
#if 0    
    HAL_PWM_CONFIG  params;
    int devnum; 
   
    params.frequency    = 1000;//BeepFreqTab[mode%7+1]+500;//-100; //200  400   2000=fail
    params.pulse        = 99;
    devnum=hal_pwm_dev0; 
    hal_pwm_init(devnum, (HAL_PWM_CONFIG *)&params );  

    if(mode)
    {
		hal_pwm_stop(devnum); 
    }
    else
    {
		hal_pwm_run(devnum ); 
    } 
    k_LcdBackLightMode=mode;
    if(mode==1)
	{
        k_LcdBackLightTime=30;//1 minute
        s_OpenSysTimer();
    }
#endif
    if(mode>2)
      return;
    LcdBL_Control(mode);   
    
    k_LcdBackLightMode=mode;
    if(mode==1)
    {
        k_LcdBackLightTime=6000;//1 minute=60000 ms=6000x10
        //s_OpenSysTimer();
    }
    
}

void Lib_LcdPrintxyCE(BYTE col, BYTE row, BYTE mode, char *pCHN , char *pEN)
{
#if 0  
    if (g_iChineseFontFlag)
    {
        Lib_LcdPrintxy(col, row, mode, pCHN);
    }
    else
#endif      
    {
        Lib_LcdPrintxy(col, row, mode, pEN);
    }
}

/****************************************************************************
  函数名     :  void Lib_LcdGetSize(uchar * x, uchar *y)
  描述       :  读取显示屏属性，返回横与纵向点像素
  输入参数   :  无
  输出参数   :  1、BYTE *x：当前显示屏的宽度
                2、BYTE *y：当前显示屏的高度
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void Lib_LcdGetSize(BYTE *x, BYTE *y)
{
    if(x!=NULL)
    {
        if(LCDValue < LCD_AD_FLAG)
            *x = LCD_WIDTH;
        else if(LCDValue >= LCD_AD_FLAG)
            *x = LCD_WIDTH_OLD;
    }
    if(y!=NULL)
    {
        if(LCDValue < LCD_AD_FLAG)
            *y = LCD_HIGH;
        else if(LCDValue >= LCD_AD_FLAG)
            *y = LCD_HIGH_OLD;
    }
}


int TestLcd(void)
{
  
/*
	int iFlag = 1;
    int i = 0;

    Lib_LcdCls();
    Lib_LcdGotoxy(0, 0);
    Lib_Lcdprintf("背光测试");
    while (1)
    {
        Lib_LcdGotoxy(0, 32);
        if (0 != iFlag)
        {
            Lib_Lcdprintf("开背光=%d", i);
            Lib_LcdSetBackLight(2);
            iFlag = 0;
        }
        else
        {
            Lib_Lcdprintf("关背光=%d", i);
            Lib_LcdSetBackLight(0);
            iFlag = 1;
        }
        Lib_KbGetCh();
        i++;
    }
	*/

	uchar EnMenu[5][16] = {
									 "可Uart Test    ", 
									 //"Uart Test    ", 
									 "USB有 Test     ", 
									 //"USB Test     ", 
									 "Modem Test   ",
				 					 "GPRS Test     ",
				 					 "LAN Test      "
								 };

    //BYTE ch;
    //int i = 0, j;
	//static char count = 0;
   // BYTE strTemp[200] = {0xc4,0xbc,0x07,0xc4,0x3c,0x21,0x21,0xf9,0x25,0x23,0x20,0x00,0x04,0x02,0x01,0x01,
//0x02,0x00,0x04,0x07,0x00,0x00,0x00,0x00};/*"好",0*/
    /*unsigned char hao0[]={ 
    0x04,0x0, 0xFF,0xE, 0x1F,0x8, 0x10,0x8, 
    0xFF,0xE, 0x80,0x2, 0x3F,0xC, 0x54,0x8, 
    0x2B,0x0, 0x12,0x8, 0x6E,0x6, 0x00,0x0 
    }; */

    //Lib_LcdCls();

    //test
    Lib_LcdCls();
//    Lcdarray_On(0,0,24-12,strTemp);
//    Lcdarray_On(1,0,12,&strTemp[12]);
    
    Lib_LcdPrintxy(0,0,2, (char *)EnMenu[0]);
    Lib_LcdPrintxy(0,12*1,2, (char *)EnMenu[1]);

    Lib_LcdPrintxy(0,0,0, (char *)EnMenu[0]);
    Lib_LcdPrintxy(0,8*1,0, (char *)EnMenu[1]);
    
    Lib_LcdPrintxy(0,8*2,0, (char *)EnMenu[2]);
    Lib_LcdPrintxy(0,8*3,0, (char *)EnMenu[3]);
#if 0    
    strTemp[0] = 20;
    strTemp[1] = 17;
    
    
    for (i=0; i<20; i++)
    {
        if (0 == (i%2))
        {
            strTemp[2+i] = 0x3f;
            strTemp[2+20+i] = 0xf3;
            strTemp[2+40+i] = 0xf3;
        }
        else
        {
            strTemp[2+i] = 0x51;
            strTemp[2+20+i] = 0x79;
            strTemp[2+40+i] = 0x7f;
        }
    }
	count =0;
    while(1)
    {
        s_UartPrint(COM_DEBUG,"Lcd display Test...\r\n");
        s_UartPrint(COM_DEBUG,"==================================\r\n");
        s_UartPrint(COM_DEBUG,"1-Printf()                        \r\n");
        s_UartPrint(COM_DEBUG,"2-Gray()                          \r\n");
        s_UartPrint(COM_DEBUG,"3-Backlight()       0-init()      \r\n");
        s_UartPrint(COM_DEBUG,"==================================\r\n");

        ch=s_getkey(COM_DEBUG);
        //if(ch=='3')     Lib_LcdSetBackLight(0);
        //else            Lib_LcdSetBackLight(1);
        switch(ch)
        {
        case '0':
			/*
            Lib_LcdCls();
            Lib_LcdGotoxy(33, 3);
            Lib_LcdSetFont(8, 16, 0);
            Lib_Lcdprintf("显示中文abc");
            Lib_LcdGotoxy(3, 27);
            Lib_LcdSetFont(16, 16, 0);
            Lib_Lcdprintf("显示中文abc");
            Lib_LcdGotoxy(13, 50);
            Lib_LcdSetFont(12, 12, 0);
            Lib_Lcdprintf("显示中文abc");
			*/
            s_UartPrint(COM_DEBUG,"s_lcdinit(s)\n");
			s_LcdInit(); 
			s_LcdSetAll();
			Lib_LcdSetBackLight(0);
			s_UartPrint(COM_DEBUG,"s_lcdinit(e)\n");
            break;
        case '1':
            //Lib_LcdCls();
            //Lib_LcdGotoxy(0, 2);
            //Lib_LcdSetFont(12, 12, 0);
            //s_Lcdprintf("abcedefadfka;df asdkfa;ldkfjasdkfjasldkfja;ldjf;lasdfkasd");
			j = s_ChineseSelectItem((unsigned char*)EnMenu, 5, 1);
			s_UartPrint(0, "\n\rreturn %d\n\r", j);
            break;
        case '2':
            Lib_LcdCls();
            s_LcdPrintxy(0,0,0x81, "dsss上海");
            s_LcdPrintxy(22,23,0x82, "dsss上海");
            s_LcdPrintxy(42,40,0x80, "dsss上海");
            break;
        case '3':
            //Lib_LcdCls();
            //s_LcdPrintxy(00,00,0x01, "dsss上海dsss上海dsss上海dsss上海");
            //s_LcdPrintxy(22,23,0x02, "dsss上海");
            //s_LcdPrintxy(42,40,0x00, "dsss上海");
            break;
        case '4':
            Lib_LcdCls();
            for (i=0; i<=40; i++)
            {
                Lib_LcdDrawPlot(i, i, 1);
                Lib_LcdDrawPlot(40-i, i, 1);
                Lib_LcdDrawPlot(0, i, 1);
                Lib_LcdDrawPlot(i, 0, 1);
                Lib_LcdDrawPlot(40, i, 1);
                Lib_LcdDrawPlot(i, 40, 1);
            }
            break;
        case '5':
//            Lib_LcdCls();
//            Lib_LcdGotoxy(0, 0);
//            s_LcdDrawLogo(spp20);
 //           s_LcdFlush();
//            Lib_LcdGotoxy(0, 0);
//            s_LcdDrawLogo(strTemp);
//            Lib_LcdDrawLine(0, 0, 127, 63, 1);
//            Lib_LcdDrawLine(0, 0, 64, 127, 1);
//            s_LcdFlush();
            //Lib_LcdDrawBox(1, 0, 7, 9);
            break;
        case '6':
            Lib_LcdCls();
            Lib_LcdDrawLine(0, 63, 127, 0, 1);
//            Lib_LcdGotoxy(3, 30);
//            s_LcdDrawLogo(strTemp);
//            s_LcdFlush();
            //Lib_LcdDrawBox(0, 0, 25, 54);
            break;
        case '7':
            Lib_LcdCls();
            Lib_LcdDrawLine(127, 0, 47, 63, 1);
            Lib_LcdDrawLine(47, 0, 47, 63, 1);
            Lib_LcdDrawLine(127, 0, 127, 63, 1);
            Lib_LcdDrawLine(127, 63, 47, 63, 1);
//            Lib_LcdGotoxy(3, 35);
//            s_LcdDrawLogo(strTemp);
//            s_LcdFlush();
            //Lib_LcdDrawBox(21, 33, 127, 63);
            break;
        case '8':
            Lib_LcdCls();
            Lib_LcdDrawLine(126, 0, 127, 63, 1);
            Lib_LcdDrawLine(0, 62, 127, 63, 1);
            Lib_LcdDrawLine(0, 32, 127, 43, 1);
            Lib_LcdDrawLine(26, 0, 47, 63, 1);
//            Lib_LcdDrawLine(4, 0, 2, 96, 1);
//            Lib_LcdDrawLine(120, 61, 0, 63, 1);
//            Lib_LcdGotoxy(115, 50);
//            s_LcdDrawLogo(strTemp);
//            s_LcdFlush();
            //Lib_LcdDrawBox(24, 20, 128, 127);
            break;
        case '9':
            Lib_LcdCls();
            j = 0;
            while (1)
            {
                if (0 == j)
                {
                    j = 1;
                }
                else
                {
                    j = 0;
                }
                for (i=0; i<LCD_WIDTH; i++)
                {
                    Lib_LcdDrawLine(i, 0, LCD_WIDTH-i-1, LCD_HIGH-1, j);
                    Lib_DelayMs(100);
                }

                for (i=0; i<LCD_HIGH; i++)
                {
                    Lib_LcdDrawLine(0, LCD_HIGH-i-1, LCD_WIDTH-1, i, j);
                    Lib_DelayMs(100);
                }
                if (0 == j)
                {
                    for (i=0; i<LCD_WIDTH; i++)
                    {
                        Lib_LcdDrawLine(i, 0, i, LCD_HIGH-1, 1);
                        Lib_DelayMs(100);
                    }
                    for (i=0; i<LCD_WIDTH; i++)
                    {
                        Lib_LcdDrawLine(i, 0, i, LCD_HIGH-1, 0);
                        Lib_DelayMs(100);
                    }
                }
                else
                {
                    for (i=0; i<LCD_WIDTH; i++)
                    {
                        Lib_LcdDrawLine(i, 0, i, LCD_HIGH-1, 0);
                        Lib_DelayMs(100);
                    }
                    for (i=0; i<LCD_WIDTH; i++)
                    {
                        Lib_LcdDrawLine(i, 0, i, LCD_HIGH-1, 1);
                        Lib_DelayMs(100);
                    }
                }
            }
//            Lib_LcdGotoxy(105, 40);
//            s_LcdDrawLogo(strTemp);
//            s_LcdFlush();
     
            break;
        case 'a':
            Lib_LcdCls();
      
            break;
        case 'b':
            Lib_LcdDrawBox(0, 0, 131, 47);
			Lib_LcdDrawLine(0,0,131,47, 1);
			Lib_LcdDrawLine(131,0,0,47,1);
            break;
        case 'c':
//            InitFontLib();
//            s_UartPrint(COM_DEBUG,"tttttttt");
//            Lib_LcdGotoxy(0, 0);
//            s_LcdWrStrHeight16("中国", 4);
//            s_LcdFlush();
      
            break;
        case 'd':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight16("abcd", 4);
            s_LcdFlush();
            break;
        case 'e':
            InitFontLib();
            break;
        case 'f':
//            s_UartPrint(COM_DEBUG,",s1=%d,s2=%d,s3=%d,", sizeof(VFONT_LIB_STRUCT),
//                sizeof(ChineseFontEncode), sizeof(EnglishFontEncode));
//            s_UartPrint(COM_DEBUG, ",M#=%s, t1=%d, t2=%d,",
//                g_sVFontLibHead.abyVFontFlag, g_sVFontLibHead.byLangeType,
//                g_sVFontLibHead.byLangeCodeType);
            break;
        case 'g':
            s_UartPrint(COM_DEBUG,"tttttttt");
            Lib_LcdGotoxy(32, 8/2);
            s_LcdWrStrHeight16("黄山", 4);
            s_LcdFlush();
            break;
        case 'h':
            s_UartPrint(COM_DEBUG,"tttttttt");
            Lib_LcdGotoxy(64, 12/2);
            s_LcdWrStrHeight16("峨嵋山", 6);
            s_LcdFlush();
            break;
        case 'i':
            s_UartPrint(COM_DEBUG,"tttttttt");
            Lib_LcdGotoxy(112, 12/2);
            s_LcdWrStrHeight16("五指山", 6);
            s_LcdFlush();
            break;
        case 'j':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight8("五abcd", 6);
            s_LcdFlush();
            break;
        case 'k':
            Lib_LcdGotoxy(0, 16);
            s_LcdWrStrHeight8("五ab刘", 6);
            s_LcdFlush();
            break;
        case 'l':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight8("abcdef", 6);
            s_LcdFlush();
            break;
        case 'm':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight12("五jfabcg", 8);
            s_LcdFlush();
            break;
        case 'n':
            Lib_LcdSetAttr(1);
            Lib_LcdGotoxy(64, 12/2);
            s_LcdWrStrHeight12("峨嵋山", 6);
            s_LcdFlush();
            break;
        case 'o':
            Lib_LcdGotoxy(0, 0);
            Lib_LcdSetFont(8, 16, 0);
            Lib_Lcdprintf("abcdef北京上海动车组");
            break;
        case 'p':
            Lib_LcdGotoxy(0, 0);
            Lib_LcdSetFont(12, 12, 0);
            Lib_Lcdprintf("a北京上海动车组北京上b海动车组北京上海动车c组北京上海动车组北京d上海动车组北京上海动e车组北京上海动车组北京上海动车组");
            break;
        case 'q':
            Lib_LcdClrLine(8/2, 15/2);
            break;
        case 'r':
            Lib_LcdClrLine(0, 63);
            Lib_LcdPrintxy(0, 0, 0x81, "ABCD");
            Lib_LcdPrintxy(0, 16/2, 0x81, "EFGH");
            break;
        case 's':
            Lib_LcdRestore(0);  // 保存当前屏
            break;
        case 't':
            Lib_LcdRestore(1);  // 恢复前面保存的屏
            break;
        case 'E':
            return 0;
        }
    } 
    
#endif
    
return 0;
}

/****************************************************************************
  函数名     :  void s_LcdDrawBox(BYTE x1, BYTE y1, BYTE x2,BYTE y2)
  描述       :  在指定的坐标画矩形框，中间空
  输入参数   :  1、BYTE x1：取值范围是0~131
                2、BYTE y1：取值范围是0~47
                3、BYTE x2：取值范围是0~131
                4、BYTE y2：取值范围是0~47
  备   注    :  输入的值x1<=x2，y1<=y2
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、
****************************************************************************/
void s_LcdDrawBox(BYTE x1, BYTE y1, BYTE x2,BYTE y2)
{
    BYTE bit, Line;
    BYTE i, end;

    end = x2;
    if(LCDValue < LCD_AD_FLAG)
    {
        if (x2 > 131)
        {
            end = 131;
        }
    }
    else  if(LCDValue >= LCD_AD_FLAG)
    {
        if (x2 > 121)
        {
            end = 121;
        }
    }
    for (i=x1; i<=end; i++)
    {
        Line = y1/8;
        bit = y1%8;
        gs_abyDisplayData[Line][i] |= (1<<bit);
        if(LCDValue < LCD_AD_FLAG)
        {
            if (y2 <= 47)
            {
                Line = y2/8;
                bit = y2%8;
                gs_abyDisplayData[Line][i] |= (1<<bit);
            }
        }
        else if(LCDValue >= LCD_AD_FLAG)
        {
            if (y2 <= 31)
            {
                Line = y2/8;
                bit = y2%8;
                gs_abyDisplayData[Line][i] |= (1<<bit);
            }
        }
    }
    end = y2;
    if(LCDValue < LCD_AD_FLAG)
    {
        if (y2 > 47)
        {
            end = 47;
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        if (y2 > 31)
        {
            end = 31;
        }
    }
    for (i=y1; i<=end; i++)
    {
        Line = i/8;
        bit = i%8;
        gs_abyDisplayData[Line][x1] |= (1<<bit);
        if(LCDValue < LCD_AD_FLAG)
        {
            if (x2 <= 131)
            {
                gs_abyDisplayData[Line][x2] |= (1<<bit);
            }
        }
        else if(LCDValue >= LCD_AD_FLAG)
        {
            if (x2 <= 121)
            {
                gs_abyDisplayData[Line][x2] |= (1<<bit);
            }
        }
    }
}

/****************************************************************************
  函数名     :  void Lib_LcdDrawBox(BYTE x1, BYTE y1, BYTE x2, BYTE y2)
  描述       :  在指定的坐标画矩形框，中间空。如果输入参数错误将无任何操作
  输入参数   :  1、BYTE x1：取值范围是0~131
                2、BYTE y1：取值范围是0~47
                3、BYTE x2：取值范围是0~131
                4、BYTE y2：取值范围是0~47
  备   注    :  输入的值x1<=x2，y1<=y2
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-19  V1.0         修改
****************************************************************************/
void Lib_LcdDrawBox(BYTE x1, BYTE y1, BYTE x2, BYTE y2)
{
    if(LCDValue < LCD_AD_FLAG)
    {
        if ((y2<y1) || (x2<x1) || (y1>47) || (x1>131))
        {
            return;
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        if ((y2<y1) || (x2<x1) || (y1>31) || (x1>121))
        {
            return;
        }
    }
    if (gs_byLcdBusyFlag)
    {
        return;
    }
    gs_byLcdBusyFlag = 1;
    s_LcdDrawBox(x1, y1, x2, y2);
    s_LcdFlush();
    gs_byLcdBusyFlag = 0;
}

/****************************************************************************
  函数名     :  void Lib_LcdDrawLine(BYTE x1, BYTE y1, BYTE x2, BYTE y2)
  描述       :  画直线函数，点不在取值范围内则不作任何操作，直接返回
  输入参数   :  1、BYTE x1：点1的x坐标，取值范围是0~131
                2、BYTE y1：点1的y坐标，取值范围是0~47
                3、BYTE x2：点2的x坐标，取值范围是0~131
                4、BYTE y2：点2的y坐标，取值范围是0~47
  输出参数   :  无
  返回值     :  无
  修改历史   :
      修改人     修改时间    修改版本号   修改原因
  1、 黄俊斌     2007-09-20  V1.1         修改使该函数能画任意直线
****************************************************************************/
void Lib_LcdDrawLine(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE byColor)
{
    int i, k, j, l10;
    BYTE byTemp, t1, t2;

    i = 0;
    if(LCDValue < LCD_AD_FLAG)
    {
        if ((x1>=LCD_WIDTH) || (x2>=LCD_WIDTH) || (y1>=LCD_HIGH) || (y2>=LCD_HIGH))
        {
            return ;
        }
    
        if (x1 == x2)
        {
            if (y1 > y2)
            {
                byTemp = y1;
                y1 = y2;
                y2 = byTemp;
            }
            for (i=y1; i<=y2; i++)
            {
                s_LcdDrawPlot(x1, i, byColor);
            }
        }
        else if (y1 == y2)
        {
            if (x1 > x2)
            {
                byTemp = x1;
                x1 = x2;
                x2 = byTemp;
            }
            for (i=x1; i<=x2; i++)
            {
                s_LcdDrawPlot(i, y1, byColor);
            }
        }
        else
        {
            k = (y2-y1)/(x2-x1);
            if ((k<1) && (k>-1))
            {
                t1 = x1;
                t2 = x2;
                if (x1 > x2)
                {
                    t1 = x2;
                    t2 = x1;
                }
                for (i=t1; i<=t2; i++)
                {
                    l10 = ((y2-y1)*(i-x1)*LCD_WIDTH) / (x2-x1);
                    if (l10 > 0)
                    {
                        if ((l10%LCD_WIDTH) >= LCD_HIGH)
                        {
                            j = l10/LCD_WIDTH + 1 + y1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH + y1;
                        }
                    }
                    else
                    {
                        if ((-l10%LCD_WIDTH) >= LCD_HIGH)
                        {
                            j = l10/LCD_WIDTH - 1 + y1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH + y1;
                        }
                    }
                    s_LcdDrawPlot(i, j, byColor);
                }
            }
            else
            {
                k = (x2-x1)/(y2-y1);
                t1 = y1;
                t2 = y2;
                if (y1 > y2)
                {
                    t1 = y2;
                    t2 = y1;
                }
                for (i=t1; i<=t2; i++)
                {
                    l10 = ((x2-x1)*(i-y1)*LCD_WIDTH)/(y2-y1);
                    if (l10 > 0)
                    {
                        if ((l10%LCD_WIDTH) > LCD_HIGH)
                        {
                            j = l10/LCD_WIDTH + 1 + x1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH + x1;
                        }
                    }
                    else
                    {
                        if ((-l10%LCD_WIDTH) >= LCD_HIGH)
                        {
                            j = l10/LCD_WIDTH + x1 -1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH + x1;
                        }
                    }
                    s_LcdDrawPlot(j, i, byColor);
                }
            }
        }
    }
    else if(LCDValue >= LCD_AD_FLAG)
    {
        if ((x1>=LCD_WIDTH_OLD) || (x2>=LCD_WIDTH_OLD) || (y1>=LCD_HIGH_OLD) || (y2>=LCD_HIGH_OLD))
        {
            return ;
        }
    
        if (x1 == x2)
        {
            if (y1 > y2)
            {
                byTemp = y1;
                y1 = y2;
                y2 = byTemp;
            }
            for (i=y1; i<=y2; i++)
            {
                s_LcdDrawPlot(x1, i, byColor);
            }
        }
        else if (y1 == y2)
        {
            if (x1 > x2)
            {
                byTemp = x1;
                x1 = x2;
                x2 = byTemp;
            }
            for (i=x1; i<=x2; i++)
            {
                s_LcdDrawPlot(i, y1, byColor);
            }
        }
        else
        {
            k = (y2-y1)/(x2-x1);
            if ((k<1) && (k>-1))
            {
                t1 = x1;
                t2 = x2;
                if (x1 > x2)
                {
                    t1 = x2;
                    t2 = x1;
                }
                for (i=t1; i<=t2; i++)
                {
                    l10 = ((y2-y1)*(i-x1)*LCD_WIDTH_OLD) / (x2-x1);
                    if (l10 > 0)
                    {
                        if ((l10%LCD_WIDTH_OLD) >= LCD_HIGH_OLD)
                        {
                            j = l10/LCD_WIDTH_OLD + 1 + y1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH_OLD + y1;
                        }
                    }
                    else
                    {
                        if ((-l10%LCD_WIDTH_OLD) >= LCD_HIGH_OLD)
                        {
                            j = l10/LCD_WIDTH_OLD - 1 + y1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH_OLD + y1;
                        }
                    }
                    s_LcdDrawPlot(i, j, byColor);
                }
            }
            else
            {
                k = (x2-x1)/(y2-y1);
                t1 = y1;
                t2 = y2;
                if (y1 > y2)
                {
                    t1 = y2;
                    t2 = y1;
                }
                for (i=t1; i<=t2; i++)
                {
                    l10 = ((x2-x1)*(i-y1)*LCD_WIDTH_OLD)/(y2-y1);
                    if (l10 > 0)
                    {
                        if ((l10%LCD_WIDTH_OLD) > LCD_HIGH_OLD)
                        {
                            j = l10/LCD_WIDTH_OLD + 1 + x1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH_OLD + x1;
                        }
                    }
                    else
                    {
                        if ((-l10%LCD_WIDTH_OLD) >= LCD_HIGH_OLD)
                        {
                            j = l10/LCD_WIDTH_OLD + x1 -1;
                        }
                        else
                        {
                            j = l10/LCD_WIDTH_OLD + x1;
                        }
                    }
                    s_LcdDrawPlot(j, i, byColor);
                }
            }
        }
    }
    s_LcdFlush();
}

