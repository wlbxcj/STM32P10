/**
  ******************************************************************************
  * @file    hw_config.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Hardware Configuration & Setup
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/

#include "stm32f10x_map.h"
#include "stm32f10x.h"
#include "stm32f10x_nvic.h"
#include "stm32f10x_lib.h" 

//#include "platform_config.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
//#include "stm32_eval.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

ErrorStatus HSEStartUpStatus;
uint32_t ADC_ConvertedValueX = 0;
uint32_t ADC_ConvertedValueX_1 = 0;
__IO uint16_t  ADC1ConvertedValue = 0, ADC1ConvertedVoltage = 0, calibration_value = 0;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len);
/* Private functions ---------------------------------------------------------*/
#if 0
/*******************************************************************************
* Function Name  : Set_System
* Description    : Configures Main system clocks & power.
* Input          : None.
* Return         : None.
*******************************************************************************/
void Set_System(void)
{
#if defined (STM32F37X) || defined (STM32F30X)
  GPIO_InitTypeDef  GPIO_InitStructure;
#endif /*STM32L1XX_XD */
  
#if defined(USB_USE_EXTERNAL_PULLUP)
  GPIO_InitTypeDef  GPIO_InitStructure;
#endif /* USB_USE_EXTERNAL_PULLUP */
  
  /*!< At this stage the microcontroller clock setting is already configured, 
  this is done through SystemInit() function which is called from startup
  file (startup_stm32xxx.s) before to branch to application main.
  To reconfigure the default setting of SystemInit() function, refer to
  system_stm32xxx.c file
  */ 
  
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS) ||  defined (STM32F37X) ||  defined (STM32F30X)
  /* Enable the SYSCFG module clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
#endif /* STM32L1XX_XD */ 
  
#if !defined(STM32L1XX_MD) && !defined(STM32L1XX_HD) && !defined(STM32L1XX_MD_PLUS) && !defined(STM32F37X) && !defined(STM32F30X)
  /* Enable USB_DISCONNECT GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);
  
  /* ADCCLK = PCLK2/8 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);    
#endif /* STM32L1XX_XD */
  
  /* Configure the used GPIOs*/
  GPIO_Configuration();
  
#if defined(USB_USE_EXTERNAL_PULLUP)
  /* Enable the USB disconnect GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIO_DISCONNECT, ENABLE);
  
  /* USB_DISCONNECT used as USB pull-up */
  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);  
#endif /* USB_USE_EXTERNAL_PULLUP */
  
#if defined(STM32F37X) || defined(STM32F30X)
  
  /* Enable the USB disconnect GPIO clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIO_DISCONNECT, ENABLE);
  
  /*Set PA11,12 as IN - USB_DM,DP*/
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /*SET PA11,12 for USB: USB_DM,DP*/
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_14);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_14);
  
  /* USB_DISCONNECT used as USB pull-up */
  GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
  
#endif
#if defined(STM32L1XX_MD) 
  /* Configure the LEFT button in EXTI mode */
  STM_EVAL_PBInit(Button_LEFT, Mode_EXTI);
  
  /* Configure the RIGHT button in EXTI mode */
  STM_EVAL_PBInit(Button_RIGHT, Mode_EXTI);
#else  
  /* Configure the KEY button in EXTI mode */
  STM_EVAL_PBInit(Button_KEY, Mode_EXTI);
#if !defined(STM32L1XX_HD)&& !defined(STM32L1XX_MD_PLUS) && !defined(STM32F37X) && !defined(STM32F30X) 
  /* Configure the Tamper button in EXTI mode */
  STM_EVAL_PBInit(Button_TAMPER, Mode_EXTI);
#endif /* STM32L1XX_XD */
#endif  
  /* Additional EXTI configuration (configure both edges) */
  EXTI_Configuration();
  
  /* Configure the LEDs */
  STM_EVAL_LEDInit(LED1);
  STM_EVAL_LEDInit(LED2);
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  
#if defined (STM32F30X)
  ADC30x_Configuration();
#else  
  /* Configure the ADC*/
  ADC_Configuration();
#endif
  
}
#endif

#if 1
/**
  * @brief Sets System clock frequency to 72MHz and configure HCLK, PCLK2 
  *        and PCLK1 prescalers. 
  * @param None.
  * @arg None.
  * @note : This function should be used only after reset.
  * @retval value: None.
  */
