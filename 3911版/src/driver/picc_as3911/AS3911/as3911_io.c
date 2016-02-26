/*
 *****************************************************************************
 * Copyright @ 2009 by austriamicrosystems AG                                *
 * All rights are reserved.                                                  *
 *                                                                           *
 * Reproduction in whole or in part is prohibited without the written consent*
 * of the copyright owner. Austriamicrosystems reserves the right to make    *
 * changes without notice at any time. The software is provided as is and    *
 * Austriamicrosystems makes no warranty, expressed, implied or statutory,   *
 * including but not limited to any implied warranty of merchantability or   *
 * fitness for any particular purpose, or that the use will not infringe any *
 * third party patent, copyright or trademark. Austriamicrosystems should    *
 * not be liable for any loss or damage arising from its use.                *
 *****************************************************************************
 */

/*
 * PROJECT: AS3911 firmware
 * $Revision: $
 * LANGUAGE: ANSI C
 */

/*! \file as3911_io.c
 *
 * \author Oliver Regenfelder
 *
 * \brief AS3911 SPI communication.
 *
 * Implementation of the AS3911 SPI communication. The PIC is set to IPL 7 to disable
 * interrupts while accessing the SPI.
 */

/*
******************************************************************************
* INCLUDES
******************************************************************************
*/
//#include <p24FJ64GB002.h>
#include "stm32f10x_gpio.h"
#include "as3911_io.h"
#include "as3911_def.h"
#include "as3911_irq.h"

//#include "spi_driver.h"
//13/08/07
#define s_UartPrint trace_debug_printf

/*
******************************************************************************
* DEFINES
******************************************************************************
*/

#define AS3911_SPI_ADDRESS_MASK         (0x3F)
#define AS3911_SPI_CMD_READ_REGISTER    (0x40)
#define AS3911_SPI_CMD_WRITE_REGISTER   (0x00)
#define AS3911_SPI_CMD_READ_FIFO        (0xBF)
#define AS3911_SPI_CMD_WRITE_FIFO       (0x80)
#define AS3911_SPI_CMD_DIREC_CMD        (0xC0)

/*
******************************************************************************
* MACROS
******************************************************************************
*/

#define RF_CS_IONUM  GPIO_Pin_4
#define RF_CLK_IONUM GPIO_Pin_5
#define RF_MOSI_IONUM GPIO_Pin_7
#define RF_MISO_IONUM  GPIO_Pin_6
#define RF_IRQ_IONUM GPIO_Pin_1
	
#define RF_CS_IOTYPE GPIOA  //GPIOG->GPIOB 13/08/07
#define RF_CLK_IOTYPE GPIOA
#define RF_MOSI_IOTYPE GPIOA
#define RF_MISO_IOTYPE GPIOA
#define RF_IRQ_IOTYPE GPIOA

//#define AS3911_SEN_ON() { _LATB8 = 0; }
//#define AS3911_SEN_OFF() { _LATB8 = 1; }

#define AS3911_SEN_ON() GPIO_ResetBits(RF_CS_IOTYPE,RF_CS_IONUM)
#define AS3911_SEN_OFF() GPIO_SetBits(RF_CS_IOTYPE,RF_CS_IONUM)
//#define SPI_I2S_GetFlagStatus
//#define SPI_I2S_SendData
//#define SPI_I2S_ReceiveData
#define SET_AND_SAVE_CPU_IPL(a,b) 
#define  SET_CPU_IPL(a) 

/*
******************************************************************************
* LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL TABLES
******************************************************************************
*/

