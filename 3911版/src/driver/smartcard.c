
//#include "stm32f10x.h"
#include "stm32f10x_lib.h"
#include "Toolkit.h"
#include "smartcard.h"


#if 0

/* Global variables definition and initialization ----------------------------*/
SC_ATR SC_A2R;
uint8_t SC_ATR_Table[40];
static volatile uint8_t SCData = 0;
static uint32_t F_Table[16] = {0, 372, 558, 744, 1116, 1488, 1860, 0,
                               0, 512, 768, 1024, 1536, 2048, 0, 0};
static uint32_t D_Table[8] = {0, 1, 2, 4, 8, 16, 0, 0};

/* Private function prototypes -----------------------------------------------*/
/* Transport Layer -----------------------------------------------------------*/
/*--------------APDU-----------*/
static void SC_SendData(SC_ADPU_Commands *SC_ADPU, SC_ADPU_Responce *SC_ResponceStatus);

/*------------ ATR ------------*/
static void SC_AnswerReq(SC_State *SCState, uint8_t *card, uint8_t length);  /* Ask ATR */
static uint8_t SC_decode_Answer2reset(uint8_t *card);  /* Decode ATR */

/* Physical Port Layer -------------------------------------------------------*/
//static void SC_Init(void);
 void SC_Init(void);

static void SC_DeInit(void);
static void SC_VoltageConfig(uint32_t SC_Voltage);
static uint8_t SC_Detect(void);
static ErrorStatus USART_ByteReceive(uint8_t *Data, uint32_t TimeOut);

/**
  * @brief  Handles all Smartcard states and serves to send and receive all
  *   communication data between Smartcard and reader.
  * @param  SCState: pointer to an SC_State enumeration that will contain the Smartcard state.
  * @param  SC_ADPU: pointer to an SC_ADPU_Commands structure that will be initialized.  
  * @param  SC_Response: pointer to a SC_ADPU_Responce structure which will be initialized.
  * @retval None
  */
void SC_Handler(SC_State *SCState, SC_ADPU_Commands *SC_ADPU, SC_ADPU_Responce *SC_Response)
{
  uint32_t i = 0;

  switch(*SCState)
  {
    case SC_POWER_ON:
      if (SC_ADPU->Header.INS == SC_GET_A2R)
      {
        /* Smartcard intialization ------------------------------------------*/
        SC_Init();
        
// USART_DMACmd(SC_USART, USART_DMAReq_Rx, ENABLE);
#if 0        
          for(;;)
          {
            //USART_SendData(SC_USART, 0X55);
            SC_Reset(SET);
            WaitNuS(2);            
            SC_Reset(RESET);
            WaitNuS(4);            
            
            
          }
#endif
          
        /* Reset Data from SC buffer -----------------------------------------*/
        for (i = 0; i < 40; i++)
        {
          SC_ATR_Table[i] = 0;
        }
        
        /* Reset SC_A2R Structure --------------------------------------------*/
        SC_A2R.TS = 0;
        SC_A2R.T0 = 0;
        for (i = 0; i < SETUP_LENGTH; i++)
        {
          SC_A2R.T[i] = 0;
        }
        for (i = 0; i < HIST_LENGTH; i++)
        {
          SC_A2R.H[i] = 0;
        }
        SC_A2R.Tlength = 0;
        SC_A2R.Hlength = 0;
        
        /* Next State --------------------------------------------------------*/
        *SCState = SC_RESET_LOW;
        
        //add 12/11/15 
        SC_Reset(Bit_SET);
      }
    break;

    case SC_RESET_LOW:
      if(SC_ADPU->Header.INS == SC_GET_A2R)
      {
        /* If card is detected then Power ON, Card Reset and wait for an answer) */
       // if (SC_Detect())
        {
          i = 0;
          //while(((*SCState) != SC_POWER_OFF) && ((*SCState) != SC_ACTIVE))
          while(1)
          {
          if(((*SCState) != SC_POWER_OFF) && ((*SCState) != SC_ACTIVE))
          {
            SC_AnswerReq(SCState, &SC_ATR_Table[0], 40); /* Check for answer to reset */
          }
          else
            break;
            if(++i==0xfff)
            {
              (*SCState) = SC_POWER_OFF;
              break;
              
            }
          }
        }
#if 0       
        else
        {
          (*SCState) = SC_POWER_OFF;
        }
#endif       
      }
    break;

    case SC_ACTIVE:
      if (SC_ADPU->Header.INS == SC_GET_A2R)
      {
        if(SC_decode_Answer2reset(&SC_ATR_Table[0]) == T0_PROTOCOL)
        {
          (*SCState) = SC_ACTIVE_ON_T0;
        }
        else
        {
          (*SCState) = SC_POWER_OFF; 
        }
      }
    break;

    case SC_ACTIVE_ON_T0:
      SC_SendData(SC_ADPU, SC_Response);
    break;

    case SC_POWER_OFF:
      //disable 12/10/27
      SC_DeInit(); /* Disable Smartcard interface */
    break;

    default: (*SCState) = SC_POWER_OFF;
  }
}

