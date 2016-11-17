
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

BYTE g_byStartX = 0;    // ��ǰ��ʼλ������x��ȡֵ0~127
BYTE g_byStartY = 0;    // ��ǰ��ʼλ������y��ȡֵ0~63
BYTE g_byFontType;      // ��ʾ���塣
                        // 0��     ASCII��ʾģʽ������16x16��   ASCII�ַ�6x8
                        // 1��GB2312��׼��ʾģʽ������16x16��   ASCII�ַ�8x16
                        // 2��GB2312������ʾģʽ������12x12��   ASCII�ַ�6x12

BYTE g_byFontAttr;      // Һ����ʾ���ԣ�0��ʾ������ʾ����0��ʾ�ڰ׵ߵ���ʾ

static BYTE gs_byLcdBusyFlag = 0;           // Һ����æ��־��0��ʾ���У���0��ʾæ��æ��ʱ���ܽ�������
BYTE gs_byBaseGrayVal = DEFAULT_GRAY_BASE; // �ҶȻ�ֵ
//static BYTE gs_byRestoreX = 0;              // ������ʼλ������x��ȡֵ0~127
//static BYTE gs_byRestoreY = 0;              // ������ʼλ������y��ȡֵ0~63
//static BYTE gs_byRestoreEnable = 0;         // ����ʹ�ܣ�1�ɱ��ݣ�0���ɱ��ݣ���û���������ݵ�ʱ�򣬸ñ���ȡֵΪ0
static BYTE gs_abyDisplayData[6][132];      // ��Ļ��ʾ���ݻ���
static BYTE gs_abyCurrentData[6][132];      // ��ǰ��Ļ����ʾ������

unsigned long LCDValue; // <3000 ���� >=3000 ���� ����ֵ2043���� ����ֵ3520����
                        // <= 300 ������USB   >= 4000 ������USB


// 15��ͼ�������ֵ
BYTE g_byIconVal[15] = {2, 3, 4, 20, 22, 24, 26, 28, 30, 42, 61, 77, 93, 108, 122};

BYTE g_byIconBufUpdated;
BYTE g_abyIconBufMode[8];

// ������ʾģʽ��0 �ر��⣻1 ���Ᵽ����1���ӣ���1���Ӻ��Զ��رգ���
// Ĭ��ֵ��2 ���ⳣ�����ñ������ڴſ������ܿ�ģ��ʹ��
BYTE k_LcdBackLightMode;
int  k_LcdBackLightTime;

int g_iLcdTimeCount = 6000;   //0

extern int g_iChineseFontFlag;

int g_iHaveFontFlag = 0;  // �ֿ���ڱ�־��0��ʾ�����ڣ�1��ʾ����

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
  ������     :  int s_ADCInit(void)
  ����       :  Ӳ��ADC��ʼ������
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��

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
  ������     :  int s_LcdInit(void)
  ����       :  Һ����ʼ������
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��

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

    
    Lib_LcdCls(); // LCD����
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
    LcdWriteCmd(0xE2);    WaitNuS(2000);      //��λ
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
    
    s_LCD_WrCMD(0xb0, LcdWrCmd);  //0ҳ
    s_LCD_WrCMD(0x10, LcdWrCmd);  //0��highλ
    s_LCD_WrCMD(0, LcdWrCmd);     //0��lowλ
    
    
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
    s_LCD_WrCMD((0xb0|( y & 0x07)), LcdWrCmd);  //yҳ����//
    s_LCD_WrCMD((0x10|(x>>4)), LcdWrCmd);  //x��highλ//
    s_LCD_WrCMD((x&0x0f), LcdWrCmd);              //x��lowλ//
    x++;
}

