/// @file   UARTSC.h
/// This is for module @ref SMART_CARD
/// @author Travis Tu
/// @date   2006-Sep-1st

#ifndef _UARTSC_H_
#define _UARTSC_H_

/**
 * @addtogroup Platform_Device_Library
 * @{
 */

/**
 * @addtogroup SMART_CARD
 * @{
 */

/**
 * @defgroup SMART_CARD_DEFINATION
 * @ingroup SMART_CARD
 * @{
 */
//#define UART_FIFOEnableBit      10      ///<CR regiter bit definition
//#define UART_SMARTCARDEnableBit  9      ///<CR regiter bit definition
//#define UART_RxEnableBit         8      ///<CR regiter bit definition
//#define UART_RunBit              7      ///<CR regiter bit definition
//#define UART_LoopBackBit         6      ///<CR regiter bit definition
//#define UART_ParityOddBit        5      ///<CR regiter bit definition
//#define UART_StopBits            3      ///<CR regiter bit definition
//
//#define UART_05StopBits     0x00        ///<Stop bits definition
//#define UART_1StopBit       (0x01<<3)   ///<Stop bits definition
//#define UART_15StopBits     (0x02<<3)   ///<Stop bits definition
//#define UART_2StopBits      (0x03<<3)   ///<Stop bits definition
//
//#define UART_8BitsData       0x01       ///<Modes definition
//#define UART_7BitsData       0x03       ///<Modes definition
//#define UART_9BitsData       0x04       ///<Modes definition
//#define UART_8BitsDataWakeUp 0x05       ///<Modes definition
//#define UART_8BitsDataParity 0x07       ///<Modes definition
//
//#define SC_DATA         10              ///<PIN Number: SC data in or out
//#define SC_CLK          12              ///<PIN Number: SC CLK for both IC Card
//#define SC_RST          3               ///<PIN Number: BIG CARD or small card reset P1.3
//#define SC_POWER_SMALL  7               ///<PIN Number: P1.7
//#define SC_POWER_BIG    8               ///<PIN Number: P1.8
//#define SC_SEL          4               ///<PIN Number: SMart Card Select PIN
//#define SC_DET_VCC_BIG    0             ///<PIN Number: P1.0 used as detector on  Big   Card VCC to test whether VCC and GND are short cut
//#define SC_DET_VCC_SMALL  2             ///<PIN Number: P1.3 used as detector on  SMart Card VCC to test whether VCC and GND are short cut
//
//#define SC_DATA_MASK            (1<<SC_DATA)        ///<PIN mASK: SC data in or out
//#define SC_CLK_MASK             (1<<SC_CLK)         ///<PIN mASK: SC CLK for both IC Card
//#define SC_RST_MASK             (1<<SC_RST)         ///<PIN mASK: SMALL CARD or big card reset
//#define SC_POWER_SMALL_MASK     (1<<SC_POWER_SMALL) ///<PIN mASK: P1.7
//#define SC_POWER_BIG_MASK       (1<<SC_POWER_BIG)   ///<PIN mASK: P1.8
//#define SC_SEL_MASK             (1<<SC_SEL)         ///<PIN mASK: SMart Card Select PIN
//#define SC_DET_VCC_BIG_MASK     (1<<SC_DET_VCC_BIG) ///<PIN mASK: P1.0
//#define SC_DET_VCC_SMALL_MASK   (1<<SC_DET_VCC_SMALL)///<PIN mASK: P1.3
//
///*driver information*/
//#define SC_BASE         (APB1_BASE + 0x5040)        ///<driver information
//#define SC_CLK_VALUE    3000000                     ///<Smart Card Frequence    3000000MHz

#define SC_SW_TIMEOUT_IDLE 0x0001                   ///<if removed unexpectedly or just no response sw will be set to this value
#define SC_SW_UNKNOW_PROCEDURE_BYTE 0x0002          ///<if unrecognized procedure byte received, sw will be set to this value
/**@}*/

#define SC_CLK_VAL                  4500000    //4 MHz
//#define SC_TIMEOUT_VAL              (1000000+((372*1000000)/SC_CLK_VAL)*12*256)    //1 Second + 256bytes' transfer time.

/* Smartcard Inteface GPIO pins */
#define SC_BIG_PWR_PIN              GPIO_Pin_4 /* GPIOc Pin 4 */
#define SC_BIG_RESET_PIN            GPIO_Pin_5 /* GPIOc Pin 5 */
#define SC_BIG_IO_PIN               GPIO_Pin_2 /* GPIOa Pin 2 */
#define SC_BIG_CLK_PIN              GPIO_Pin_4 /* GPIOa Pin 4 */

#define SC_BIG_PWR_GPIO             GPIOC /* GPIOc Pin 4 */
#define SC_BIG_RESET_GPIO           GPIOC /* GPIOc Pin 5 */
#define SC_BIG_IO_GPIO              GPIOA /* GPIOa Pin 2 */
#define SC_BIG_CLK_GPIO             GPIOA /* GPIOa Pin 4 */

#define SC_SML_PWR_PIN              GPIO_Pin_8 /* GPIOC Pin 8 */
#define SC_SML_RESET_PIN            GPIO_Pin_9 /* GPIOC Pin 9 */
#define SC_SML_IO_PIN               GPIO_Pin_9 /* GPIOA Pin 9 */
#define SC_SML_CLK_PIN              GPIO_Pin_8 /* GPIOA Pin 8 */