void SelectSlot(unsigned char slot)
{
  //gbCurSam = slot;
  char i;
  if(slot==0)
  {
    //Y1
    GPIO_SetBits(SC_PIN_3_5V_GPIO, SC_PIN_3_5V_SAM1);
    GPIO_ResetBits(SC_PIN_3_5V_GPIO, SC_PIN_3_5V_SAM2);
    

    

    
  }
  else
  {
    //Y2
    GPIO_ResetBits(SC_PIN_3_5V_GPIO, SC_PIN_3_5V_SAM1);
    GPIO_SetBits(SC_PIN_3_5V_GPIO, SC_PIN_3_5V_SAM2);
    
  }
  
}

/**
  * @brief  Initializes all peripheral used for Smartcard interface.
  * @param  None
  * @retval None
  */
//static void SC_Init(void)
 void SC_Init(void)

{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  //USART_ClockInitTypeDef USART_ClockInitStructure;  
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable GPIO clocks */
  RCC_APB2PeriphClockCmd(SC_PIN_3_5V_GPIO_CLK | SC_PIN_RESET_GPIO_CLK |
                         SC_PIN_CMDVCC_GPIO_CLK | SC_USART_GPIO_CLK |
                         RCC_APB2Periph_AFIO, ENABLE);
                         
  /* Enable USART clock */
  RCC_APB1PeriphClockCmd(SC_USART_CLK, ENABLE);

#if 0
//#ifdef USE_STM3210C_EVAL
  /* Enable the USART3 Pins Software Full Remapping */
  GPIO_PinRemapConfig(GPIO_FullRemap_USART3, ENABLE);
//#endif
#endif
  
  /* Configure USART CK pin as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = SC_USART_PIN_CK;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(SC_USART_GPIO, &GPIO_InitStructure);
  
  /* Configure USART Tx pin as alternate function open-drain */
  GPIO_InitStructure.GPIO_Pin = SC_USART_PIN_TX;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;//12/10/28 add
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_Init(SC_USART_GPIO, &GPIO_InitStructure);

  /* Configure Smartcard Reset pin */
  //GPIO_InitStructure.GPIO_Pin = SC_PIN_RESET_SAM1;
#ifdef CARD_V2
  GPIO_InitStructure.GPIO_Pin = SC_PIN_RESET_SAM1 ;

#else  
  if(gbCurSam==0)
  GPIO_InitStructure.GPIO_Pin = SC_PIN_RESET_SAM1 ;
  //GPIO_InitStructure.GPIO_Pin = SC_PIN_RESET |SC_PIN_RESET_SAM2;
  else
  GPIO_InitStructure.GPIO_Pin = SC_PIN_RESET_SAM2;
#endif
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(SC_PIN_RESET_GPIO, &GPIO_InitStructure);

  /* Configure Smartcard 3/5V pin */
  GPIO_InitStructure.GPIO_Pin = SC_PIN_3_5V_SAM1|SC_PIN_3_5V_SAM2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(SC_PIN_3_5V_GPIO, &GPIO_InitStructure);

  /* Configure Smartcard CMDVCC pin */
  GPIO_InitStructure.GPIO_Pin = SC_PIN_CMDVCC;
  GPIO_Init(SC_PIN_CMDVCC_GPIO, &GPIO_InitStructure);

  /* Enable USART IRQ */
  NVIC_InitStructure.NVIC_IRQChannel = SC_USART_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
  NVIC_Init(&NVIC_InitStructure);
    
/* SC_USART configuration ----------------------------------------------------*/
  /* SC_USART configured as follow:
        - Word Length = 9 Bits
        - 0.5 Stop Bit
        - Even parity
        - BaudRate = 9677 baud
        - Hardware flow control disabled (RTS and CTS signals)
        - Tx and Rx enabled
        - USART Clock enabled
  */

  /* USART Clock set to 3.6 MHz (PCLK1 (36 MHZ) / 10) */
  USART_SetPrescaler(SC_USART, 0x05);
  
  /* USART Guard Time set to 16 Bit */
  //USART_SetGuardTime(SC_USART, 16);
  USART_SetGuardTime(SC_USART, 12);
#if 0  
  USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
  USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
  USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;
  USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
  USART_ClockInit(SC_USART, &USART_ClockInitStructure);
#endif
  
  USART_InitStructure.USART_Clock = USART_Clock_Enable;
  USART_InitStructure.USART_CPOL = USART_CPOL_Low;
  USART_InitStructure.USART_CPHA = USART_CPHA_1Edge;
  USART_InitStructure.USART_LastBit = USART_LastBit_Enable;
  
  USART_InitStructure.USART_BaudRate = 9677;
  USART_InitStructure.USART_WordLength = USART_WordLength_9b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
  USART_InitStructure.USART_Parity = USART_Parity_Even;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(SC_USART, &USART_InitStructure); 

  /* Enable the SC_USART Parity Error Interrupt */
  USART_ITConfig(SC_USART, USART_IT_PE, ENABLE);

  /* Enable the SC_USART Framing Error Interrupt */
  USART_ITConfig(SC_USART, USART_IT_ERR, ENABLE);

  /* Enable SC_USART */
  USART_Cmd(SC_USART, ENABLE);

  /* Enable the NACK Transmission */
  USART_SmartCardNACKCmd(SC_USART, ENABLE);

  /* Enable the Smartcard Interface */
  USART_SmartCardCmd(SC_USART, ENABLE);
  
  /* Set RSTIN HIGH */  
  //SC_Reset(Bit_SET);
  //SC_Reset(Bit_RESET);//modi -> low 12/09/28 
  SC_Reset(Bit_RESET);//modi -> low 12/09/28 
  
 
  /* Select 5V */ 
  //SC_VoltageConfig(SC_VOLTAGE_5V); //disable 12/09/28
#if 0  
  SelectSlot(0); //add 12/09/28
#endif
  
  /* Disable CMDVCC */
  SC_PowerCmd(DISABLE);
 
  
}