static void SetSysClockTo72(void)
{
#define  uint8_t unsigned          char
#define  uint16_t unsigned          short
//#define  uint32_t unsigned int
  
#define  RCC_CR_HSEON                        ((unsigned int)0x00010000)        /*!< External High Speed clock enable */
#define  RCC_CR_HSERDY                       ((unsigned int)0x00020000)        /*!< External High Speed clock ready flag */
#define HSEStartUp_TimeOut   ((uint16_t)0x0500) /*!< Time out for HSE start up */
#define  FLASH_ACR_PRFTBE                    ((uint8_t)0x10)               /*!<Prefetch Buffer Enable */
#define  FLASH_ACR_LATENCY                   ((uint8_t)0x03)               /*!<LATENCY[2:0] bits (Latency) */
#define  FLASH_ACR_LATENCY_0                 ((uint8_t)0x00)               /*!<Bit 0 */
#define  FLASH_ACR_LATENCY_1                 ((uint8_t)0x01)               /*!<Bit 0 */
#define  FLASH_ACR_LATENCY_2                 ((uint8_t)0x02)               /*!<Bit 1 */
#define  RCC_CFGR_HPRE_DIV1                  ((unsigned int)0x00000000)        /*!< SYSCLK not divided */
#define  RCC_CFGR_PPRE2_DIV1                 ((unsigned int)0x00000000)        /*!< HCLK not divided */
#define  RCC_CFGR_PPRE1_DIV2                 ((unsigned int)0x00000400)        /*!< HCLK divided by 2 */
#define  RCC_CFGR_PLLSRC                     ((unsigned int)0x00010000)        /*!< PLL entry clock source */
#define  RCC_CFGR_PLLXTPRE                   ((unsigned int)0x00020000)        /*!< HSE divider for PLL entry */
#define  RCC_CFGR_PLLMULL                    ((uint32_t)0x003C0000)        /*!< PLLMUL[3:0] bits (PLL multiplication factor) */
#define  RCC_CFGR_PLLMULL9                   ((uint32_t)0x001C0000)        /*!< PLL input clock*9 */
#define  RCC_CR_PLLON                        ((uint32_t)0x01000000)        /*!< PLL enable */
#define  RCC_CR_PLLRDY                       ((uint32_t)0x02000000)        /*!< PLL clock ready flag */
#define  RCC_CFGR_SW                         ((uint32_t)0x00000003)        /*!< SW[1:0] bits (System clock Switch) */
#define  RCC_CFGR_SW_PLL                     ((uint32_t)0x00000002)        /*!< PLL selected as system clock */

#define  RCC_CFGR_SWS                        ((uint32_t)0x0000000C)        /*!< SWS[1:0] bits (System Clock Switch Status) */
  
  volatile unsigned int  StartUpCounter = 0, HSEStatus = 0;
  
  /*!< SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/    
  /*!< Enable HSE */    
  RCC->CR |= ((unsigned int)RCC_CR_HSEON);
 
  /*!< Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSEStartUp_TimeOut));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (unsigned int)0x01;
  }
  else
  {
    HSEStatus = (unsigned int)0x00;
  }  

  if (HSEStatus == (unsigned int)0x01)
  {
    /*!< Enable Prefetch Buffer */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /*!< Flash 2 wait state */
    FLASH->ACR &= (unsigned int)((unsigned int)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (unsigned int)FLASH_ACR_LATENCY_2;    
 
    /*!< HCLK = SYSCLK */
    RCC->CFGR |= (unsigned int)RCC_CFGR_HPRE_DIV1;
      
    /*!< PCLK2 = HCLK */
    RCC->CFGR |= (unsigned int)RCC_CFGR_PPRE2_DIV1;
    
    /*!< PCLK1 = HCLK */
    RCC->CFGR |= (unsigned int)RCC_CFGR_PPRE1_DIV2;
    
    /*!< PLLCLK = 8MHz * 9 = 72 MHz */
    RCC->CFGR &= (unsigned int)((unsigned int)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (unsigned int)(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);

    /*!< Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /*!< Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    /*!< Select PLL as system clock source */
    RCC->CFGR &= (unsigned int)((unsigned int)~(RCC_CFGR_SW));
    RCC->CFGR |= (unsigned int)RCC_CFGR_SW_PLL;    

    /*!< Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (unsigned int)RCC_CFGR_SWS) != (unsigned int)0x08)
    {
    }
  }
  else
  { /*!< If HSE fails to start-up, the application will have wrong clock 
         configuration. User can add here some code to deal with this error */    

    /*!< Go to infinite loop */
    while (1)
    {
    }
  }
}

void SystemInit (void)
{
  /*!< RCC system reset(for debug purpose) */
  /*!< Set HSION bit */
  RCC->CR |= (unsigned int)0x00000001;
  /*!< Reset SW[1:0], HPRE[3:0], PPRE1[2:0], PPRE2[2:0], ADCPRE[1:0] and MCO[2:0] bits */
  RCC->CFGR &= (unsigned int)0xF8FF0000;  
  /*!< Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (unsigned int)0xFEF6FFFF;
  /*!< Reset HSEBYP bit */
  RCC->CR &= (unsigned int)0xFFFBFFFF;
  /*!< Reset PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE bits */
  RCC->CFGR &= (unsigned int)0xFF80FFFF;
  /*!< Disable all interrupts */
  RCC->CIR = 0x00000000;
    
  /*!< Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
  /*!< Configure the Flash Latency cycles and enable prefetch buffer */
  SetSysClockTo72();

}
#endif

/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz).
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_USBClock(void)
{
  /* Select USBCLK source */
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  
  /* Enable the USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : Enter_LowPowerMode.
* Description    : Power-off system clocks and power while entering suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode.
* Description    : Restores system clocks and power while exiting suspend mode.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
    DEVICE_INFO *pInfo = &Device_Info;

    /* Set the device state to the correct state */
    if (pInfo->Current_Configuration != 0)
    {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
    else 
    {
        bDeviceState = ATTACHED;
    }
    SystemInit();
#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
    /* Enable The HSI (16Mhz) */
    RCC_HSICmd(ENABLE); 
#endif
#if defined(STM32F30X)
    ADC30x_Configuration();
#endif
}

/*******************************************************************************
* Function Name  : USB_Config.
* Description    : Configures the USB interrupts.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    EXTI_InitTypeDef EXTI_InitStructure;

    /* 2 bit for pre-emption priority, 2 bits for subpriority */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
    /**/
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;        //USB米赤車??豕???D?????車
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;         //?角??車??豕?? 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;                //℅車車??豕???a1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;         //USB??車??豕???D?????車
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;         //?角??車??豕?? 1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;                //℅車車??豕???a0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    /* Enable the USB Wake-up interrupt*/  
    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);

    /* Configure the EXTI line 18 connected internally to the USB IP */
    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Line = EXTI_Line18; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);


    GPIO_Configuration();
    Set_USBClock();
}

