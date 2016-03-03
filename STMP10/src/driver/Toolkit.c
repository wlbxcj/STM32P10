/// @file   Toolkit.c
/// This is for module @ref TaxBox
/// @author Travis Tu
/// @date   2006-Sep-1st

#include "TaxBox.h"
#include <string.h>
//#define APPLICATIONADDRESS 0x5000       //SKX FOR APP RUN STARTADDR IS 0X8005000
//#define APPLICATIONADDRESS 0x0000       //SKX FOR APP RUN STARTADDR IS 0X8005000  ->1000(12/10/24)
//#define APPLICATIONADDRESS 0x1000       //SKX FOR APP RUN STARTADDR IS 0X8005000  ->1000(12/10/24)
#define APPLICATIONADDRESS 0x6800       //0x9000->0x6800 13/01/24
//#define SELFBOOT
extern RTCTimeType CurrentTime;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;
vu16 CCR1_Val = 0x8ca0;
/*Const global variable*/
/*CRC-16 Look up Table*/
const u16 CRC16Table[256] = {
                              0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
                              0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
                              0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
                              0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
                              0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
                              0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
                              0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
                              0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
                              0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
                              0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
                              0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
                              0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
                              0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
                              0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
                              0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
                              0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
                              0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
                              0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
                              0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
                              0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
                              0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
                              0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
                              0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
                              0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
                              0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
                              0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
                              0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
                              0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
                              0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
                              0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
                              0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
                              0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
                          };

/// @param data  \b Point to the data array
/// @param length  \b Length for the data
/// @param cs  \b Point to output data
/// @return None
void CalulateCRC16(const u8* data, u16 length, u8* cs)
{
    u16 crc = 0;

    while (length-- != 0)
    {
        crc = (crc << 8) ^ CRC16Table[((crc >> 8) ^ (*data++)) & 0xFF];
    }

    cs[0] = crc >> 8;
    cs[1] = (crc & 0xFF);
}

void SysTick_Config(void)
{
    /* Configure HCLK clock as SysTick clock source */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

    ///* SysTick interrupt each 1000000Hz with HCLK equal to 48MHz */
    SysTick_SetReload(0xFFFFFFFF);

    /* Enable the SysTick Interrupt */
    SysTick_ITConfig(ENABLE);
}

bool SysTickReached = FALSE;
/// @param N \b value for wait time max 4 294 967 295 uS
/// @return None
/// Maximum 89 Seconds  means 89000000 uS
//AT skx
void WaitNuS(u32 x)//50us delay real is 54us
{
  volatile unsigned int i=0,j=0;
  for(i=0;i<x;i++)
  {
    for(j=0;j<0x05;j++);
  }
}
/*
void WaitNuS(u32 N)
{
    do
    {
        u32 NThisRound;
        if(N>(0xFFFFFF/SYSTEM_PLL_CLOCK))
        {
            NThisRound=(0xFFFFFF/SYSTEM_PLL_CLOCK);
            N-=(0xFFFFFF/SYSTEM_PLL_CLOCK);
        }
        else
        {
            NThisRound=N;
            N=0;
        }

        if (NThisRound > 5)
            NThisRound -= 5;
        else
            NThisRound = 1;

        SysTick_SetReload(NThisRound*SYSTEM_PLL_CLOCK);
        //Enable the SysTick Counter
        SysTick_CounterCmd(SysTick_Counter_Enable);
        //just wait
        while(!SysTickReached){;}
        SysTickReached = FALSE;
    }while(N!=0);
}
*/

void SendWaitInit(void)
{


/* ---------------------------------------------------------------
  TIM2 Configuration: Output Compare Timing Mode:
  TIM2CLK = 36 MHz, Prescaler = 0x4, TIM2 counter clock = 7.2 MHz 
  CC1 update rate = TIM2 counter clock / CCR1_Val = 146.48 Hz
  CC2 update rate = TIM2 counter clock / CCR2_Val = 219.7 Hz 
  CC3 update rate = TIM2 counter clock / CCR3_Val = 439.4 Hz 
  CC4 update rate = TIM2 counter clock / CCR4_Val =  878.9 Hz 
--------------------------------------------------------------- */
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;          
  TIM_TimeBaseStructure.TIM_Prescaler = 0x00;       
  TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
  
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM2, 9, TIM_PSCReloadMode_Immediate);

  /* Output Compare Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;          
  TIM_OCInitStructure.TIM_Channel = TIM_Channel_1;          
  TIM_OCInitStructure.TIM_Pulse = CCR1_Val;  
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    
  TIM_OCInit(TIM2, &TIM_OCInitStructure);

  TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);
    /* TIM IT enable */
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);
}
/// @param buf \b Point to the data
/// @param len \b Length for the data
/// @return \b Result of the LRC compute
u8 ComputeLRC(const u8 *buf,u16 len)
{
    u8 i,lrc = 0;

    for (i = 0; i < len; i++)
    {
        lrc ^= *buf;
        buf++;
    }

    return lrc;
}