/**
  * @brief  Deinitializes all ressources used by the Smartcard interface.
  * @param  None
  * @retval None
  */
static void SC_DeInit(void)
{
  /* Disable CMDVCC */
  //SC_PowerCmd(ENABLE);
  SC_PowerCmd(DISABLE);

  SC_Reset(Bit_RESET);// 12/09/28 
  
  /* Deinitializes the SC_USART */
  USART_DeInit(SC_USART);
  
#if 0
  /* Deinitializes the SC_PIN_3_5V_GPIO */
  GPIO_DeInit(SC_PIN_3_5V_GPIO);

  /* Deinitializes the SC_PIN_RESET_GPIO */
  GPIO_DeInit(SC_PIN_RESET_GPIO);

  /* Deinitializes the SC_PIN_CMDVCC_GPIO */
  GPIO_DeInit(SC_PIN_CMDVCC_GPIO);

  /* Disable GPIO clocks */
  RCC_APB2PeriphClockCmd(SC_PIN_3_5V_GPIO_CLK | SC_PIN_RESET_GPIO_CLK |
                         SC_PIN_CMDVCC_GPIO_CLK | SC_USART_GPIO_CLK |
                         RCC_APB2Periph_AFIO, DISABLE);
                         
  /* Disable SC_USART clock */
  RCC_APB1PeriphClockCmd(SC_USART_CLK, DISABLE);
#endif  
}

/**
  * @brief  Requests the reset answer from card.
  * @param  SCState: pointer to an SC_State enumeration that will contain the Smartcard state.
  * @param  card: pointer to a buffer which will contain the card ATR.
  * @param  length: maximum ATR length
  * @retval None
  */