#define SC_SML_PWR_GPIO             GPIOC /* GPIOC Pin 8 */
#define SC_SML_RESET_GPIO           GPIOC /* GPIOC Pin 9 */
#define SC_SML_IO_GPIO              GPIOA /* GPIOA Pin 9 */
#define SC_SML_CLK_GPIO             GPIOA /* GPIOA Pin 8 */

#define SC_BIG_UART                 USART2
#define SC_SML_UART                 USART1

/**
 * @defgroup SMART_CARD_TYPES
 * @ingroup SMART_CARD
 * @{
 */

///Smart Card ID
typedef enum
{
    SMALL_SC = 0,   ///<ID for Small Card
    BIG_SC   = 1    ///<ID for BIg Card
}SC_ID;

/////Smart Card
//typedef enum
//{
//    CLK_DIV2   = 0x01, ///<divides the source clock frequency by 2
//    CLK_DIV4   = 0x02  ///<divides the source clock frequency by 4
//} SC_CLKVal;

/////SC register Map
//typedef volatile struct
//{
//    vu16 CLKVAL;  ///< internal register
//    vu16 EMPTY;   ///< internal register
//    vu16 CLKCON;  ///< internal register
//} SC_TypeDef;

///ATR recorder
typedef struct
{
    struct
    {
        u8 TS;
        u8 T0;
        bool bTA1;  ///<whether TA1 is present
        u8 TA1;
        bool bTB1;  ///<whether TB1 is present
        u8 TB1;
        bool bTC1;  ///<whether TC1 is present
        u8 TC1;
        bool bTD1;  ///<whether TD1 is present
        u8 TD1;
        bool bTA2;  ///<whether TA2 is present
        u8 TA2;
        bool bTB2;  ///<whether TB2 is present
        u8 TB2;
        bool bTC2;  ///<whether TC2 is present
        u8 TC2;
        bool bTD2;
        u8 TD2;
        bool bTA3;
        u8 TA3;
        bool bTB3;
        u8 TB3;
        bool bTC3;
        u8 TC3;
        bool bTD3;
        u8 TD3;
    }ATR;
    u8 buf[33];     ///<33 BYTES to contain ATR string
    u8 length;      ///<length of ATR
    bool bTCKNOShow;
    bool bTCKError;
}SCART_TypeDef;

///APDU structure
typedef struct
{
    u8 CLS;
    u8 INS;
    u8 P1;
    u8 P2;
    u8 Lc;
    u8 Data[255];
}APDUCmd_TypeDef;
/**@}*/
/**@}*/

extern SCART_TypeDef SCATR;
extern u32  SC_BaudRate_SMALL;
extern u32  SC_BaudRate_BIG;

/**
 * @addtogroup SMART_CARD
 * @{
 */

/**
 * @defgroup SMART_CARD_LOW_INTERFACE
 * @ingroup SMART_CARD
 * @{
 */
//inline void SC_CLKConfig(SC_CLKVal val) {SC->CLKVAL = (SC->CLKVAL&0xFFE0)|(u16)val;}        ///<contral the Smart Card Clock
//inline void SC_CLKEnable(void)          {if ((SC->CLKCON&0x01)!=0x01) SC->CLKCON|=0x01;}    ///<Enable the Clock
//inline void SC_CLKDisable(void)         {SC->CLKCON&=~0x01;}                                ///<Disable the Clock
//inline void SC_UART1_ON(void)           {UART1->CR|=0x0200;}                                ///<enable the Smart Card function on UART1
//inline void SC_UART1_OFF(void)          {UART1->CR&=(~0x0200);}                             ///<disable the Smart Card function on UART1
//inline void SC_UART1_Clear_Status(void) {UART1->IER=0x00;(void)UART1->RxBUFR;}///<clear the status on UART1 for SMart Card
//inline void SC_UART1_reset(void)        {UART_FifoReset(UART1, UART_RxFIFO);UART_FifoReset(UART1, UART_TxFIFO);SC_UART1_Clear_Status();}///<reset the Smart Card communication channel
void SC_Setup_BIG(void);                                     ///<setup UART configration for Smart Card connection
void SC_Setup_SMALL(void);                                     ///<setup UART configration for Smart Card connection
void SC_Turnoff_BIG(void);
void SC_Turnoff_SMALL(void);
void SC_SendBytes(const u8* byte,u16 length);           ///<send bytes array with length indicated
bool  SC_ReceiveBytes(u8* data, u16 length,u32 timeout);    ///<receive bytes
void SC_ParityErrorHandler(USART_TypeDef * UARTx);       ///<interrupt service program
/**@}*/


/**
 * @defgroup SMART_CARD_HIGH_INTERFACE
 * @ingroup SMART_CARD
 * @{
 */
SCART_TypeDef*   SC_PowerOn(SC_ID id);       ///<Power On big card or small one
void             SC_PowerOff(SC_ID id);      ///<Power Off big card or small one
SCART_TypeDef*   SC_Cold_Reset(SC_ID id);    ///<Cold reset the big or small card
SCART_TypeDef*   SC_Warm_Reset(SC_ID id);    ///<Worm reset the big or small card
void SC_Powerconnect();//yyw
void SC_PowerCut();//yyw
u8            SC_SendAPDU(SC_ID id, const u8* apdu, u16 apduLength, u16* sw, u8* rsp); ///<send a APDU Buffer assum the apdu buffer is right
u8 SC_SendAPDU_KaiFa(u8 *ptr, SC_ID id);
/**@}*/

/**@}*/

/**@}*/
#endif