/// @param pStatus \b Status of the user card
/// @return \b Result
bool TOOLKIT_Judge_Declared(u8 *pStatus)
{
    u8 ucI;

    for (ucI = 1; ucI < 0xFF; ucI++)
    {
        if (!ICCReadRecord(User_ICC, 0x04, ucI, 220))
        {                                                   // 没有相应的纪录文件
            if (APDUBuf.APDURsp.SW != 0x6A83)
            {
                Response_ER(UART_COMMAND_READ_USER_ICC_ERROR);
                return FALSE;
            }
            else
                break;
        }
        else
        {
            if (MemEql(POSUserInfo.FiscalICCNB, &APDUBuf.APDURsp.Data[1], 8))
            {
                if (APDUBuf.APDURsp.Data[0] != 0x00)
                {
                    if (AppFlag.bDeclareAbortFlag)
                        *pStatus = 0x01;
                    else
                    {
                        Response_ER(UART_COMMAND_ALREADY_DECLARE_RECORD);
                        return FALSE;
                    }
                }
            }
        }
    }
    return TRUE;
}

//length include the 4 bytes header and paremeter portion but without Check Sum inside UARTRsp
void AddCSAndSendUARTRsp(u8* cmd, u8 length)
{
    CalulateCRC16(cmd, length, cmd + length);
    GetU16(cmd + length);
    //Comm_SendPacket(cmd, length + 2);//send 4 bytes Header and parameter
}

/// @param None
/// @return None
void SYSCLOCKInit(void)
{
    ErrorStatus HSEStartUpStatus;

    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if(HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);

        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);

        /* PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1);

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* PLLCLK = 8MHz * 9 = 72 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        
        /* Enable PLL */
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){}

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08){}
    }

    /* Enable peripheral clocks --------------------------------------------------*/
    /* Enable GPIOC, GPIOE and AFIO clocks */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO  |
                            RCC_APB2Periph_GPIOA |
                            RCC_APB2Periph_GPIOB |
                            RCC_APB2Periph_GPIOC |
                            RCC_APB2Periph_GPIOD , ENABLE);

 
    
    /* Enable I2C1 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    
    /* Enable SPI1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    
    /* Enable SPI3 */ //13/08/12
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    
#if 0    //usart3 use spi2 (spi2_nss/usart3_ck)
    //AT enable spi2 clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
#endif
    /* enable UART3 for communication with PC */
    //RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART3, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    
	  /* TIM2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	  /* TIM3 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 
    
    //AT skx
    /* TIM4 clock enable for sys timer*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    
    //Full SWJ Disabled (JTAG-DP + SW-DP) 12/09/21
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
    
      /* Enable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);


}
#if 0
/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz)
* Input          : None.
* Return         : None.
*******************************************************************************/
void Set_USBClock(void)
{
    /* USBCLK = PLLCLK/1.5 =48 MHz */
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
    /* Enable USB clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
}


#endif
/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures NVIC and Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Set the Vector Table base location at 0x08001000 */
#ifdef SELFBOOT
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
#else
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, APPLICATIONADDRESS);
#endif


  
  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);//8个抢占，2个子优先级


    // configure the UART interrupt for com
    /* Enable the USART3 Interrupt */
    
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;
    
    //AT skx
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    //AT skx
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //usart2
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;
    
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    //AT skx
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Enable the EXTI0 Interrupt */
    //FOR KF311
    //SPI2数据接收中断与RF外部中断设定为最高优先级，子优先级RF大于SPI2
    //NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;  //RF外部中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    //AT skx
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the TIM3 gloabal Interrupt for sys timer*/
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the SPI2 gloabal Interrupt */
  SPI_ClearITPendingBit(SPI2,SPI_FLAG_RXNE);
  NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
    // configure the ...
  //keyboard
  NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the RTC Interrupt */  //12/11/07
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQChannel;
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable TAMPER IRQChannel */   //12/10/25
  NVIC_InitStructure.NVIC_IRQChannel = TAMPER_IRQChannel;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
}
#if 0
void USB_Interrupts_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    //configure the interrupt for USB Communication
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN_RX0_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
#endif
/*
void USB_Interrupts_Disable(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    //configure the interrupt for USB Communication
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN_RX0_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}
*/
/// @param None
/// @return None
void InitExtInt(void)
{
    // configuer the Power lost interrupt
    /* Configure PC10 as input floating (EXTI10) */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Connect EXTI Line10 to Pc10 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource10);

    /* Configure EXTI Line10 to generate an interrupt on falling edge */
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

