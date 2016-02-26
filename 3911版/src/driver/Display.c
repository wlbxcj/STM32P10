#include "Display.h"
#include "Toolkit.h"
#include "ASCII8X8.h"

#define LcdWrCmd     0
#define LcdWrData    2

						//0	1	2	3	4	5	6	7	8	9	A,    b,    C,    d,    E,    F,    =     -     NULL
unsigned char LedSegCode[19] = {0xC0,	0xF9,	0xA4,	0xB0,	0x99,	0x92,	0x82,	0xF8,	0x80,	0x90,	0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E, 0xB7, 0xBF, 0xFF};

unsigned char DisplayType=0;

#define  SetLcdA0_H()      GPIO_SetBits(GPIOC, GPIO_Pin_11)   
#define  SetLcdA0_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_11)   
#define  SetLcdCS_H()      GPIO_SetBits(GPIOC, GPIO_Pin_10)   
#define  SetLcdCS_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_10)   
#define  SetLcdWR_H()      GPIO_SetBits(GPIOC, GPIO_Pin_9)   
#define  SetLcdWR_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_9)   
#define  SetLcdRD_H()      GPIO_SetBits(GPIOC, GPIO_Pin_8)   
#define  SetLcdRD_L()      GPIO_ResetBits(GPIOC, GPIO_Pin_8)   

extern TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
//u8  ASCII8X8[];
//extern TIM_OCInitTypeDef  TIM_OCInitStructure;
//extern vu16 CCR1_Val;
u8 *pLedData;
u8 ActDig = 0x01;


void Ledarray_On(u8 *NewLedData)
{
  pLedData = NewLedData;
  LedBits_Control(FALSE);
  TIM_Cmd(TIM2, ENABLE);
  
  //TIM_Cmd(TIM4, ENABLE);
}

void Ledarray_Off(void)
{
  u16 LedBits;
  
  TIM_Cmd(TIM2, DISABLE); 
  //TIM_Cmd(TIM4, DISABLE);
  
  LedBits = GPIO_ReadOutputData(GPIOC);
  LedBits |= 0x00FF;
  GPIO_Write(GPIOC, LedBits);
  LedBits_Control(FALSE);
}

//LED(L-->R)  bit1, bit2, bit3, bit4, bit5, bit6, bit7, bit8
//VER10       B12   B13   B14   B15   C8    C9    C10   C11
//VER11       C11   C10   C9    C8    B15   B14   B13   B12
void LedBit8_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOB, GPIO_Pin_12);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

void LedBit7_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOB, GPIO_Pin_13);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}

void LedBit6_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
}

void LedBit5_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOB, GPIO_Pin_15);
    else
        GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}

void LedBit4_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOC, GPIO_Pin_8);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}

void LedBit3_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOC, GPIO_Pin_9);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
}

void LedBit2_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOC, GPIO_Pin_10);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_10);
}

void LedBit1_Control(bool Control)
{
    if (Control)
        GPIO_SetBits(GPIOC, GPIO_Pin_11);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_11);
}

void LedBits_Control(bool Control)
{
    if (Control)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11);
        GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    }
    else
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 | GPIO_Pin_11);
        GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);
    }
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

void  Lib_LcdSetBackLight(unsigned char mode)
{
  
  LcdBL_Control(mode);
  
}



//AT skx
void LcdReset(void)
{
    GPIO_SetBits(GPIOB, GPIO_Pin_15);   
    //WaitNuS(5);
    WaitNuS(1);
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);  
    //WaitNuS(100);
    WaitNuS(5);
    GPIO_SetBits(GPIOB, GPIO_Pin_15);       
    //WaitNuS(5);
    WaitNuS(1);
}


void LcdWriteCmd(u8 Command,u8 cmd_mode)
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

#if 0
//AT skx
void LcdWriteCmd(u8 Command)
{
  u16   GpioData;

  SetLcdA0_L();                 //WaitNuS(2);         //Falling
  SetLcdCS_L();                 //WaitNuS(2);  
  SetLcdRD_H();                 //WaitNuS(2);       
  SetLcdWR_L();                 //WaitNuS(2);    
  GpioData = GPIO_ReadOutputData(GPIOC);
  GpioData &= 0xFF00;
  GpioData |= Command;
  WaitNuS(1);//for 80 ns hold L,80 ns hold H  
  GPIO_Write(GPIOC, GpioData);  //WaitNuS(2);  
  SetLcdWR_H();                 //WaitNuS(2);    
  SetLcdCS_H();                 //WaitNuS(2);    
//  SetLcdA0_H();                 WaitNuS(2);       //Raising
}
#endif

