/// @file   SPIFlash.h
/// This is for module @ref SPI_Flash
/// @author Travis Tu
/// @date   2006-Sep-1st

#ifndef _DataFLASH_H_
#define _DataFLASH_H_

#define DATA_FLASH_PAGE_ID_MASK             0x0FFE00
#define DATA_FLASH_BASE_ADDRESS_MASK        0X0001FF
#define DATA_FLASH_PAGE_SIZE 264
#define DATA_FLASH_HALF_CLOCK() {/*u32 i=13;i*=32;*/}
#define DF_PAGEID(addr)                     ((u16)((addr)/DATA_FLASH_PAGE_SIZE))
#define DF_BASEADDR(addr)                   ((u16)((addr)%DATA_FLASH_PAGE_SIZE))


//init the data flash communication and destory the smart card communication
bool InitDataFlash();

//send one byte
void DF_SendByte(u8 byte);
//receive one byte
u8 DF_RecvByte(void);
//set IO high
inline void DF_SetIOHigh() {GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_SET);}
//set IO Low
inline void DF_SetIOLow() {GPIO_WriteBit(GPIOA,GPIO_Pin_5,Bit_RESET);}
//Chip select
inline void DF_ChipSlect() {GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_RESET);}
//Chip Deselect
inline void DF_ChipDeslect() {GPIO_WriteBit(GPIOA,GPIO_Pin_6,Bit_SET);}
//convert sequence address to phisical address
inline u32  DF_GetPhyAddr(u32 logicalAddr) {return DF_PAGEID(logicalAddr)<<9 | DF_BASEADDR(logicalAddr);}
//wait till finished
void DF_WaitToReady();
//send an physical address with given logical address
void DF_SendAddr(u32 logicalAddr);
//STATUS REGISTER
u8   DF_GetStatusReg(void);
//MAIN MEMORY PAGE READ
u8   DF_MemRead(u32 addr, u16 length, u8* buf);
//MAIN MEMORY PAGE PROGRAM
u8   DF_MemProgram(u32 addr, u16 length, u8* buf);
//reset the flash card
void DF_Reset(void);

#endif /* _DataFLASH_H_ */