static void SC_AnswerReq(SC_State *SCState, uint8_t *card, uint8_t length)
{
  uint8_t Data = 0;
  uint32_t i = 0;

  switch(*SCState)
  {
    case SC_RESET_LOW:
      /* Check responce with reset low ---------------------------------------*/
      for (i = 0; i < length; i++)
      {
        if((USART_ByteReceive(&Data, SC_RECEIVE_TIMEOUT)) == SUCCESS)
        {
          card[i] = Data;
        }
      }
      if(card[0])
      {
        (*SCState) = SC_ACTIVE;
        //SC_Reset(Bit_SET);
        //SC_Reset(Bit_RESET); //12/10/28 
        SC_Reset(Bit_SET); //12/10/28 
        //test
        //for(;;);
      }
      else
      {
        (*SCState) = SC_RESET_HIGH;
      }
    break;

    case SC_RESET_HIGH:
      /* Check responce with reset high --------------------------------------*/
      SC_Reset(Bit_SET); /* Reset High */
      //add
      //Lib_DelayMs(10);
      
      while(length--)
      {
        if((USART_ByteReceive(&Data, SC_RECEIVE_TIMEOUT)) == SUCCESS)
        {
          *card++ = Data; /* Receive data for timeout = SC_RECEIVE_TIMEOUT */
        }       
      }
      if(card[0])
      {
        (*SCState) = SC_ACTIVE;
        //12/10/28
        //SC_Reset(Bit_RESET);
        SC_Reset(Bit_SET);
        //test
        //for(;;);
      }
      else
      {
        (*SCState) = SC_POWER_OFF;
      }
    break;

    case SC_ACTIVE:
    break;
    
    case SC_POWER_OFF:
      /* Close Connection if no answer received ------------------------------*/
#if 0 //12/10/28      
      SC_Reset(Bit_SET); /* Reset high - a bit is used as level shifter from 3.3 to 5 V */
      SC_PowerCmd(DISABLE);
#endif      
    break;

    default:
      (*SCState) = SC_RESET_LOW;
  }
}

/**
  * @brief  Decodes the Answer to reset received from card.
  * @param  card: pointer to the buffer containing the card ATR.
  * @retval None
  */