/*
******************************************************************************
* LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
* GLOBAL VARIABLE DEFINITIONS
******************************************************************************
*/
bool as3911Runing = FALSE;
/*
******************************************************************************
* GLOBAL FUNCTIONS
******************************************************************************
*/
 static int soc_spi_read(SPI_TypeDef *spiid, unsigned char *buf, int count)
 {
 //for old FWLib
 #if  1
	 int i;

	 do {
		 
		 for (i = 0; i < count; i++) {
		 	 while(SPI_GetFlagStatus(spiid, SPI_FLAG_TXE) == RESET);
			 SPI_SendData(spiid, 0xff);
			 while(SPI_GetFlagStatus(spiid, SPI_FLAG_RXNE) == RESET);
			 buf[i] = (u8) SPI_ReceiveData(spiid);
		 }
	 } while (0);

 #else
	 int i;
	 ulong after_tick;
 
	 do {
		 
		 for (i = 0; i < count; i++) {
		 	 while((SPI_I2S_GetFlagStatus(spiid, SPI_I2S_FLAG_TXE) == RESET) );
			 SPI_I2S_SendData(spiid, 0xff);
			 while((SPI_I2S_GetFlagStatus(spiid, SPI_I2S_FLAG_RXNE) == RESET) );
			 buf[i] = (u8) SPI_I2S_ReceiveData(spiid);
		 }
	 } while (0);
 #endif
         
	 return 0;
 }
 
 static int soc_spi_write(SPI_TypeDef *spiid,const u8 *buf, int count)
 {
 //for old FWLib
#if 1
	 int i;
	 u8 unused;

	 do {
	 	
		 for (i = 0; i < count; i++) {
		 	 while(SPI_GetFlagStatus(spiid, SPI_FLAG_TXE) == RESET);
			 SPI_SendData(spiid, buf[i]);
			 while(SPI_GetFlagStatus(spiid, SPI_FLAG_RXNE) == RESET);
			 unused = (u8) SPI_ReceiveData(spiid);
		 }
	 } while (0);

#else
	 int i;
	 u8 unused;
     ulong after_tick;
	 do {
	 	
		 for (i = 0; i < count; i++) {
		 	 while((SPI_I2S_GetFlagStatus(spiid, SPI_I2S_FLAG_TXE) == RESET) ); 
			 SPI_I2S_SendData(spiid, buf[i]);
			 while((SPI_I2S_GetFlagStatus(spiid, SPI_I2S_FLAG_RXNE) == RESET));
			 unused = (u8) SPI_I2S_ReceiveData(spiid);
		 }
	 } while (0);
#endif
         
	 return 0;
 }

 s8 spiRxTx(u16 numberOfBytesToTx, const u8 * txData, u16 numberOfBytesToRx, u8 * rxData, bool sync)
 {
       
	soc_spi_write(SPI1, txData, numberOfBytesToTx);
	soc_spi_read(SPI1, rxData, numberOfBytesToRx);
	
        return 0;
 }



 
 
/* 此函数必须在picc_open 之前调用 */
static bool as3911IsRunning(void)
{
	
	u8 mode = 0x10;	/* ISO14443B mode */
	u8 temp = 0;
	u8 i = 0;
        
    for (i=0; i<10; i++) {
                
        as3911WriteRegister(AS3911_REG_MODE, mode);
        as3911ReadRegister(AS3911_REG_MODE, &temp);
        s_UartPrint("AS3911_REG_MODE = 0x%x\r\n", temp); //sxl
        //delay_ms(200);
        if (temp == mode)
        {
            for (i=0; i<10; i++){
                as3911ExecuteCommand(AS3911_CMD_SET_DEFAULT); 
                as3911ReadRegister(AS3911_REG_MODE, &temp);
                s_UartPrint("after set default, AS3911_REG_MODE = 0x%x\r\n", temp);
                //delay_ms(200);
                if (temp == 0x08)  /* after set defualt, mode is ISO14443A, 0x08 */
                {
                    return TRUE;
                }
            }
        } 
    }

    return FALSE;
}
 
 void initAS3911()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef spi;
	EXTI_InitTypeDef EXTI_InitStructure ;
	
	//AFIO->MAPR = ((AFIO->MAPR & 0xF8FFFFFF) | 0x02000000);
///////////////////////////////////////////////	  INT init

	//AT enable spi2 clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
   
    //GPIO_PinRemapConfig(GPIO_Remap_SPI1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin	  = RF_IRQ_IONUM;  
    GPIO_InitStructure.GPIO_Mode	  = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(RF_IRQ_IOTYPE, &GPIO_InitStructure);
	
	EXTI_ClearITPendingBit(AS3911_EINT_LINE) ;   //清除中断标志位
    GPIO_EXTILineConfig(AS3911_IRQ_SOURCETYPE , AS3911_IRQ_SOURCENUM);
	EXTI_InitStructure.EXTI_Line = AS3911_EINT_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure) ;
    
 
