#include "stm32f10x_it.h"
#include "Toolkit.h"
#include "stm32f10x_spi.h"
#include "RF1356.h"
#include "ISO14443.h"
#include "phcsBflHw1Reg.h"
#include "phcsBflStatus.h"

//#define SPI1_NSS_LOW()     GPIO_ResetBits(GPIOB, GPIO_Pin_0)
//#define SPI1_NSS_HIGH()    GPIO_SetBits(GPIOB, GPIO_Pin_0)

//FOR KF311

//#define SPI1      SPI2

#define SPI1_NSS_LOW()     GPIO_ResetBits(GPIOA, GPIO_Pin_4)
#define SPI1_NSS_HIGH()    GPIO_SetBits(GPIOA, GPIO_Pin_4)

extern u8 PN512RegsWrite(u8 *pData);
extern u8 PN512RegsRead(u8 *pData);

//ADD BY SKX
void PN512_RFSwitchNCS(char chHigh)
{
  if(chHigh)
    SPI1_NSS_HIGH();
  else
    SPI1_NSS_LOW();
}


/* FOR KF322
void RFinterrupt_Init(void)
{
  
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  

    //Connect EXTI Line8 to Pa8 
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);

    //Configure EXTI Line8 to generate an interrupt on falling edge
    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}
*/

//FOR KF311
void RFinterrupt_Init(void)
{
  
    GPIO_InitTypeDef GPIO_InitStructure;

    //GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  

    //Connect EXTI Line8 to PB0 
    //GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

    //Configure EXTI Line8 to generate an interrupt on falling edge
    EXTI_InitTypeDef EXTI_InitStructure;

    //EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}


/* FOR KF322
void SPI1_Init(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  //Enable SPI1 clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  //Configure SPI1 pins: NSS, SCK, MISO and MOSI
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //Configure PB.0 as Output push-pull, used as Flash Chip select
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  //SPI1 Config
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI1_NSS_HIGH();  
  //SPI1 enable
  SPI_Cmd(SPI1, ENABLE);
}
*/

//FOR KF311
void SPI1_Init(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  //Enable SPI1 clock
  //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  //Configure SPI1 pins: NSS, SCK, MISO and MOSI
  /*
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //Configure PA4 as Output push-pull, used as Flash Chip select
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //SPI1 Config
  SPI_Cmd(SPI1, DISABLE);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI1_NSS_HIGH();  
  //SPI1 enable
  SPI_Cmd(SPI1, ENABLE);
}



//AT skx
//volatile unsigned char rf_get_value=0;
u8 SPIDataSendReceive(u8 DATA)
//vu8 SPIDataSendReceive(u8 DATA)
{
    /* Loop while DR register in not emplty */
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);
    /* Send byte through the SPI1 peripheral */
    SPI_SendData(SPI1, DATA);
    /* Wait to receive a byte */
    while(SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);
    /* Return the byte read from the SPI bus */
    return SPI_ReceiveData(SPI1);
}

void PN512SetReg(u8 RegAddr,u8 RegValue)
{
  SPI1_NSS_LOW();
  SPIDataSendReceive((RegAddr <<1)&0x7F);
  SPIDataSendReceive(RegValue);
  //AT SKX
  //WaitNuS(1);
  SPI1_NSS_HIGH();   
}

//AT skx
u8 PN512GetReg(u8  RegAddr)
//vu8 PN512GetReg(u8  RegAddr)
{
  //AT skx
  u8 RegValue;
  
  SPI1_NSS_LOW();  
  SPIDataSendReceive((RegAddr <<1)|0x80);
  //AT skx
  RegValue = SPIDataSendReceive(0x00);
  //AT SKX
  //WaitNuS(1);
  SPI1_NSS_HIGH();  
  return RegValue;
}