/*******************************************************************************
* Function Name  : USB_Cable_Config.
* Description    : Software Connection/Disconnection of USB Cable.
* Input          : NewState: new state.
* Output         : None.
* Return         : None
*******************************************************************************/
void USB_Cable_Config (FunctionalState NewState)
{ 
    if (NewState != DISABLE)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_11);
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_11);
    }

}

/*******************************************************************************
* Function Name  : GPIO_Configuration
* Description    : Configures the different GPIO ports.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  

    /* USB_DISCONNECT used as USB pull-up*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_11);
}

/*******************************************************************************
* Function Name : EXTI_Configuration.
* Description   : Configure the EXTI lines for Key and Tamper push buttons.
* Input         : None.
* Output        : None.
* Return value  : The direction value.
*******************************************************************************/
void EXTI_Configuration(void)
{
#if 0
  EXTI_InitTypeDef EXTI_InitStructure;

#if defined (USE_STM32L152_EVAL)
  /* Configure RIGHT EXTI line to generate an interrupt on rising & falling edges */  
  EXTI_InitStructure.EXTI_Line = RIGHT_BUTTON_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Clear the RIGHT EXTI line pending bit */
  EXTI_ClearITPendingBit(RIGHT_BUTTON_EXTI_LINE);
   
  /* Configure LEFT EXTI Line to generate an interrupt rising & falling edges */  
  EXTI_InitStructure.EXTI_Line = LEFT_BUTTON_EXTI_LINE;
  EXTI_Init(&EXTI_InitStructure);

  /* Clear the LEFT EXTI line pending bit */
  EXTI_ClearITPendingBit(LEFT_BUTTON_EXTI_LINE);
  
#else    
  /* Configure Key EXTI line to generate an interrupt on rising & falling edges */  
  EXTI_InitStructure.EXTI_Line = KEY_BUTTON_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Clear the Key EXTI line pending bit */
  EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
   
  /* Configure Tamper EXTI Line to generate an interrupt rising & falling edges */  
#if !defined (USE_STM32L152D_EVAL) && !defined (STM32F30X)
  EXTI_InitStructure.EXTI_Line = TAMPER_BUTTON_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Clear the Tamper EXTI line pending bit */
  EXTI_ClearITPendingBit(TAMPER_BUTTON_EXTI_LINE);
#endif /* USE_STM32L152_EVAL */  
#endif
  EXTI_ClearITPendingBit(EXTI_Line18);
  EXTI_InitStructure.EXTI_Line = EXTI_Line18; 
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
#endif
}