static uint8_t SC_decode_Answer2reset(uint8_t *card)
{
  uint32_t i = 0, flag = 0, buf = 0, protocol = 0;

  SC_A2R.TS = card[0];  /* Initial character */
  SC_A2R.T0 = card[1];  /* Format character */
//test
trace_debug_printf("TS[%02x] T0[%02x]\n",SC_A2R.TS,SC_A2R.T0);

  SC_A2R.Hlength = SC_A2R.T0 & (uint8_t)0x0F;

  if ((SC_A2R.T0 & (uint8_t)0x80) == 0x80)
  {
    flag = 1;
  }

  for (i = 0; i < 4; i++)
  {
    SC_A2R.Tlength = SC_A2R.Tlength + (((SC_A2R.T0 & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
  }

  for (i = 0; i < SC_A2R.Tlength; i++)
  {
    SC_A2R.T[i] = card[i + 2];
  }

  protocol = SC_A2R.T[SC_A2R.Tlength - 1] & (uint8_t)0x0F;

  while (flag)
  {
    if ((SC_A2R.T[SC_A2R.Tlength - 1] & (uint8_t)0x80) == 0x80)
    {
      flag = 1;
    }
    else
    {
      flag = 0;
    }

    buf = SC_A2R.Tlength;
    SC_A2R.Tlength = 0;

    for (i = 0; i < 4; i++)
    {
      SC_A2R.Tlength = SC_A2R.Tlength + (((SC_A2R.T[buf - 1] & (uint8_t)0xF0) >> (4 + i)) & (uint8_t)0x1);
    }
	
    for (i = 0;i < SC_A2R.Tlength; i++)
    {
      SC_A2R.T[buf + i] = card[i + 2 + buf];
    }
    SC_A2R.Tlength += (uint8_t)buf;
  }

  for (i = 0; i < SC_A2R.Hlength; i++)
  {
    SC_A2R.H[i] = card[i + 2 + SC_A2R.Tlength];
  }

  return (uint8_t)protocol;
}

/**
  * @brief  Manages the Smartcard transport layer: send APDU commands and receives
  *   the APDU responce.
  * @param  SC_ADPU: pointer to a SC_ADPU_Commands structure which will be initialized.  
  * @param  SC_Response: pointer to a SC_ADPU_Responce structure which will be initialized.
  * @retval None
  */
static void SC_SendData(SC_ADPU_Commands *SC_ADPU, SC_ADPU_Responce *SC_ResponceStatus)
{
  uint32_t i = 0,nTime;
  uint8_t locData = 0;

  /* Reset responce buffer ---------------------------------------------------*/
  for(i = 0; i < LC_MAX; i++)
  {
    SC_ResponceStatus->Data[i] = 0;
  }
  
  SC_ResponceStatus->SW1 = 0;
  SC_ResponceStatus->SW2 = 0;

  /* Enable the DMA Receive (Set DMAR bit only) to enable interrupt generation
     in case of a framing error FE */  
#if 0 
  USART_DMACmd(SC_USART, USART_DMAReq_Rx, ENABLE);
#endif
  
  /* Send header -------------------------------------------------------------*/
  SCData = SC_ADPU->Header.CLA;
  USART_SendData(SC_USART, SCData);
  i = 0;
  while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
  {
    if(i++==0xffff)
    {
      return;
    }
  }  
  
  SCData = SC_ADPU->Header.INS;
  USART_SendData(SC_USART, SCData);
  i = 0;
  while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
  {
    if(i++==0xffff)
    {
      return;
    }
  }
   
  SCData = SC_ADPU->Header.P1;
  USART_SendData(SC_USART, SCData);
  i = 0;
  while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
  {
    if(i++==0xffff)
    {
      return;
    }
    
  } 
  
  SCData = SC_ADPU->Header.P2;
  USART_SendData(SC_USART, SCData);
  while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
  {
  }   

  /* Send body length to/from SC ---------------------------------------------*/
  if(SC_ADPU->Body.LC)
  {
    SCData = SC_ADPU->Body.LC;
    USART_SendData(SC_USART, SCData);
    while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
    {
    }     
  }
  else if(SC_ADPU->Body.LE)
  { 
    SCData = SC_ADPU->Body.LE;
    USART_SendData(SC_USART, SCData);
    while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
    {
    }     
  }
 
  /* Flush the SC_USART DR */
  (void)USART_ReceiveData(SC_USART);

  
#if 0 
  //test 12/10/25
  i = 0;
  while(1)
  {
    if((USART_ByteReceive(&locData, SC_RECEIVE_TIMEOUT)) == SUCCESS)
      SC_ResponceStatus->Data[i++] =locData ;
  }
#endif
  
  /* --------------------------------------------------------
    Wait Procedure byte from card:
    1 - ACK
    2 - NULL
    3 - SW1; SW2
   -------------------------------------------------------- */

  if((USART_ByteReceive(&locData, SC_RECEIVE_TIMEOUT)) == SUCCESS)
  {
    if(((locData & (uint8_t)0xF0) == 0x60) || ((locData & (uint8_t)0xF0) == 0x90))
    {
      /* SW1 received */
      SC_ResponceStatus->SW1 = locData;

      if((USART_ByteReceive(&locData, SC_RECEIVE_TIMEOUT)) == SUCCESS)
      {
        /* SW2 received */
        SC_ResponceStatus->SW2 = locData;
      }
    }
    else if (((locData & (uint8_t)0xFE) == (((uint8_t)~(SC_ADPU->Header.INS)) & \
             (uint8_t)0xFE))||((locData & (uint8_t)0xFE) == (SC_ADPU->Header.INS & (uint8_t)0xFE)))
    {
      SC_ResponceStatus->Data[0] = locData;/* ACK received */
    }
  }

  /* If no status bytes received ---------------------------------------------*/
  if(SC_ResponceStatus->SW1 == 0x00)
  {
    /* Send body data to SC--------------------------------------------------*/
    if (SC_ADPU->Body.LC)
    {
      for(i = 0; i < SC_ADPU->Body.LC; i++)
      {
        SCData = SC_ADPU->Body.Data[i];
        
        USART_SendData(SC_USART, SCData);
        while(USART_GetFlagStatus(SC_USART, USART_FLAG_TC) == RESET)
        {
        } 
      }
      /* Flush the SC_USART DR */
      (void)USART_ReceiveData(SC_USART);
      /* Disable the DMA Receive (Reset DMAR bit only) */  
      USART_DMACmd(SC_USART, USART_DMAReq_Rx, DISABLE);
    }

    /* Or receive body data from SC ------------------------------------------*/
    else if (SC_ADPU->Body.LE)
    {
      for(i = 0; i < SC_ADPU->Body.LE; i++)
      {
        if(USART_ByteReceive(&locData, SC_RECEIVE_TIMEOUT) == SUCCESS)
        {
          SC_ResponceStatus->Data[i] = locData;
        }
      }
    }
    /* Wait SW1 --------------------------------------------------------------*/
    i = 0;
    while(i < 10)
    {
      if(USART_ByteReceive(&locData, SC_RECEIVE_TIMEOUT) == SUCCESS)
      {
        SC_ResponceStatus->SW1 = locData;
        i = 11;
      }
      else
      {
        i++;
      }
    }
    /* Wait SW2 ------------------------------------------------------------*/   
    i = 0;
    while(i < 10)
    {
      if(USART_ByteReceive(&locData, SC_RECEIVE_TIMEOUT) == SUCCESS)
      {
        SC_ResponceStatus->SW2 = locData;
        i = 11;
      }
      else
      {
        i++;
      }
    }
  }
}


//test
u8 smartcardtest(void)
{
  unsigned char i;
  SC_ADPU_Commands SC_ADPU;
  SC_ADPU_Responce SC_Responce;
  u8 MasterRoot[2] = {0x3F, 0x00};
  u8 PayRoot[15] ;

  memcpy(PayRoot,"1PAY.SYS.DDF01", 14);
  
 /* Setup SysTick Timer for 1 msec interrupts  */
  //SysTick_Config(SystemFrequency / 1000);
  
  SC_State SCState = SC_POWER_OFF;
  while(1)
  {
    
    //add 12/09/28
    /* Smartcard detected */
    //CardInserted = 1;
    /* Power ON the card */
    SC_PowerCmd(ENABLE);
    /* Reset the card */
    SC_Reset(Bit_RESET);
    //SC_Reset(Bit_SET); //high
    
    SelectSlot(0); //12/10/29
    
    /* Wait A2R --------------------------------------------------------------*/
    SCState = SC_POWER_ON;

    SC_ADPU.Header.CLA = 0x00;
    SC_ADPU.Header.INS = SC_GET_A2R;
    SC_ADPU.Header.P1 = 0x00;
    SC_ADPU.Header.P2 = 0x00;
    SC_ADPU.Body.LC = 0x00;
   
    while(SCState != SC_ACTIVE_ON_T0) 
    {
      SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    }
    
    trace_debug_printf("ATR:");
    for(i=0;i<20;i++)
      debug_printf(0,0,0,"%02x ",SC_ATR_Table[i]);
    debug_printf(0,0,0,"\n");
    
    
//test 12/10/28
  //SC_Reset(Bit_RESET);//modi -> low 12/09/28
#if 0  
  /* enable CMDVCC */
  SC_PowerCmd(ENABLE);
    trace_debug_printf("VCC:%02x\n",GPIO_ReadOutputDataBit(SC_PIN_CMDVCC_GPIO, SC_PIN_CMDVCC));       
#endif
    Lib_DelayMs(200);
    
    SC_ADPU.Header.CLA = 0x00;
    SC_ADPU.Header.INS = SC_SELECT_FILE;
    SC_ADPU.Header.P1 = 0x04;
    //SC_ADPU.Header.P1 = 0x00;
    SC_ADPU.Header.P2 = 0x00;
    //SC_ADPU.Body.LC = 0x02;
    SC_ADPU.Body.LC = 0x14;

    for(i = 0; i < SC_ADPU.Body.LC; i++)
    {
      
      SC_ADPU.Body.Data[i] = MasterRoot[i];
      //SC_ADPU.Body.Data[i] = PayRoot[i];
    }
    while(i < LC_MAX) 
    {    
      SC_ADPU.Body.Data[i++] = 0;
    }
    SC_ADPU.Body.LE = 0;

    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);

    trace_debug_printf("command:SW1[%02x] SW[%02x] \n",SC_Responce.SW1,SC_Responce.SW2);
    trace_debug_printf("reset:%02x\n",GPIO_ReadOutputDataBit(SC_PIN_RESET_GPIO,SC_PIN_RESET_SAM1));
    trace_debug_printf("VCC:%02x\n",GPIO_ReadOutputDataBit(SC_PIN_CMDVCC_GPIO, SC_PIN_CMDVCC));       
#if 0    
              for(;;)
          {
            USART_SendData(SC_USART, 0X55);
          }
#endif
    //test
    for(;;);
    
    /* Disable the Smartcard interface */
    SCState = SC_POWER_OFF;
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    //CardInserted = 0;    
  }
  
}

/**
  * @brief  Configures the card power voltage.
  * @param  SC_Voltage: specifies the card power voltage.
  *   This parameter can be one of the following values:
  *     @arg SC_VOLTAGE_5V: 5V cards.
  *     @arg SC_VOLTAGE_3V: 3V cards.
  * @retval None
  */
static void SC_VoltageConfig(uint32_t SC_Voltage)
{
  if(SC_Voltage == SC_VOLTAGE_5V)
  {
    /* Select Smartcard 5V */  
    GPIO_SetBits(SC_PIN_3_5V_GPIO, SC_PIN_3_5V_SAM1);
  }
  else
  {
    /* Select Smartcard 3V */      
    GPIO_ResetBits(SC_PIN_3_5V_GPIO, SC_PIN_3_5V_SAM1);
  } 
}


/**
  * @brief  Receives a new data while the time out not elapsed.
  * @param  None
  * @retval An ErrorStatus enumuration value:
  *          - SUCCESS: New data has been received
  *          - ERROR: time out was elapsed and no further data is received
  */
static ErrorStatus USART_ByteReceive(uint8_t *Data, uint32_t TimeOut)
{
  uint32_t Counter = 0;

  while((USART_GetFlagStatus(SC_USART, USART_FLAG_RXNE) == RESET) && (Counter != TimeOut))
  {
    Counter++;
  }

  if(Counter != TimeOut)
  {
    *Data = (uint8_t)USART_ReceiveData(SC_USART);
    return SUCCESS;    
  }
  else 
  {
    return ERROR;
  }
}


/**
  * @brief  Enables or disables the power to the Smartcard.
  * @param  NewState: new state of the Smartcard power supply. 
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void SC_PowerCmd(FunctionalState NewState)
{
  if(NewState != DISABLE)
  {
#ifdef CARD_V2
    GPIO_ResetBits(SC_PIN_CMDVCC_GPIO, SC_PIN_CMDVCC);
#else  
    GPIO_SetBits(SC_PIN_CMDVCC_GPIO, SC_PIN_CMDVCC);
#endif    
  }
  else
  {
#ifdef CARD_V2
    GPIO_SetBits(SC_PIN_CMDVCC_GPIO, SC_PIN_CMDVCC);
#else    
    GPIO_ResetBits(SC_PIN_CMDVCC_GPIO, SC_PIN_CMDVCC);
#endif    
  } 
}

/**
  * @brief  Sets or clears the Smartcard reset pin.
  * @param  ResetState: this parameter specifies the state of the Smartcard 
  *   reset pin. BitVal must be one of the BitAction enum values:
  *     @arg Bit_RESET: to clear the port pin.
  *     @arg Bit_SET: to set the port pin.
  * @retval None
  */
void SC_Reset(BitAction ResetState)
{
  //GPIO_WriteBit(SC_PIN_RESET_GPIO, SC_PIN_RESET_SAM1, ResetState);
    BitAction bState;
    //取反
    if(ResetState)
      bState = Bit_RESET;
    else
      bState = Bit_SET;

#if 0
  if(ResetState==0)
  {
    
    GPIO_ResetBits(SC_PIN_RESET_GPIO,SC_PIN_RESET);
    
  }
  else
    GPIO_SetBits(SC_PIN_RESET_GPIO,SC_PIN_RESET);
#endif
#ifdef CARD_V2
  //取反
  //GPIO_WriteBit(SC_PIN_RESET_GPIO, SC_PIN_RESET_SAM1, ResetState);
  GPIO_WriteBit(SC_PIN_RESET_GPIO, SC_PIN_RESET_SAM1, bState);
#else  
  if(gbCurSam==0)
  GPIO_WriteBit(SC_PIN_RESET_GPIO, SC_PIN_RESET_SAM1, ResetState);
  else
  GPIO_WriteBit(SC_PIN_RESET_GPIO, SC_PIN_RESET_SAM2, ResetState);
#endif      
}

#endif