///////////////////////////////////////////////////	SPI IO init
        
	GPIO_InitStructure.GPIO_Pin 	= RF_CS_IONUM;  
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(RF_CS_IOTYPE, &GPIO_InitStructure);
        
       
	GPIO_InitStructure.GPIO_Pin			= RF_CLK_IONUM;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
	GPIO_Init(RF_CLK_IOTYPE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin			= RF_MOSI_IONUM;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
	GPIO_Init(RF_MOSI_IOTYPE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin			= RF_MISO_IONUM;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
       
	GPIO_Init(RF_MISO_IOTYPE, &GPIO_InitStructure);
//////////////////////////////////////////////////////////////SPI REG init	
    SPI_Cmd(SPI1, DISABLE);
	spi.SPI_Direction		= SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode			= SPI_Mode_Master;
	spi.SPI_DataSize		= SPI_DataSize_8b;
	spi.SPI_CPOL			= SPI_CPOL_Low;
	spi.SPI_CPHA			= SPI_CPHA_2Edge;
	spi.SPI_NSS			= SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler	= SPI_BaudRatePrescaler_32;
	spi.SPI_FirstBit		= SPI_FirstBit_MSB;
	spi.SPI_CRCPolynomial		= 0x0007;

	SPI_Init(SPI1, &spi);
	
	//SPI_SSOutputCmd(SPI2, ENABLE);
	//SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);
	GPIO_SetBits(RF_CS_IOTYPE,RF_CS_IONUM);
       
       
	SPI_Cmd(SPI1, ENABLE);
        
	if (as3911IsRunning())
	{
		as3911Runing = TRUE;
	}
	else
	{	s_UartPrint("AS3911 is not running\r\n");
		as3911Runing = FALSE;
	}
	
///////////////////////////////////////////////////////////////////
	//return 0;
}


s8 as3911WriteRegister(u8 address, u8 data)
{
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911WriteCommand[2] = { address & AS3911_SPI_ADDRESS_MASK, data };

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(2, &as3911WriteCommand[0], 0, NULL, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);

    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911ReadRegister(u8 address, u8 *data)
{
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911ReadCommand = AS3911_SPI_CMD_READ_REGISTER | (address & AS3911_SPI_ADDRESS_MASK);

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(1, &as3911ReadCommand, 1, data, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);

    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911WriteTestRegister(u8 address, u8 data)
{
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911WriteCommand[3] = { AS3911_SPI_CMD_DIREC_CMD | AS3911_CMD_TEST_ACCESS, address & AS3911_SPI_ADDRESS_MASK, data };

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(3, &as3911WriteCommand[0], 0, NULL, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);

    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911ReadTestRegister(u8 address, u8 *data)
{
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911ReadCommand[2] = { AS3911_SPI_CMD_DIREC_CMD | AS3911_CMD_TEST_ACCESS, AS3911_SPI_CMD_READ_REGISTER | (address & AS3911_SPI_ADDRESS_MASK) };

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(2, &as3911ReadCommand[0], 1, data, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);

    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911ModifyRegister(u8 address, u8 mask, u8 data)
{
    s8 error = ERR_NONE;
    u8 registerValue = 0;

    error |= as3911ReadRegister(address, &registerValue);
    registerValue = (registerValue & ~mask) | data;
    error |= as3911WriteRegister(address, registerValue);

    if (ERR_NONE == error)
        return ERR_NONE;
    else
        return ERR_IO;
}

s8 as3911ContinuousWrite(u8 address, const u8 *data, u8 length)
{
    AS3911_IRQ_OFF();//sxl 14/07/14
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    if (length == 0)
        return ERR_NONE;

    u8 as3911WriteCommand = AS3911_SPI_CMD_WRITE_REGISTER | (address & AS3911_SPI_ADDRESS_MASK);
    AS3911_SEN_ON();
    error |= spiRxTx(1, &as3911WriteCommand, 0, NULL, FALSE);
    error |= spiRxTx(length, data, 0, NULL, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);
    AS3911_IRQ_ON();//sxl 14/07/14
    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911ContinuousRead(u8 address, u8 *data, u8 length)
{
  AS3911_IRQ_OFF();//sxl 14/07/14
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911ReadCommand = AS3911_SPI_CMD_READ_REGISTER | (address & AS3911_SPI_ADDRESS_MASK);
    
    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);
  
    AS3911_SEN_ON();
    error |= spiRxTx(1, &as3911ReadCommand, length, data, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl); 
    AS3911_IRQ_ON();  //sxl 14/07/14
    if (ERR_NONE != error)
        return ERR_IO;
    else
      
        return ERR_NONE;
}

s8 as3911WriteFifo(const u8 *data, u8 length)
{
  AS3911_IRQ_OFF();  //sxl 14/07/14
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911WriteFifoCommand = AS3911_SPI_CMD_WRITE_FIFO;

    if (0 == length)
        return ERR_NONE;

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(1, &as3911WriteFifoCommand, 0, NULL, TRUE);
    error |= spiRxTx(length, data, 0, NULL, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);
    AS3911_IRQ_ON();  //sxl 14/07/14
    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911ReadFifo(u8 *data, u8 length)
{
    AS3911_IRQ_OFF();  //sxl 14/07/14
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911ReadFifoCommand = AS3911_SPI_CMD_READ_FIFO;

    if (length == 0)
        return ERR_NONE;

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(1, &as3911ReadFifoCommand, length, data, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);
    AS3911_IRQ_ON();  //sxl 14/07/14
    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

s8 as3911ExecuteCommand(u8 directCommand)
{
    s8 error = ERR_NONE;
    int current_cpu_ipl = 0;
    u8 as3911DirectCommand = AS3911_SPI_CMD_DIREC_CMD | (directCommand & AS3911_SPI_ADDRESS_MASK);

    SET_AND_SAVE_CPU_IPL(current_cpu_ipl, 7);

    AS3911_SEN_ON();
    error |= spiRxTx(1, &as3911DirectCommand, 0, NULL, TRUE);
    AS3911_SEN_OFF();

    SET_CPU_IPL(current_cpu_ipl);

    if (ERR_NONE != error)
        return ERR_IO;
    else
        return ERR_NONE;
}

/*
******************************************************************************
* LOCAL FUNCTIONS
******************************************************************************
*/