/****************************************************************************
  ������     :  void s_LcdCls(void)
  ����       :  �������建��
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-19  V1.0         ����
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
              
            for (i=0; i<=132; i++) //�к�//
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
              
            for (i=0; i<=122; i++) //�к�//
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
  ������     :  void Lib_LcdCls(void)
  ����       :  ������ֻ���������ʾ���򣬲����ͼ��
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         ����
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
  ������     :
  ����       :  �ָ�ͼ����ʾ
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1��
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
  ������     :  void s_ControlIcon(int iNumber, BYTE byOnOff)
  ����       :  ����ͼ����ʾ���߹ر�
  �������   :  1��int iNumber��ͼ���ţ�ȡֵ��Χ��0��14������ֵ�Ƿ�
                2��BYTE byOnOff�����أ�0��ʾ�رգ���0��ʾ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-17  V1.0         ����
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
    s_LCD_WrCMD(0xb8, LcdWrCmd);    // ��8ҳ����
    s_LCD_WrCMD((0x10|(byTemp>>4)&0x07), LcdWrCmd);  // ��highλ
    s_LCD_WrCMD(byTemp&0x0f, LcdWrCmd);   // ��lowλ
    s_LCD_WrCMD(byData, LcdWrData);    // ���ؿ���
}

/****************************************************************************
  ������     :  void s_CloseAllIcon()
  ����       :  �ر�����ͼ��
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         ����
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
  ������     :  void s_LcdSetIcon(BYTE byIconNo, BYTE byMode)
  ����       :  ����ͼ�꺯������������������κβ���
  �������   :  1��BYTE byIconNo��ͼ���ţ�ȡֵΪ1��8������ֵ�Ƿ�
                2��BYTE byMode��ͼ����ʾģʽ����ͼ���ž���
   byIconNo   byMode  ����
   1----------0-------�绰ͼ��رգ�2��3��4���ر�
   1----------1-------�绰ͼ��ժ����2��4�򿪣�3�ر�
   1----------2-------�绰ͼ��һ���3��4�򿪣�2�ر�
   2----------0-------�����ź�ͼ��رգ�   20��22��24��26��28��30���ر�
   2----------1-------�����ź�ͼ��ǿ��Ϊ0��20�򿪣�22��24��26��28��30�ر�
   2----------2-------�����ź�ͼ��ǿ��Ϊ1��20��22�򿪣�24��26��28��30�ر�
   2----------3-------�����ź�ͼ��ǿ��Ϊ2��20��22��24�򿪣�26��28��30�ر�
   2----------4-------�����ź�ͼ��ǿ��Ϊ3��20��22��24��26�򿪣�28��30�ر�
   2----------5-------�����ź�ͼ��ǿ��Ϊ4��20��22��24��26��28�򿪣�30�ر�
   2----------6-------�����ź�ͼ��ǿ��Ϊ5��20��22��24��26��28��30����
   3----------0-------��ӡ��ͼ��رգ�42�ر�
   3----------1-------��ӡ��ͼ��򿪣�42��
   4----------0-------IC��ͼ��رգ�61�ر�
   4----------1-------IC��ͼ��򿪣�61��
   5----------0-------��ͼ��رգ�77�ر�
   5----------1-------��ͼ��򿪣�77��
   6----------0-------���ͼ��رգ�93�ر�
   6----------1-------���ͼ��򿪣�93��
   7----------0-------����ͼ��رգ�108�ر�
   7----------1-------����ͼ��򿪣�108��
   8----------0-------����ͼ��رգ�122�ر�
   8----------1-------����ͼ��򿪣�122��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1��
****************************************************************************/
void s_LcdSetIcon(BYTE byIconNo, BYTE byMode)
{
    BYTE i;
    switch (byIconNo)
    {
    case 1: //telephone
        switch (byMode)
        {
        case 0:   // �ر�
            s_ControlIcon(0, 0);
            s_ControlIcon(1, 0);
            s_ControlIcon(2, 0);
            break;
        case 1:   //ժ��
            s_ControlIcon(0, 1);
            s_ControlIcon(1, 0);
            s_ControlIcon(2, 1);
            break;
        case 2:   // �һ�
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
  ������     :  void Lib_LcdSetIcon(BYTE byIconNo, BYTE byMode)
  ����       :  �޸�Һ��ͼ����ʾ
  �������   :  1��BYTE byIconNo��ͼ���ţ�ȡֵΪ1��8������ֵ�Ƿ�
                2��BYTE byMode��ͼ����ʾģʽ����ͼ���ž���
   byIconNo   byMode  ����
   1----------0-------�绰ͼ��رգ�2��3��4���ر�
   1----------1-------�绰ͼ��ժ����2��4�򿪣�3�ر�
   1----------2-------�绰ͼ��һ���3��4�򿪣�2�ر�
   2----------0-------�����ź�ͼ��رգ�   20��22��24��26��28��30���ر�
   2----------1-------�����ź�ͼ��ǿ��Ϊ0��20�򿪣�22��24��26��28��30�ر�
   2----------2-------�����ź�ͼ��ǿ��Ϊ1��20��22�򿪣�24��26��28��30�ر�
   2----------3-------�����ź�ͼ��ǿ��Ϊ2��20��22��24�򿪣�26��28��30�ر�
   2----------4-------�����ź�ͼ��ǿ��Ϊ3��20��22��24��26�򿪣�28��30�ر�
   2----------5-------�����ź�ͼ��ǿ��Ϊ4��20��22��24��26��28�򿪣�30�ر�
   2----------6-------�����ź�ͼ��ǿ��Ϊ5��20��22��24��26��28��30����
   3----------0-------��ӡ��ͼ��رգ�42�ر�
   3----------1-------��ӡ��ͼ��򿪣�42��
   4----------0-------IC��ͼ��رգ�61�ر�
   4----------1-------IC��ͼ��򿪣�61��
   5----------0-------��ͼ��رգ�77�ر�
   5----------1-------��ͼ��򿪣�77��
   6----------0-------���(����������)ͼ��رգ�93�ر�
   6----------1-------���(����������)ͼ��򿪣�93��
   7----------0-------����ͼ��رգ�108�ر�
   7----------1-------����ͼ��򿪣�108��
   8----------0-------����ͼ��رգ�122�ر�
   8----------1-------����ͼ��򿪣�122��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-17  V1.0         ����
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
  ������     :  void s_LcdDrawPlot(BYTE XO, BYTE YO, BYTE Color)
  ����       :  �޸����ݻ����ڵ�ĳ������λ
  �������   :  1��BYTE XO���кţ�ȡֵ0��122
                2��BYTE YO���кţ�ȡֵ0��31
                3��BYTE Color����ɫ��0��ʾ��ɫ����0��ʾ��ɫ
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-17   V1.0        ����
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
  ������     :  void Lib_LcdDrawPlot(BYTE XO,BYTE YO,BYTE Color)
  ����       :  ���㺯��
  �������   :  1��BYTE XO���кţ�ȡֵ0��127
                2��BYTE YO���кţ�ȡֵ0��63
                3��BYTE Color�������ɫ��0��ʾ��ɫ����0��ʾ��ɫ
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         �޸�
  1�� �ƿ���     2007-10-24  V1.1         ������������Ϸ��Լ��
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
  ������     :  void Lib_LcdSetGray(BYTE level)
  ����       :  ������ʾ��Ļ�ĶԱȶ�.
  �������   :  1��BYTE level���Աȶȼ���[0~63��0���63����]��Ĭ��ֵΪ44��
                ����ֵ�޲�����
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  
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
  ������     :  void Lib_LcdGotoxy(BYTE x, BYTE y)
  ����       :  ������Ƶ�ָ��λ�ã���������Ƿ����޲���
  �������   :  1��BYTE x���бꡣ ��Ļ��ˮƽ���꣨0<=x<=127��
                2��BYTE y���бꡣ ��Ļ�Ĵ�ֱ���꣨0<=y<=63��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         �޸�
  1�� �ƿ���     2007-10-24  V1.1         ������������Ϸ��Լ��
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
  ������     :  int Lib_LcdSetFont(BYTE AsciiFontHeight,
                           BYTE ExtendFontHeight, BYTE Zoom)
  ����       :  ������ʾ����
  �������   :  1��BYTE AsciiFontHeight��ASCII��ʾ����߶ȣ���ȡֵ8��12��16��
                2��BYTE ExtendFontHeight����չ����߶ȣ���ȡֵ12��16
                3��BYTE Zoom���Ŵ�ģʽ��Ŀǰ��֧�ַŴ󣬸ò�����ʹ��
  ��    ע   ��(AsciiFontHeight,ExtendFontHeight)��ȡ(8,16)��(12,12)��(16,16)
               ����ֵ��������ϵ�ֵ������ȷ
  �������   :  ��
  ����ֵ     :  0���óɹ���-1500�����������ȷ
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         ����
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
  ������     :  int Lib_LcdGetFont(BYTE *AsciiFontHeight, BYTE *ExtendFontHeight, BYTE *Zoom)
  ����       :  ��ȡ��ǰ��ʾ��ʽ
  �������   :  ��
  �������   :  1��BYTE *AsciiFontHeight��ASCII���������С
                2��BYTE *ExtendFontHeight����չ���������С������ָ���Ǻ���
                3��BYTE *Zoom���Ŵ������������Զ���0��
  ��  ע     :  (*AsciiFontHeight,*ExtendFontHeight)�Ŀ������ֵΪ(8,16)��(12,12)
                ��(16,16)
  ����ֵ     :  0��ʾ�ɹ���-1500��ʾʧ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         ����
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
  ������     :  BYTE Lib_LcdSetAttr(BYTE attr)
  ����       :  ������ʾ����
  �������   :  1��BYTE attr����ʾ���ԡ�
                      0������ʾ��
                      ��0�ڰ׷���ʾ��
  �������   :  ��
  ����ֵ     :  ԭ������ʾ����
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         ����
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

// дҳ���ַ�ӳ���*********************************************************************
void set_page_address(u8 x)
{
    u8  page_temp;
    //AT skx
    //page_temp=x|0xb0;
    page_temp=(x&0x0f)|0xb0;
    s_LCD_WrCMD(page_temp,LcdWrCmd);
}

// д�е�ַ�ӳ���*********************************************************************
void set_column_address(u8 x)
{
    u8  column_temp;
    column_temp=x%16;		//ȡ��λ�е�ַ
    s_LCD_WrCMD(column_temp,LcdWrCmd);

    column_temp=x/16;		//ȡ��λ�е�ַ
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
  ������     :  void s_LcdFlush()
  ����       :  ���������ڵ�����д��Һ����
  �������   :  ��
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-17   V1.0        ����
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
            for (i=0; i<132; i++)                           //�к�//
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
            for (i=0; i<122; i++)                           //�к�//
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
  ������     :  void Lib_LcdClrLine(BYTE startline, BYTE endline)
  ����       :  �����Ļ��ָ���У�ִ�гɹ������ͣ�ڣ�0��endline���������
                �����������ȷ�������κβ���
  �������   :  1��BYTE startline����ʼ�кţ�ȡֵ0��31
                2��BYTE endline�������кţ�ȡֵ0��31
  ��   ע    :  ����Ĳ�����������startline<=endline
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-18  V1.0         ����ʾ��ʽ�ı���޸�
  1�� �ƿ���     2007-10-24  V1.1         ��ǿ��������Ϸ��Լ��
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
  ������     :  int GetCharDotMatrix(BYTE *str, BYTE byCharSet, BYTE byFontHeight,
                     BYTE *pbyCharDotMatrix)
  ����       :  ��ȡ�ֿ���ַ�������Ϣ
  �������   :  1��char *str���ַ�����byCharSet=ASCII_FONTʱ��str����Ϊ1
                                    ��byCharSet=GB2312_FONTʱ��str����Ϊ2
                2��BYTE byCharSet���ַ����룬ASCII_FONT/GB2312_FONT
                3��BYTE byFontHeight���ַ��ĸ߶ȣ�ȡֵ8��12��16��24��
                   ����ASCII ��ȡֵ 8��16��24��
                       GB2312��ȡֵ12��16��24��
  �������   :  1��BYTE *pbyCharDotMatrix���ַ��ĵ�����Ϣ
  ����ֵ     :  0���ɹ���1�������������2��û�иõ�����Ϣ��3��û���ֿ��ļ�
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-14   V1.0        ����
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
        // ��������GB2312�ֿ�
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
  ������     :  void s_InitDisplayBuffer(BYTE *pbyBuff, BYTE byCharSet, BYTE byHeight)
  ����       :  ��ȡû���ֿ�ʱ�����ֵ���
  �������   :  1��BYTE byCharSet���ַ�����0��ʾASCII������ֵ��ʾ��չ�ַ�
                2��BYTE byHeight���ַ��߶ȣ���ȡֵΪ8��12��16
  �������   :  1��BYTE *pbyBuff����������
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-07   V1.0        ����
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
  ������     :  void s_LcdWrStrHeight8(BYTE *str, BYTE strLen)
  ����       :  ����Ļ����ʾ�߶�Ϊ8���ص��ַ������ڲ�ʹ�ã�����к��֣�����
                ���߶�Ϊ16������ʾ
  �������   :  1��BYTE *str��Ҫ��ʾ���ַ���
                2��BYTE strLen���ַ����ĳ���
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-07   V1.0        ����
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
            byFontHeight = 16;  // ����к��֣����ְ�16X16��ʾ��ASCII��Ȼ��6X8��ʾ
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
  ������     :  void s_LcdWrStrHeight12(BYTE *str, BYTE strLen)
  ����       :  ����Ļ����ʾ�߶�Ϊ12���ص��ַ������ڲ�ʹ��
  �������   :  1��BYTE *str��Ҫ��ʾ���ַ���
                2��BYTE strLen���ַ����ĳ���
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-07   V1.0        ����
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
  ������     :  void s_LcdWrStr(BYTE *str, BYTE strLen)
  ����       :  ����Ļ����ʾ�ַ������ڲ�ʹ��
  �������   :  1��BYTE *str��Ҫ��ʾ���ַ���
                2��BYTE strLen���ַ����ĳ���
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-07   V1.0        ����
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
  ������     :  void s_LcdDrawLogo(BYTE *pDataBuffer)
  ����       :  ����־ͼ����������־ͼ֮ǰ�������û�ͼ��ʼλ�ã���־ͼ�Ĺ���
                �в�����ı仭ͼ����ʼλ��
  �������   :  1��BYTE *pDataBuffer����������ݣ�ǰ�����ֽ��Ǳ�־ͼ�Ŀ�͸ߣ�
                   �����Ǳ�־ͼ�ĵ�������
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-19  V1.0         �޸�
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
                return ;  // ����������ֲ�����
            }
            lineOffset = j/8;
            bitOffset = j%8;
            
            //����м��жϲ� >12
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
                    break;  // ����������ֲ�����
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
                return ;  // ����������ֲ�����
            }
            lineOffset = j/8;
            bitOffset = j%8;
            
            //����м��жϲ� >12
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
                    break;  // ����������ֲ�����
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
  ������     :  int Lib_Lcdprintf(char *fmt,...)
  ����       :  ��Һ�����ϴ�ӡ�ַ����������ʽ��ο�C���Ե�printf()����������
                �ú�����Һ�����ϵĹ��λ��ͣ���ڸô�ӡ������ӡ�����һ����
                ���ĺ��
  �������   :  1��char *fmt,...��Ҫ��ʾ�ĸ�ʽ�ַ���
  �������   :  ��
  ����ֵ     :  ��������Ļ����ʾ���ַ��ĸ���
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-19  V1.0         �޸�
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
  ������     :  void Lib_LcdPrintxy(BYTE col, BYTE row, BYTE mode, char *str,...)
  ����       :  ��ָ��λ�ô�ӡָ����ʽ���ַ�����������ֵ�Ƿ���ʱ�����κβ���
  �������   :  1��BYTE col���кţ�ȡֵ��Χ��0~131
                2��BYTE row���кţ�ȡֵ��Χ��0~47
                3��BYTE mode����ʾģʽ����ȡֵ0x00��0x01��0x02��0x80��0x81��
                   0x82������ֵ�Ƿ�
                4��char *str,...��Ҫ����Ļ����ʾ�ĸ�ʽ�ַ���
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-19  V1.0         �޸�
  1�� �ƿ���     2007-10-24  V1.1         ������������Ϸ��Լ��
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
  ������     :  void Lib_LcdSetBackLight(BYTE mode)
  ����       :  ������Ļ���⣨���а�����ˢ�����忨ʱ�Զ�����
  �������   :  1��BYTE mode��������ʾģʽ��
                     0 �ر���
                     1 ���Ᵽ����1���ӣ���1���Ӻ��Զ��رգ���Ĭ��ֵ
                     2 ���ⳣ��
                     ����ֵ�޲���
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1��
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
  ������     :  void Lib_LcdGetSize(uchar * x, uchar *y)
  ����       :  ��ȡ��ʾ�����ԣ����غ������������
  �������   :  ��
  �������   :  1��BYTE *x����ǰ��ʾ���Ŀ��
                2��BYTE *y����ǰ��ʾ���ĸ߶�
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1��
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
    Lib_Lcdprintf("�������");
    while (1)
    {
        Lib_LcdGotoxy(0, 32);
        if (0 != iFlag)
        {
            Lib_Lcdprintf("������=%d", i);
            Lib_LcdSetBackLight(2);
            iFlag = 0;
        }
        else
        {
            Lib_Lcdprintf("�ر���=%d", i);
            Lib_LcdSetBackLight(0);
            iFlag = 1;
        }
        Lib_KbGetCh();
        i++;
    }
	*/

	uchar EnMenu[5][16] = {
									 "��Uart Test    ", 
									 //"Uart Test    ", 
									 "USB�� Test     ", 
									 //"USB Test     ", 
									 "Modem Test   ",
				 					 "GPRS Test     ",
				 					 "LAN Test      "
								 };

    //BYTE ch;
    //int i = 0, j;
	//static char count = 0;
   // BYTE strTemp[200] = {0xc4,0xbc,0x07,0xc4,0x3c,0x21,0x21,0xf9,0x25,0x23,0x20,0x00,0x04,0x02,0x01,0x01,
//0x02,0x00,0x04,0x07,0x00,0x00,0x00,0x00};/*"��",0*/
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
            Lib_Lcdprintf("��ʾ����abc");
            Lib_LcdGotoxy(3, 27);
            Lib_LcdSetFont(16, 16, 0);
            Lib_Lcdprintf("��ʾ����abc");
            Lib_LcdGotoxy(13, 50);
            Lib_LcdSetFont(12, 12, 0);
            Lib_Lcdprintf("��ʾ����abc");
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
            s_LcdPrintxy(0,0,0x81, "dsss�Ϻ�");
            s_LcdPrintxy(22,23,0x82, "dsss�Ϻ�");
            s_LcdPrintxy(42,40,0x80, "dsss�Ϻ�");
            break;
        case '3':
            //Lib_LcdCls();
            //s_LcdPrintxy(00,00,0x01, "dsss�Ϻ�dsss�Ϻ�dsss�Ϻ�dsss�Ϻ�");
            //s_LcdPrintxy(22,23,0x02, "dsss�Ϻ�");
            //s_LcdPrintxy(42,40,0x00, "dsss�Ϻ�");
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
//            s_LcdWrStrHeight16("�й�", 4);
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
            s_LcdWrStrHeight16("��ɽ", 4);
            s_LcdFlush();
            break;
        case 'h':
            s_UartPrint(COM_DEBUG,"tttttttt");
            Lib_LcdGotoxy(64, 12/2);
            s_LcdWrStrHeight16("����ɽ", 6);
            s_LcdFlush();
            break;
        case 'i':
            s_UartPrint(COM_DEBUG,"tttttttt");
            Lib_LcdGotoxy(112, 12/2);
            s_LcdWrStrHeight16("��ָɽ", 6);
            s_LcdFlush();
            break;
        case 'j':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight8("��abcd", 6);
            s_LcdFlush();
            break;
        case 'k':
            Lib_LcdGotoxy(0, 16);
            s_LcdWrStrHeight8("��ab��", 6);
            s_LcdFlush();
            break;
        case 'l':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight8("abcdef", 6);
            s_LcdFlush();
            break;
        case 'm':
            Lib_LcdGotoxy(0, 0);
            s_LcdWrStrHeight12("��jfabcg", 8);
            s_LcdFlush();
            break;
        case 'n':
            Lib_LcdSetAttr(1);
            Lib_LcdGotoxy(64, 12/2);
            s_LcdWrStrHeight12("����ɽ", 6);
            s_LcdFlush();
            break;
        case 'o':
            Lib_LcdGotoxy(0, 0);
            Lib_LcdSetFont(8, 16, 0);
            Lib_Lcdprintf("abcdef�����Ϻ�������");
            break;
        case 'p':
            Lib_LcdGotoxy(0, 0);
            Lib_LcdSetFont(12, 12, 0);
            Lib_Lcdprintf("a�����Ϻ������鱱����b�������鱱���Ϻ�����c�鱱���Ϻ������鱱��d�Ϻ������鱱���Ϻ���e���鱱���Ϻ������鱱���Ϻ�������");
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
            Lib_LcdRestore(0);  // ���浱ǰ��
            break;
        case 't':
            Lib_LcdRestore(1);  // �ָ�ǰ�汣�����
            break;
        case 'E':
            return 0;
        }
    } 
    