//void InitUSBHost(void)
//{
//    // configuer the Power lost interrupt
//    /* Configure PB0 as input floating (EXTI0) */
//    GPIO_InitTypeDef GPIO_InitStructure;
//    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0;
//    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
//    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//    /* Connect EXTI Line0 to Pb0 */
//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
//
//    /* Configure EXTI Line0 to generate an interrupt on falling edge */
//    EXTI_InitTypeDef EXTI_InitStructure;
//    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
//    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);
//}

/// @param None
/// @return None
void Flashing(void)
{
    u32 j = 5;

    while (j--)
    {
        LED_Show(ORANGE_LED,0);
        WaitNuS(25000);
        LED_Show(ORANGE_LED,1);
        WaitNuS(25000);
    }
}

/// @param value \b Point to Big Endian data
/// @return \b A unsigned long data
u32 GetU32(void* value)
{
    return (((u8*)value)[0] << 24) | (((u8*)value)[1] << 16)
        | (((u8*)value)[2] << 8) | (((u8*)value)[3]);
}

/// @param value \b Point to Big Endian data
/// @param fact \b A unsigned long data
/// @return None
void SetU32(void* value, u32 fact)
{
    ((u8*)value)[0] = (fact >> 24) & 0xFF;
    ((u8*)value)[1] = (fact >> 16) & 0xFF;
    ((u8*)value)[2] = (fact >> 8) & 0xFF;
    ((u8*)value)[3] = (fact) & 0xFF;
}

/// @param value \b Point to Big Endian data
/// @return None
void IncU32(void* value)
{
    u32 temp = GetU32(value);
    temp++;
    SetU32(value,temp);
}

/// @param value \b Point to Big Endian data
/// @param fact \b A unsigned long data
/// @return None
void AddU32(void* value, u32 fact)
{
    u32 temp = GetU32(value);
    temp += fact;
    SetU32(value,temp);
}

/// @param value \b Point to input Big Endian data
/// @return \b A unsigned short data
u16 GetU16(void* value)
{
    return (((u8*)value)[0]<<8)|
           (((u8*)value)[1]);
}

/// @param value \b Point to Big Endian data
/// @param fact \b A unsigned short data
/// @return None
void SetU16(void* value, u16 fact)
{
    ((u8*)value)[0] = (fact>>8)&0xFF;
    ((u8*)value)[1] = (fact)&0xFF;
}

/// @param value \b Point to Big Endian data
/// @return None
void IncU16(void* value)
{
    u16 temp = GetU16(value);
    temp++;
    SetU16(value,temp);
}

/// @param value \b Point to Big Endian data
/// @param fact \b A unsigned short data
/// @return None
void AddU16(void* value, u16 fact)
{
    u16 temp = GetU16(value);
    temp += fact;
    SetU16(value,temp);
}