//AT skx
u8 LcdReadStatus(void)
{
  u16   GpioData;
  
  //SetLcdA0_H();                 //WaitNuS(2);  
  //SetLcdWR_H();                 //WaitNuS(2);     
  SetLcdA0_L();                 //WaitNuS(2);       //Falling
  SetLcdCS_L();                 //WaitNuS(2); 
  SetLcdWR_H();//AT 
  SetLcdRD_L();                 //WaitNuS(2); 
  WaitNuS(1);//for 140 ns hold L,80 ns hold H
  GpioData = GPIO_ReadOutputData(GPIOC);
  //SetLcdWR_H();                 //WaitNuS(2); 
  SetLcdRD_H();
  SetLcdCS_H();                 //WaitNuS(2); 
  //AT skx
  //SetLcdA0_H();                 //WaitNuS(2);       //Raising

  return((u8)GpioData);
}

//AT skx
void LcdWriteData(u8 DisplayData)
{
  
#if 0  
  u16   GpioData;

  SetLcdA0_H();                 //WaitNuS(2);       //Raising  
  SetLcdCS_L();                 //WaitNuS(2);  
  SetLcdRD_H();                 //WaitNuS(2);   
  SetLcdWR_L();                 //WaitNuS(2);    
  GpioData = GPIO_ReadOutputData(GPIOC);
  GpioData &= 0xFF00;
  GpioData |= DisplayData;
  WaitNuS(1);//for 140 ns hold L,80 ns hold H
  GPIO_Write(GPIOC, GpioData);  //WaitNuS(2); 
  SetLcdWR_H();                 //WaitNuS(2);    
  SetLcdCS_H();                 //WaitNuS(2);    
//  SetLcdA0_H();                 WaitNuS(2);       //Keep high
#endif
    LcdWriteCmd(DisplayData, LcdWrData);

}

//AT skx

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


// 写页面地址子程序*********************************************************************
void set_page_address(u8 x)
{
    u8  page_temp;
    //AT skx
    //page_temp=x|0xb0;
    page_temp=(x&0x0f)|0xb0;
    LcdWriteCmd(page_temp,LcdWrCmd);
}


// 写列地址子程序*********************************************************************
void set_column_address(u8 x)
{
    u8  column_temp;
    column_temp=x%16;		//取低位列地址
    LcdWriteCmd(column_temp,LcdWrCmd);

    column_temp=x/16;		//取高位列地址
    column_temp=column_temp|0x10;
    LcdWriteCmd(column_temp,LcdWrCmd);
}


// *********************************************************************
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
	for(i=0;i<ByteLength;i++)	LcdWriteData(NewLcdData[i]);     
}



void Lcdarray_clear(u8 LineBit)
{
   u8 i;
           
        if(LineBit & 0x01)        
        {
          set_page_address(2);
          set_column_address(0);
          for(i=0;i<122;i++)	LcdWriteData(0x00);       
        }
        
        if(LineBit & 0x02) 
        {
          set_page_address(3);
          set_column_address(0);
          for(i=0;i<122;i++)	LcdWriteData(0x00);      
        }
        
        if(LineBit & 0x04) 
        {
          set_page_address(4);
          set_column_address(0);
          for(i=0;i<122;i++)	LcdWriteData(0x00);      
        }
        
        if(LineBit & 0x08) 
        {
          set_page_address(5);
          set_column_address(0);
          for(i=0;i<122;i++)	LcdWriteData(0x00);           
        }
}

u8 Char2Dot8X8(u8 *pCharSrc,u8 *pDotSrc)
{
  u8 i,j;
  u16 AsciiOffset;
  u16 DotOffset;
  
  for(i=0;i<15;i++)
  {
    if(*(pCharSrc+i)==0) break;           //string end
    AsciiOffset = (*(pCharSrc+i)-0x20)*8;
    if(AsciiOffset > 0x5F*8) AsciiOffset = 0;
    DotOffset  = i*8;
    for(j=0;j<8;j++)  *(pDotSrc + DotOffset +j) = ASCII8X8[AsciiOffset+j];
  }
  
  return i;
}

void Lcdstring_On(u8 LineX,u8 LineY,u8 *NewLcdStr)
{
  u8 StringLength;
  u8 LineDot[122];
  
  StringLength = Char2Dot8X8(NewLcdStr,LineDot);  
  if(LineY + StringLength > 121)  StringLength = 121 - LineY;
  Lcdarray_On(LineX,LineY,StringLength*8,LineDot);    
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
    
    if(DisplayType) 
      Ledarray_Init();
    else            
      Stnlcd_Init();
    
    return(DisplayType);
}