/*******************************************************************************
* Function Name : ADC_Configuration.
* Description   : Configure the ADC and DMA.
* Input         : None.
* Output        : None.
* Return value  : The direction value.
*******************************************************************************/
void ADC_Configuration(void)
{
#if 0
  ADC_InitTypeDef ADC_InitStructure;
  DMA_InitTypeDef DMA_InitStructure;

  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    
  /* DMA1 channel1 configuration ---------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValueX;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
   
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* Enable the DMA1 Channel1 Transfer complete interrupt */
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

#if defined(STM32L1XX_MD) || defined(STM32L1XX_HD)|| defined(STM32L1XX_MD_PLUS)
  /* Enable the HSI for the ADC operations */
  RCC_HSICmd(ENABLE);
    
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_StructInit(&ADC_InitStructure);
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  
#if defined (USE_STM32L152D_EVAL)
  /* ADC1 regular channel31 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_31, 1, ADC_SampleTime_384Cycles);
  
#else
  /* ADC1 regular channel18 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_18, 1, ADC_SampleTime_384Cycles);
#endif
  
#if !defined (USE_STM32373C_EVAL)
  /* Enable the request after last transfer for DMA Circular mode */
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
#endif
  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
#else
  /* ADC1 configuration ------------------------------------------------------*/
#if !defined (USE_STM32373C_EVAL)
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
#endif
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_AIN_CHANNEL, 1, ADC_SampleTime_55Cycles5);

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));  
#endif /* STM32L1XX_MD */
#endif
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

#if 0
  Device_Serial0 = *(uint32_t*)(0x1FF80050);
  Device_Serial1 = *(uint32_t*)(0x1FF80054);
  Device_Serial2 = *(uint32_t*)(0x1FF80064);
#else   
  Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
  Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
  Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);
#endif /* STM32L1XX_MD */ 
  
  Device_Serial0 += Device_Serial2;

  if (Device_Serial0 != 0)
  {
    IntToUnicode (Device_Serial0, &CustomHID_StringSerial[2] , 8);
    IntToUnicode (Device_Serial1, &CustomHID_StringSerial[18], 4);
  }
}

/*******************************************************************************
* Function Name  : HexToChar.
* Description    : Convert Hex 32Bits value into char.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10; 
    }
    
    value = value << 4;
    
    pbuf[ 2* idx + 1] = 0;
  }
}

/*******************************************************************************
* Function Name : ADC30x_Configuration
* Description   : Configure the ADC and DMA.
* Input         : None.
* Output        : None.
* Return value  : The direction value.
*******************************************************************************/
#if defined (STM32F30X)

void ADC30x_Configuration(void)
{
  ADC_InitTypeDef    ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  DMA_InitTypeDef  DMA_InitStructure;
  
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  /* DMA1 channel1 configuration ---------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_ConvertedValueX;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
  /* Enable the DMA1 Channel1 Transfer complete interrupt */
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  
  /* Configure the ADC clock */  
  RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div2);
  
  /* ADC1 Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
  ADC_DeInit(ADC1);
  ADC_StructInit(&ADC_InitStructure);
  
  /* Calibration procedure */
  ADC_VoltageRegulatorCmd(ADC1, ENABLE);
  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC1);
  
  while(ADC_GetCalibrationStatus(ADC1) != RESET );
  calibration_value = ADC_GetCalibrationValue(ADC1);
  
  /* Configure the ADC1 in continuous mode */  
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                    
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
  
  /* ADC1 DMA Enable */
  ADC_DMACmd(ADC1, ENABLE);
  ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);
  
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;  
  ADC_InitStructure.ADC_NbrOfRegChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  
  /* ADC1 regular channel7 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_7Cycles5);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
  
  /* wait for ADRDY */
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));
  
  /* Start ADC1 Software Conversion */ 
  ADC_StartConversion(ADC1);
  
  /* Test EOC flag */
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
  /* Get ADC1 converted data */
  
  ADC_ConvertedValueX =ADC_GetConversionValue(ADC1);
}

#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