/// @param pucDest \b Point to BCD data
/// @param uiLen \b A Length for the conventioned data
/// @param pulSrc \b Point to LONG data
/// @return None
///LONG convention BCD
void TOOLKIT_LongBcd(u8 *pucDest, u16 uiLen,u32 *pulSrc)
{
    u16 uiI;
    u32 ulTmp1,ulTmp2;
    u8 aucTab[5];

    ulTmp1 = *pulSrc;
    ulTmp2 = 100000000L;

    for (uiI = 0; uiI < 5; uiI++)
    {
        aucTab[uiI] = (u8)(ulTmp1 / ulTmp2) ;
        aucTab[uiI] = ((aucTab[uiI] / 10 ) << 4 ) + (aucTab[uiI] % 10);
        ulTmp1 = ulTmp1 % ulTmp2;
        ulTmp2 = ulTmp2 / 100;
    }

    memset( pucDest, 0x00, uiLen ) ;
    pucDest += uiLen ;

    if ( uiLen  > 5)
        uiLen = 5 ;

    for (uiI = 0; uiI < uiLen; uiI++)
        *--pucDest = aucTab[4 - uiI] ;
}

/// @param pucSrc \b Point to BCD data
/// @param uiLen \b A Length for the conventioned data
/// @return \b A unsigned long data
u32 TOOLKIT_BcdLong(u8 *pucSrc, u16 uiSrcLen)
{
    u16 uiI;
    u8 ucCh;
    u32 ulTmp1,ulTmp2;

    ulTmp1 = 0 ;
    ulTmp2 = 1 ;
    pucSrc += (uiSrcLen + 1) / 2;

    for (uiI = 0; uiI< uiSrcLen ; uiI++)
    {
        if (uiI % 2)
            ucCh = (*pucSrc >> 4 ) & 0x0F;
        else
            ucCh = *--pucSrc & 0x0F;

        ulTmp1 += ulTmp2 * ucCh ;

        if (ulTmp2 == 1000000000L )
            ulTmp2 = 0 ;
        else
            ulTmp2 *= 10 ;
    }

    return (ulTmp1);
}

/// @param uiYear \b The year
/// @return \b
///判断闰年
u8 TOOLKIT_NotLeap(u16 uiYear)
{
    if (!(uiYear % 100))
        return(uiYear % 400);
    else
        return(uiYear % 4);
}

/// @param pucDate \b Point to the day
/// @return None
void TOOLKIT_DatePlusOneDay(u8 *pucDate)
{
    u32 Year, Month, Day;

    Year =  TOOLKIT_BcdLong(pucDate, 4);
    Month = TOOLKIT_BcdLong(pucDate + 2, 2);
    Day =   TOOLKIT_BcdLong(pucDate + 3, 2);

    if (++Day == 32)
    {
        Month++;
        Day = 1;
    }else if (Day == 31)
    {
        if ((Month == 4) || (Month == 6)
                || (Month == 9) || (Month == 11))
        {
            Month++;
            Day = 1;
        }
    }else if (Day == 30)
    {
        if (Month == 2)
        {
            Month++;
            Day = 1;
        }
    }else if (Day == 29)
    {
        if ( (Month == 2) && (TOOLKIT_NotLeap(Year)))
        {
            Month++;
            Day = 1;
        }
    }

    if (Month == 13)
    {
        Month = 1;
        Year++;
    }

    TOOLKIT_LongBcd(pucDate, 2, &Year);
    TOOLKIT_LongBcd(pucDate + 2, 1, &Month);
    TOOLKIT_LongBcd(pucDate + 3, 1, &Day);
}

/// @param pucDate \b Point to the day CCYYMM
/// @param pucDay \b Point to the output day
/// @return None
void TOOLKIT_MonthDay(unsigned char *pucDate,unsigned char *pucDay)
{
    u32 Year, Month;

    Year =  TOOLKIT_BcdLong(pucDate, 4);
    Month = TOOLKIT_BcdLong(pucDate + 2, 2);

    switch ( Month )
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        *pucDay = 0x31;
        break;
    case 4:
    case 6:
    case 9:
    case 11:
        *pucDay = 0x30;
        break;
    case 2:
        if (TOOLKIT_NotLeap(Year))
            *pucDay = 0x28;
        else
            *pucDay = 0x29;
        break;
    default:
        break;
    }
}

/// @param pTime \b Point to the time
/// @return \b Result for the check
bool TOOLKIT_ValideTimeInt(u8 *pTime)
{
    if ((*pTime > 0x09) && (*pTime < 0x10))
        return FALSE;

    if ((*pTime > 0x19) && (*pTime < 0x20))
        return FALSE;

    if ((*pTime > 0x23) && (*pTime < 0x30))
        return FALSE;

//	if (((*(pTime + 1) & 0xF0) > 0x50) || ((*(pTime + 1) & 0x0F) > 0x09))
//		return FALSE;

//	if (((*(pTime + 2) & 0xF0) > 0x50) || ((*(pTime + 2) & 0x0F) > 0x09))
//		return FALSE;

    return TRUE;
}