#endif
    
return 0;
}

/****************************************************************************
  ������     :  void s_LcdDrawBox(BYTE x1, BYTE y1, BYTE x2,BYTE y2)
  ����       :  ��ָ�������껭���ο��м��
  �������   :  1��BYTE x1��ȡֵ��Χ��0~131
                2��BYTE y1��ȡֵ��Χ��0~47
                3��BYTE x2��ȡֵ��Χ��0~131
                4��BYTE y2��ȡֵ��Χ��0~47
  ��   ע    :  �����ֵx1<=x2��y1<=y2
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1��
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
  ������     :  void Lib_LcdDrawBox(BYTE x1, BYTE y1, BYTE x2, BYTE y2)
  ����       :  ��ָ�������껭���ο��м�ա������������������κβ���
  �������   :  1��BYTE x1��ȡֵ��Χ��0~131
                2��BYTE y1��ȡֵ��Χ��0~47
                3��BYTE x2��ȡֵ��Χ��0~131
                4��BYTE y2��ȡֵ��Χ��0~47
  ��   ע    :  �����ֵx1<=x2��y1<=y2
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-19  V1.0         �޸�
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
  ������     :  void Lib_LcdDrawLine(BYTE x1, BYTE y1, BYTE x2, BYTE y2)
  ����       :  ��ֱ�ߺ������㲻��ȡֵ��Χ�������κβ�����ֱ�ӷ���
  �������   :  1��BYTE x1����1��x���꣬ȡֵ��Χ��0~131
                2��BYTE y1����1��y���꣬ȡֵ��Χ��0~47
                3��BYTE x2����2��x���꣬ȡֵ��Χ��0~131
                4��BYTE y2����2��y���꣬ȡֵ��Χ��0~47
  �������   :  ��
  ����ֵ     :  ��
  �޸���ʷ   :
      �޸���     �޸�ʱ��    �޸İ汾��   �޸�ԭ��
  1�� �ƿ���     2007-09-20  V1.1         �޸�ʹ�ú����ܻ�����ֱ��
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

