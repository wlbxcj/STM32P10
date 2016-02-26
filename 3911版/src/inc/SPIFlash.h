/// @file   SPIFlash.h
/// This is for module @ref SPI_Flash
/// @author Travis Tu
/// @date   2006-Sep-1st

#include "stm32f10x_lib.h"

#ifndef _SPIFLASH_H_
#define _SPIFLASH_H_
/**
 * @addtogroup Platform_Device_Library
 * @{
 */

/**
 * @addtogroup SPI_Flash
 * @{
 */

/**
 * @defgroup SPI_Flash_DEFINE
 * @ingroup SPI_Flash
 * @{
 */
#define  M25_WREN       0x06            ///< Instruction Byte: Write Enable instruction
#define  M25_WRDI       0x04            ///< Instruction Byte: Write Disable instrction
#define  M25_RDID       0x9F            ///< Instruction Byte: Read Identification
#define  M25_RDSR       0x05            ///< Instruction Byte: Read Status Register instruction
#define  M25_WRSR       0x01            ///< Instruction Byte: Write Status Register instruction
#define  M25_READ       0x03            ///< Instruction Byte: Read Data Byte instruction
#define  M25_FASTREAD   0x0B            ///< Instruction Byte: Read Data Byte at Higher Speed
#define  M25_PP         0x02            ///< Instruction Byte: Page Program instruction
#define  M25_SE         0xD8            ///< Instruction Byte: Sector Erase instruction
#define  M25_BE         0xC7            ///< Instruction Byte: Bulk Erase instruction
#define  M25_DP         0xB9            ///< Instruction Byte: Deep Power-down instruction
#define  M25_RES        0xAB            ///< Instruction Byte: Read Electronic Signature

#define  M25_WIP        0x01            ///< Status Register Bit definition: Write In Progress bit, polling it to establish when the previous write cycle or erase cycle is complete.
#define  M25_WEL        0x02            ///< Status Register Bit definition: Write Enable Latch bit indicates the status of the internal write enable latch.
#define  M52_BP0        0x04            ///< Status Register Bit definition: Block Protect bits, they define the area to be software
#define  M25_BP1        0x08            ///< Status Register Bit definition: protected against program and erase instructions
#define  M25_SRWD       0x80            ///< Status Register Bit definition: The Status Register Write Protect,
/**@}*/

/**
 * @defgroup SPI_Flash_LOW_INTERFACE
 * @ingroup SPI_Flash
 * @{
 */
u8 BPSI_DataSendReceive(u8 DATA);           ///<Send a byte and receive a byte at the same time
void M25_WriteAddress(u32 address);         ///<Write address to the SPI Flash
void BSPIFlashWREN  (u32 address);                 ///<write enable
void BSPIFlashWRDI  (u32 address);                 ///<write disable
u8   BSPIFlashRDSR  (u32 address);                 ///<read Satus Register
//void BSPIFlashWRSR  (void);               ///<write Status Register
//void BSPIFlashRES   (void);               ///<Read Electronic Signature
//void BSPIFlashRDID  (void);               ///<read device ID
/**@}*/
//#define SPIFLASH_SECTOR_SIZE        0x10000     //4M 8M FLASH   YYW 20080504
//#define SPIFLASH_BULK_SIZE          0x800000

#define SPIFLASH_SECTOR_SIZE        0x40000     //16M FLASH   YYW 20080504
#define SPIFLASH_BULK_SIZE          0x1000000


#define SPIFLASH_1ST_BULK_ADDR      (SPIFLASH_BULK_SIZE*0)
#define SPIFLASH_2ND_BULK_ADDR      (SPIFLASH_BULK_SIZE*1)
#define SPIFLASH_INBULK_ADDR_MASK   (SPIFLASH_BULK_SIZE-1)

/**
 * @defgroup SPI_Flash_HIGH_INTERFACE
 * @ingroup SPI_Flash
 * @{
 */
void BSPIFlashInit  (void);                                         ///<Initialize the SPI Flash communication
void BSPIFlashREAD  (u32 fromAddress, u8* buf, u16 NbOfBytes);      ///<read data
void BSPIFlashPP    (const u8* buf, u16 NbOfBytes, u32 toAddress);  ///<Page Program
void BSPIFlashSE    (u32 sectorAddress);                            ///<Sector Erase
void BSPIFlashBE    (u32 address);                                         ///<Bulk Erase
/**@}*/

/**@}*/

/**@}*/
#endif /* _SPIFLASH_H_ */