/// @param pDate \b Point to the Day
/// @return \b Result for the check
const u8 Toolkit_Month2Day_Unleap_Year[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
const u8 Toolkit_Month2Day_Leap_Year[13]  ={0,31,29,31,30,31,30,31,31,30,31,30,31};
bool DATE_ValideDateStr(u8 *pDate)
{
    return TRUE;
    /*
        u8 i;
        u16 year,month,day;
        //check charactor
        for(i=0;i<8;i++)
            if( (pDate[i]<'0') || (pDate[i]>'9') )
               return FALSE;
        //get interger number
        year  =(pDate[0]-'0')*1000+(pDate[1]-'0')*100+(pDate[2]-'0')*10+(pDate[3]-'0');
        month =(pDate[4]-'0')*10  +(pDate[5]-'0');
        day   =(pDate[6]-'0')*10  +(pDate[7]-'0');
        //check the year
        if( (year<1980) || (year>9999) )
            return FALSE;
        //check the month
        if( (month<1)   || (month>12) )
            return FALSE;
        //check the Day
        if( (year%4) != 0) //not闰年
        {
            if( (day<1) || (day>Toolkit_Month2Day_Unleap_Year[month]) )
    			return FALSE;
        }
    	else//闰年
        {
            if( (day<1) || (day>Toolkit_Month2Day_Leap_Year[month]) )
                return FALSE;
        }
        return TRUE;
    */
}

/// @param None
/// @return None
/* for kf322
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_8);
    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_12 | GPIO_Pin_13);    
}
*/

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    
    //GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4|GPIO_Pin_5 | GPIO_Pin_6| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6| GPIO_Pin_7;//sxl
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    //sxl
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13| GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
#if 0   
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif   
   // GPIO_ResetBits(GPIOB, GPIO_Pin_4|GPIO_Pin_5 | GPIO_Pin_6| GPIO_Pin_7);
    GPIO_ResetBits(GPIOB, GPIO_Pin_6| GPIO_Pin_7);
    GPIO_ResetBits(GPIOA, GPIO_Pin_13| GPIO_Pin_14);//sxl
    
//    GPIO_ResetBits(GPIOC, GPIO_Pin_8);    
}

/// @param LEDValue \b Select the LED
/// @param ShowType \b show status
/// @return None

//VER10       GREEN B7; ORANGE B6;  RED B8
//VER11       BLUE C13; ORANGE C12;  RED B8

/* FOR KF322
void LED_Show(LEDType LEDValue, u8 ShowType)
{
    if(ShowType)
    {
        switch (LEDValue)
        {
            case BLUE_LED:
                GPIO_SetBits(GPIOC, GPIO_Pin_13);
                break;
            case ORANGE_LED:
                GPIO_SetBits(GPIOC, GPIO_Pin_12);
                break;
            case RED_LED:
                GPIO_SetBits(GPIOB, GPIO_Pin_8);
                break;
            default:
                break;
        }
    }
    else
    {
        switch (LEDValue)
        {
            case BLUE_LED:
                GPIO_ResetBits(GPIOC, GPIO_Pin_13);
                break;
            case ORANGE_LED:
                GPIO_ResetBits(GPIOC, GPIO_Pin_12);
                break;
            case RED_LED:
                GPIO_ResetBits(GPIOB, GPIO_Pin_8);
                break;
            default:
                break;
        }
    }
}
*/

