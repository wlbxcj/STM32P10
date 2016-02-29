

#ifndef _RF1356_H_
#define _RF1356_H_

#include "stm32f10x_lib.h"
#include "phcsBflHw1Reg.h"

void SPI1_Init(void);
void RFinterrupt_Init(void);
void RF1356_Init(void);
//AT skx
u8 SPIDataSendReceive(u8 DATA);
//vu8 SPIDataSendReceive(u8 DATA);
u8  PN512TimerOut(u32 DelayUs);
u8  PN512ActivateRf(void);
u8 PN512SetBitRate(void);
u8 ExeISO144433ACommand(u8 PiccCmd);
u8 PN512WriteFifo(u8 *FifoBuff,u8 SendLen);
u8 PN512ReadFifo(u8 *FifoBuff,u8 *pRevLen);

void RFinterrupt(void);


#endif