void Ledarray_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11);    

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15);        
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
    
    LcdWriteCmd(0xe2, LcdWrCmd);  // Reset LCD
    LcdWriteCmd(0xc8, LcdWrCmd);
    LcdWriteCmd(0xa0, LcdWrCmd);
    LcdWriteCmd(0xa2, LcdWrCmd); //Sets the LCDdriver voltage bias ratio
    LcdWriteCmd(0x23, LcdWrCmd);
    LcdWriteCmd(0xd2, LcdWrCmd);
    LcdWriteCmd(0x2C, LcdWrCmd);
    delay_ms(60);
    LcdWriteCmd(0x2E, LcdWrCmd);
    delay_ms(5);
    LcdWriteCmd(0x2f, LcdWrCmd);
    delay_ms(5);
    LcdWriteCmd(0xaf, LcdWrCmd);  // 0xaf:display on; 0xae:display off
    LcdWriteCmd(0x2f, LcdWrCmd);

    LcdWriteCmd(0x40, LcdWrCmd);         //Sets the display RAM display start line address    
    
    LcdWriteCmd(0xb0, LcdWrCmd);  //0页
    LcdWriteCmd(0x10, LcdWrCmd);  //0列high位
    LcdWriteCmd(0, LcdWrCmd);     //0列low位
    LcdWriteCmd(0x81, LcdWrCmd);
    //LcdWriteCmd(gs_byBaseGrayVal,LcdWrCmd);
    LcdWriteCmd(50,LcdWrCmd);
    //Lcd_Cls();    


   

    
}

void s_SetXY(u8 x, u8 y)
{
    y &= 0x07;
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
    LcdWriteCmd((0xb0|( y & 0x07)), LcdWrCmd);  //y页字行//
    LcdWriteCmd((0x10|(x>>4)), LcdWrCmd);  //x列high位//
    LcdWriteCmd((x&0x0f), LcdWrCmd);              //x列low位//
    x++;
}

void s_LcdCls(void)
{
    u8 i, line;

    LcdWriteCmd(0xaf, LcdWrCmd);    //display on
    for (line=0; line<=4; line++)
    {
        if(line>=2)
          s_SetXY(0, line+4);
        else
          s_SetXY(0, line);
          
        for (i=0; i<=122; i++) //列号//
        {
            LcdWriteCmd(0, LcdWrData);
        }
    }
    
#if 0    
    memset(gs_abyDisplayData, 0, sizeof(gs_abyDisplayData));
    memset(gs_abyCurrentData, 0, sizeof(gs_abyCurrentData));

    g_byStartY = 0;
    g_byStartX = 0;
#endif    
}

void LcdDotTest(void)
{
    u8 LineString[4][15] = {"VANSTONE","Welcome","2010-06-15","14:28:59"};
    u8 StringLength;
    u8 LineDot[122];
    
    int i=0,k;
    
    s_LcdCls();
    
    //while(1)
    {
      Lcdarray_clear(0x0F);
      
      StringLength = Char2Dot8X8(LineString[0],LineDot);
      Lcdarray_On(0,0,StringLength*8,LineDot);
      
      StringLength = Char2Dot8X8(LineString[1],LineDot);
      Lcdarray_On(1,0,StringLength*8,LineDot);
      
      StringLength = Char2Dot8X8(LineString[2],LineDot);
      Lcdarray_On(2,0,StringLength*8,LineDot);
      
      StringLength = Char2Dot8X8(LineString[3],LineDot);
      Lcdarray_On(3,0,StringLength*8,LineDot);
      
      /*
      set_page_address(2);
      set_column_address(0);
      //for(i=0;i<122/2;i++)  LcdWriteData(0xa5);  
      set_page_address(2);
      set_column_address(0);
      delay_ms(100);
      LcdReadData();//读一dummy data
      for(i=0;i<122;i++)
      {
        //set_column_address(i);
        LineDot[i]=LcdReadData();
        debug_printf(0,0,0,"%02x" ,LineDot[i]);
      }
      */
      WaitNuS(1000*1000);
      LcdBL_Control(0);                         //Switch off 
      Lcdarray_clear(0x0F);
    }
       
}



void Tim2Init(void)
{
/* ---------------------------------------------------------------
  TIM2 Configuration:  Timing Mode:
  TIM2CLK = 36 MHz, Prescaler = 0x9, TIM2 counter clock = 3.6 MHz 
--------------------------------------------------------------- */
  /* Time base configuration */
  //AT skx 
  TIM_TimeBaseStructure.TIM_Period = 0x0fff*5;//low int        
  TIM_TimeBaseStructure.TIM_Prescaler = 0x00;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM2, 9, TIM_PSCReloadMode_Update);

    /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

  //清timer2各中断标志
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4 | TIM_IT_Trigger);
  /* TIM2 disable counter */
  TIM_Cmd(TIM2, DISABLE);
}

/****************以下为系统定时器驱动******************************************/