//FOR KF311
void LED_Show(LEDType LEDValue, u8 ShowType)
{
    if(ShowType)
    {
        switch (LEDValue)
        {
            case BLUE_LED:
               GPIO_SetBits(GPIOB, GPIO_Pin_7);
                break;
            case ORANGE_LED:
                GPIO_SetBits(GPIOB, GPIO_Pin_6);
                break;
            case GREEN_LED:
                //GPIO_SetBits(GPIOB, GPIO_Pin_5);
              GPIO_SetBits(GPIOA, GPIO_Pin_14);//sxl
                break;
            case RED_LED:
               // GPIO_SetBits(GPIOB, GPIO_Pin_4);
              GPIO_SetBits(GPIOA, GPIO_Pin_13);//sxl
                break;
            default:
                break;
        }
    }
    else
    {
        switch (LEDValue)
        {
            case BLUE_LED:
              GPIO_ResetBits(GPIOB, GPIO_Pin_7);
                break;
            case ORANGE_LED:
              GPIO_ResetBits(GPIOB, GPIO_Pin_6);
                break;
            case GREEN_LED:
                //GPIO_ResetBits(GPIOB, GPIO_Pin_5);
               GPIO_ResetBits(GPIOA, GPIO_Pin_14);//sxl
                break;
            case RED_LED:
                //GPIO_ResetBits(GPIOB, GPIO_Pin_4);
                 GPIO_ResetBits(GPIOA, GPIO_Pin_13);//sxl
                break;
            default:
                break;
        }
    }
}


void Protect_Switch_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC, GPIO_Pin_11);
}

void Protect_Switch(u8 State)
{
    if(State)
    {
                GPIO_SetBits(GPIOC, GPIO_Pin_11);
    }
    else
    {      
                GPIO_ResetBits(GPIOC, GPIO_Pin_11);
        }

}



unsigned long TOOLKIT_AscLong(unsigned char *pucSrc, unsigned int uiLen)
{
	unsigned int uiI;
	unsigned long ulTmp1,ulTmp2;

    ulTmp1 = 0 ;
    ulTmp2 = 1 ;
    pucSrc += uiLen;

    for (uiI = 0; uiI < uiLen ; uiI++)
    {
        ulTmp1 += (ulTmp2 * (*--pucSrc & 0x0F) ) ;
        ulTmp2 *= 10 ;
    }

    return (ulTmp1) ;
}

void TOOLKIT_LongAsc(unsigned char  *pucDest, unsigned int uiLen, unsigned long pulSrc)
{
	unsigned int uiI;
	unsigned char ucCh;
	unsigned char aucTab[20];
	unsigned long ulTmp1, ulTmp2;

    ulTmp1 = pulSrc;
    ulTmp2 = 100000000L ;

    for (uiI = 0; uiI < 5; uiI++)
    {
		ucCh = (unsigned char)(ulTmp1 / ulTmp2) ;
		aucTab[2 * uiI] = ucCh / 10 + 0x30 ;
		aucTab[2 * uiI + 1] = ucCh % 10 + 0x30;
        ulTmp1 = ulTmp1 % ulTmp2;
        ulTmp2 = ulTmp2 / 100;
    }

	memset( pucDest, 0x30, uiLen ) ;
	pucDest += uiLen ;

	if ( uiLen > 10 )
		uiLen = 10 ;
	for( uiI=0; uiI < uiLen;uiI++)
		*--pucDest = aucTab[9-uiI] ;
}

// 1:补码->原码；0：原码->补码
u32 TOOLKIT_NumberCodeConv(u32 Data, u8 ResumeFlag)
{
	if (ResumeFlag)
	{
		if (Data & 0x80000000)
		{
			Data = ((Data - 1) ^ 0x7FFFFFFF) & 0x7FFFFFFF;
			return (Data);
		}else
			return (Data);
	}else
	{
		if (Data & 0x80000000 )
		{	// 负数
			Data = ((Data ^ 0x7FFFFFFF) + 1) | 0x80000000;
			return (Data);
		}else	// 正数
			return (Data);
	}
}


/// @param pTime \b Date natural format
/// @param pTime \b Date compress format
void TOOLKIT_DateCompress(u8 *pPlainDate, u8 *pCompressDate)
{
    u32 Year, Month, Day, Hour, Minute;
    u32 CompressDate;

    Year = TOOLKIT_BcdLong(++pPlainDate, 2);
    Month = TOOLKIT_BcdLong(++pPlainDate, 2);
    Day = TOOLKIT_BcdLong(++pPlainDate, 2);
    Hour = TOOLKIT_BcdLong(++pPlainDate, 2);
    Minute = TOOLKIT_BcdLong(++pPlainDate, 2);
    CompressDate = Year * 0x00100000 + Month * 0x00010000 + Day * 0x00000800 + Hour * 0x00000040 + Minute;
    CompressDate = GetU32(&CompressDate);
    memcpy(pCompressDate, &CompressDate, 4);
}





